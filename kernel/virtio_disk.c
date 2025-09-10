//
// driver for qemu's virtio disk device.
// uses qemu's mmio interface to virtio.
//
// qemu ... -drive file=fs.img,if=none,format=raw,id=x0 -device virtio-blk-device,drive=x0,bus=virtio-mmio-bus.0
//

#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "buf.h"
#include "virtio.h"

// Debug configuration
#define DEBUG_VIRTIO 1
#define DEBUG_VIRTIO_INIT 1
#define DEBUG_VIRTIO_IO 1
#define DEBUG_VIRTIO_INTERRUPT 1
#define DEBUG_VIRTIO_DESCRIPTORS 1
#define DEBUG_PERFORMANCE 1

// VirtIO statistics
static struct {
  uint64 read_requests;
  uint64 write_requests;
  uint64 completed_ops;
  uint64 failed_ops;
  uint64 interrupts;
  uint64 descriptor_alloc_failures;
  uint64 total_bytes_read;
  uint64 total_bytes_written;
  uint64 max_queue_depth;
  uint64 avg_response_time;
  struct spinlock lock;
} virtio_stats;

// Performance timing
static struct {
  uint64 start_time[NUM];
  uint64 total_time;
  uint64 operation_count;
} perf_data;

// the address of virtio mmio register r.
#define R(r) ((volatile uint32 *)(VIRTIO0 + (r)))

static struct disk {
  // a set (not a ring) of DMA descriptors, with which the
  // driver tells the device where to read and write individual
  // disk operations. there are NUM descriptors.
  // most commands consist of a "chain" (a linked list) of a couple of
  // these descriptors.
  struct virtq_desc *desc;

  // a ring in which the driver writes descriptor numbers
  // that the driver would like the device to process.  it only
  // includes the head descriptor of each chain. the ring has
  // NUM elements.
  struct virtq_avail *avail;

  // a ring in which the device writes descriptor numbers that
  // the device has finished processing (just the head of each chain).
  // there are NUM used ring entries.
  struct virtq_used *used;

  // our own book-keeping.
  char free[NUM];  // is a descriptor free?
  uint16 used_idx; // we've looked this far in used[2..NUM].

  // track info about in-flight operations,
  // for use when completion interrupt arrives.
  // indexed by first descriptor index of chain.
  struct {
    struct buf *b;
    char status;
  } info[NUM];

  // disk command headers.
  // one-for-one with descriptors, for convenience.
  struct virtio_blk_req ops[NUM];
  
  struct spinlock vdisk_lock;
  
} disk;

// Initialize VirtIO statistics
static void
virtio_stats_init(void)
{
  initlock(&virtio_stats.lock, "virtio_stats");
  virtio_stats.read_requests = 0;
  virtio_stats.write_requests = 0;
  virtio_stats.completed_ops = 0;
  virtio_stats.failed_ops = 0;
  virtio_stats.interrupts = 0;
  virtio_stats.descriptor_alloc_failures = 0;
  virtio_stats.total_bytes_read = 0;
  virtio_stats.total_bytes_written = 0;
  virtio_stats.max_queue_depth = 0;
  virtio_stats.avg_response_time = 0;
}

// Print VirtIO statistics
void
print_virtio_stats(void)
{
  acquire(&virtio_stats.lock);
  printf("=== VirtIO Disk Statistics ===\n");
  printf("Read requests: %d\n", virtio_stats.read_requests);
  printf("Write requests: %d\n", virtio_stats.write_requests);
  printf("Completed operations: %d\n", virtio_stats.completed_ops);
  printf("Failed operations: %d\n", virtio_stats.failed_ops);
  printf("Interrupts: %d\n", virtio_stats.interrupts);
  printf("Descriptor allocation failures: %d\n", virtio_stats.descriptor_alloc_failures);
  printf("Total bytes read: %d\n", virtio_stats.total_bytes_read);
  printf("Total bytes written: %d\n", virtio_stats.total_bytes_written);
  printf("Max queue depth: %d\n", virtio_stats.max_queue_depth);
  if(perf_data.operation_count > 0) {
    printf("Average response time: %d cycles\n", 
           perf_data.total_time / perf_data.operation_count);
  }
  release(&virtio_stats.lock);
}

// Validate VirtIO register access
static int
validate_virtio_reg(uint32 offset)
{
  if(offset > 0x100) {
    printf("[ERROR] VirtIO: invalid register offset 0x%x\n", offset);
    return 0;
  }
  return 1;
}

// Safe register read with validation
static uint32
safe_read_reg(uint32 offset)
{
  if(!validate_virtio_reg(offset))
    return 0;
  return *R(offset);
}

// Safe register write with validation
static void
safe_write_reg(uint32 offset, uint32 value)
{
  if(!validate_virtio_reg(offset))
    return;
  *R(offset) = value;
}

// Debug function to dump VirtIO device state
void
virtio_debug_device_state(void)
{
  printf("=== VirtIO Device State ===\n");
  printf("Magic: 0x%x (expected: 0x74726976)\n", safe_read_reg(VIRTIO_MMIO_MAGIC_VALUE));
  printf("Version: %d (expected: 2)\n", safe_read_reg(VIRTIO_MMIO_VERSION));
  printf("Device ID: %d (expected: 2 for block)\n", safe_read_reg(VIRTIO_MMIO_DEVICE_ID));
  printf("Vendor ID: 0x%x (expected: 0x554d4551)\n", safe_read_reg(VIRTIO_MMIO_VENDOR_ID));
  printf("Status: 0x%x\n", safe_read_reg(VIRTIO_MMIO_STATUS));
  printf("Queue ready: %d\n", safe_read_reg(VIRTIO_MMIO_QUEUE_READY));
  printf("Queue num: %d\n", safe_read_reg(VIRTIO_MMIO_QUEUE_NUM));
  printf("Interrupt status: 0x%x\n", safe_read_reg(VIRTIO_MMIO_INTERRUPT_STATUS));
  printf("==========================\n");
}

void
virtio_disk_init(void)
{
  uint32 status = 0;

  virtio_stats_init();
  initlock(&disk.vdisk_lock, "virtio_disk");

  if(DEBUG_VIRTIO_INIT) {
    printf("[VIRTIO] Initializing VirtIO disk...\n");
  }

  // Validate device presence and type
  uint32 magic = safe_read_reg(VIRTIO_MMIO_MAGIC_VALUE);
  uint32 version = safe_read_reg(VIRTIO_MMIO_VERSION);
  uint32 device_id = safe_read_reg(VIRTIO_MMIO_DEVICE_ID);
  uint32 vendor_id = safe_read_reg(VIRTIO_MMIO_VENDOR_ID);

  if(magic != 0x74726976 || version != 2 || device_id != 2 || vendor_id != 0x554d4551) {
    printf("[ERROR] VirtIO validation failed:\n");
    printf("  Magic: 0x%x (expected: 0x74726976)\n", magic);
    printf("  Version: %d (expected: 2)\n", version);
    printf("  Device ID: %d (expected: 2)\n", device_id);
    printf("  Vendor ID: 0x%x (expected: 0x554d4551)\n", vendor_id);
    panic("could not find virtio disk");
  }

  if(DEBUG_VIRTIO_INIT) {
    printf("[VIRTIO] Device validation passed\n");
    virtio_debug_device_state();
  }
  
  // reset device
  safe_write_reg(VIRTIO_MMIO_STATUS, status);

  // set ACKNOWLEDGE status bit
  status |= VIRTIO_CONFIG_S_ACKNOWLEDGE;
  safe_write_reg(VIRTIO_MMIO_STATUS, status);

  // set DRIVER status bit
  status |= VIRTIO_CONFIG_S_DRIVER;
  safe_write_reg(VIRTIO_MMIO_STATUS, status);

  // negotiate features
  uint64 features = safe_read_reg(VIRTIO_MMIO_DEVICE_FEATURES);
  if(DEBUG_VIRTIO_INIT) {
    printf("[VIRTIO] Device features: 0x%x\n", features);
  }
  
  features &= ~(1 << VIRTIO_BLK_F_RO);
  features &= ~(1 << VIRTIO_BLK_F_SCSI);
  features &= ~(1 << VIRTIO_BLK_F_CONFIG_WCE);
  features &= ~(1 << VIRTIO_BLK_F_MQ);
  features &= ~(1 << VIRTIO_F_ANY_LAYOUT);
  features &= ~(1 << VIRTIO_RING_F_EVENT_IDX);
  features &= ~(1 << VIRTIO_RING_F_INDIRECT_DESC);
  
  safe_write_reg(VIRTIO_MMIO_DRIVER_FEATURES, features);

  if(DEBUG_VIRTIO_INIT) {
    printf("[VIRTIO] Negotiated features: 0x%x\n", features);
  }

  // tell device that feature negotiation is complete.
  status |= VIRTIO_CONFIG_S_FEATURES_OK;
  safe_write_reg(VIRTIO_MMIO_STATUS, status);

  // re-read status to ensure FEATURES_OK is set.
  status = safe_read_reg(VIRTIO_MMIO_STATUS);
  if(!(status & VIRTIO_CONFIG_S_FEATURES_OK)) {
    printf("[ERROR] VirtIO FEATURES_OK not set after negotiation\n");
    panic("virtio disk FEATURES_OK unset");
  }

  // initialize queue 0.
  safe_write_reg(VIRTIO_MMIO_QUEUE_SEL, 0);

  // ensure queue 0 is not in use.
  if(safe_read_reg(VIRTIO_MMIO_QUEUE_READY)) {
    printf("[ERROR] VirtIO queue already ready\n");
    panic("virtio disk should not be ready");
  }

  // check maximum queue size.
  uint32 max = safe_read_reg(VIRTIO_MMIO_QUEUE_NUM_MAX);
  if(max == 0) {
    printf("[ERROR] VirtIO queue 0 not available\n");
    panic("virtio disk has no queue 0");
  }
  if(max < NUM) {
    printf("[ERROR] VirtIO max queue size %d < required %d\n", max, NUM);
    panic("virtio disk max queue too short");
  }

  if(DEBUG_VIRTIO_INIT) {
    printf("[VIRTIO] Max queue size: %d, using: %d\n", max, NUM);
  }

  // allocate and zero queue memory.
  disk.desc = kalloc();
  disk.avail = kalloc();
  disk.used = kalloc();
  if(!disk.desc || !disk.avail || !disk.used) {
    printf("[ERROR] VirtIO failed to allocate queue memory\n");
    panic("virtio disk kalloc");
  }
  memset(disk.desc, 0, PGSIZE);
  memset(disk.avail, 0, PGSIZE);
  memset(disk.used, 0, PGSIZE);

  if(DEBUG_VIRTIO_INIT) {
    printf("[VIRTIO] Allocated queues: desc=0x%p avail=0x%p used=0x%p\n", 
           disk.desc, disk.avail, disk.used);
  }

  // set queue size.
  safe_write_reg(VIRTIO_MMIO_QUEUE_NUM, NUM);

  // write physical addresses.
  safe_write_reg(VIRTIO_MMIO_QUEUE_DESC_LOW, (uint64)disk.desc);
  safe_write_reg(VIRTIO_MMIO_QUEUE_DESC_HIGH, (uint64)disk.desc >> 32);
  safe_write_reg(VIRTIO_MMIO_DRIVER_DESC_LOW, (uint64)disk.avail);
  safe_write_reg(VIRTIO_MMIO_DRIVER_DESC_HIGH, (uint64)disk.avail >> 32);
  safe_write_reg(VIRTIO_MMIO_DEVICE_DESC_LOW, (uint64)disk.used);
  safe_write_reg(VIRTIO_MMIO_DEVICE_DESC_HIGH, (uint64)disk.used >> 32);

  // queue is ready.
  safe_write_reg(VIRTIO_MMIO_QUEUE_READY, 0x1);

  // all NUM descriptors start out unused.
  for(int i = 0; i < NUM; i++)
    disk.free[i] = 1;

  // tell device we're completely ready.
  status |= VIRTIO_CONFIG_S_DRIVER_OK;
  safe_write_reg(VIRTIO_MMIO_STATUS, status);

  if(DEBUG_VIRTIO_INIT) {
    printf("[VIRTIO] Initialization complete, final status: 0x%x\n", status);
  }

  // plic.c and trap.c arrange for interrupts from VIRTIO0_IRQ.
}

// find a free descriptor, mark it non-free, return its index.
static int
alloc_desc()
{
  for(int i = 0; i < NUM; i++){
    if(disk.free[i]){
      disk.free[i] = 0;
      
      if(DEBUG_VIRTIO_DESCRIPTORS) {
        printf("[VIRTIO] Allocated descriptor %d\n", i);
      }
      
      return i;
    }
  }
  
  if(DEBUG_VIRTIO_DESCRIPTORS) {
    printf("[ERROR] No free descriptors available\n");
  }
  
  acquire(&virtio_stats.lock);
  virtio_stats.descriptor_alloc_failures++;
  release(&virtio_stats.lock);
  
  return -1;
}

// mark a descriptor as free.
static void
free_desc(int i)
{
  if(i >= NUM) {
    printf("[ERROR] free_desc: invalid index %d\n", i);
    panic("free_desc 1");
  }
  if(disk.free[i]) {
    printf("[ERROR] free_desc: descriptor %d already free\n", i);
    panic("free_desc 2");
  }
  
  if(DEBUG_VIRTIO_DESCRIPTORS) {
    printf("[VIRTIO] Freeing descriptor %d\n", i);
  }
  
  disk.desc[i].addr = 0;
  disk.desc[i].len = 0;
  disk.desc[i].flags = 0;
  disk.desc[i].next = 0;
  disk.free[i] = 1;
  wakeup(&disk.free[0]);
}

// free a chain of descriptors.
static void
free_chain(int i)
{
  if(DEBUG_VIRTIO_DESCRIPTORS) {
    printf("[VIRTIO] Freeing descriptor chain starting at %d\n", i);
  }
  
  int count = 0;
  while(1){
    int flag = disk.desc[i].flags;
    int nxt = disk.desc[i].next;
    free_desc(i);
    count++;
    
    if(flag & VRING_DESC_F_NEXT)
      i = nxt;
    else
      break;
      
    // Prevent infinite loops
    if(count > NUM) {
      printf("[ERROR] Descriptor chain too long, possible corruption\n");
      break;
    }
  }
  
  if(DEBUG_VIRTIO_DESCRIPTORS) {
    printf("[VIRTIO] Freed %d descriptors in chain\n", count);
  }
}

// allocate three descriptors (they need not be contiguous).
// disk transfers always use three descriptors.
static int
alloc3_desc(int *idx)
{
  for(int i = 0; i < 3; i++){
    idx[i] = alloc_desc();
    if(idx[i] < 0){
      // Cleanup on failure
      for(int j = 0; j < i; j++)
        free_desc(idx[j]);
      return -1;
    }
  }
  
  if(DEBUG_VIRTIO_DESCRIPTORS) {
    printf("[VIRTIO] Allocated 3-descriptor chain: %d->%d->%d\n", 
           idx[0], idx[1], idx[2]);
  }
  
  return 0;
}

// Calculate current queue depth
static int
get_queue_depth(void)
{
  int in_use = 0;
  for(int i = 0; i < NUM; i++) {
    if(!disk.free[i]) in_use++;
  }
  return in_use;
}

void
virtio_disk_rw(struct buf *b, int write)
{
  uint64 sector = b->blockno * (BSIZE / 512);
  
  if(DEBUG_VIRTIO_IO) {
    printf("[VIRTIO] %s request: block=%d sector=%d\n", 
           write ? "Write" : "Read", b->blockno, sector);
  }
  
  // Validate parameters
  if(!b) {
    printf("[ERROR] virtio_disk_rw: null buffer\n");
    return;
  }
  
  if(b->blockno < 0) {
    printf("[ERROR] virtio_disk_rw: invalid block number %d\n", b->blockno);
    return;
  }

  acquire(&disk.vdisk_lock);
  
  // Update statistics
  acquire(&virtio_stats.lock);
  if(write) {
    virtio_stats.write_requests++;
    virtio_stats.total_bytes_written += BSIZE;
  } else {
    virtio_stats.read_requests++;
    virtio_stats.total_bytes_read += BSIZE;
  }
  
  int queue_depth = get_queue_depth();
  if(queue_depth > virtio_stats.max_queue_depth) {
    virtio_stats.max_queue_depth = queue_depth;
  }
  release(&virtio_stats.lock);

  // the spec's Section 5.2 says that legacy block operations use
  // three descriptors: one for type/reserved/sector, one for the
  // data, one for a 1-byte status result.

  // allocate the three descriptors.
  int idx[3];
  int timeout = 0;
  while(1){
    if(alloc3_desc(idx) == 0) {
      break;
    }
    
    timeout++;
    if(timeout > 1000) {
      printf("[ERROR] VirtIO: timeout waiting for descriptors\n");
      acquire(&virtio_stats.lock);
      virtio_stats.failed_ops++;
      release(&virtio_stats.lock);
      release(&disk.vdisk_lock);
      return;
    }
    
    sleep(&disk.free[0], &disk.vdisk_lock);
  }

  // Start timing for performance measurement
  uint64 start_time = 0;
  if(DEBUG_PERFORMANCE) {
    start_time = r_time();
    perf_data.start_time[idx[0]] = start_time;
  }

  // format the three descriptors.
  // qemu's virtio-blk.c reads them.
  struct virtio_blk_req *buf0 = &disk.ops[idx[0]];

  if(write)
    buf0->type = VIRTIO_BLK_T_OUT; // write the disk
  else
    buf0->type = VIRTIO_BLK_T_IN; // read the disk
  buf0->reserved = 0;
  buf0->sector = sector;

  // Descriptor 0: command header
  disk.desc[idx[0]].addr = (uint64) buf0;
  disk.desc[idx[0]].len = sizeof(struct virtio_blk_req);
  disk.desc[idx[0]].flags = VRING_DESC_F_NEXT;
  disk.desc[idx[0]].next = idx[1];

  // Descriptor 1: data buffer
  disk.desc[idx[1]].addr = (uint64) b->data;
  disk.desc[idx[1]].len = BSIZE;
  if(write)
    disk.desc[idx[1]].flags = 0; // device reads b->data
  else
    disk.desc[idx[1]].flags = VRING_DESC_F_WRITE; // device writes b->data
  disk.desc[idx[1]].flags |= VRING_DESC_F_NEXT;
  disk.desc[idx[1]].next = idx[2];

  // Descriptor 2: status byte
  disk.info[idx[0]].status = 0xff; // device writes 0 on success
  disk.desc[idx[2]].addr = (uint64) &disk.info[idx[0]].status;
  disk.desc[idx[2]].len = 1;
  disk.desc[idx[2]].flags = VRING_DESC_F_WRITE; // device writes the status
  disk.desc[idx[2]].next = 0;

  // record struct buf for virtio_disk_intr().
  b->disk = 1;
  disk.info[idx[0]].b = b;

  if(DEBUG_VIRTIO_IO) {
    printf("[VIRTIO] Descriptor chain setup complete for %s\n", 
           write ? "write" : "read");
  }

  // tell the device the first index in our chain of descriptors.
  disk.avail->ring[disk.avail->idx % NUM] = idx[0];

  __sync_synchronize();

  // tell the device another avail ring entry is available.
  disk.avail->idx += 1; // not % NUM ...

  __sync_synchronize();

  // Notify the device
  safe_write_reg(VIRTIO_MMIO_QUEUE_NOTIFY, 0); // value is queue number

  if(DEBUG_VIRTIO_IO) {
    printf("[VIRTIO] Device notified, waiting for completion\n");
  }

  // Wait for virtio_disk_intr() to say request has finished.
  timeout = 0;
  while(b->disk == 1) {
    sleep(b, &disk.vdisk_lock);
    timeout++;
    if(timeout > 10000) {
      printf("[ERROR] VirtIO: timeout waiting for completion\n");
      acquire(&virtio_stats.lock);
      virtio_stats.failed_ops++;
      release(&virtio_stats.lock);
      break;
    }
  }

  // Performance measurement
  if(DEBUG_PERFORMANCE && start_time) {
    uint64 end_time = r_time();
    uint64 duration = end_time - start_time;
    perf_data.total_time += duration;
    perf_data.operation_count++;
    
    if(DEBUG_VIRTIO_IO) {
      printf("[VIRTIO] Operation took %d cycles\n", duration);
    }
  }

  disk.info[idx[0]].b = 0;
  free_chain(idx[0]);

  if(DEBUG_VIRTIO_IO) {
    printf("[VIRTIO] %s completed successfully\n", write ? "Write" : "Read");
  }

  acquire(&virtio_stats.lock);
  virtio_stats.completed_ops++;
  release(&virtio_stats.lock);

  release(&disk.vdisk_lock);
}

void
virtio_disk_intr()
{
  acquire(&disk.vdisk_lock);
  
  acquire(&virtio_stats.lock);
  virtio_stats.interrupts++;
  release(&virtio_stats.lock);

  if(DEBUG_VIRTIO_INTERRUPT) {
    printf("[VIRTIO] Interrupt received\n");
  }

  // the device won't raise another interrupt until we tell it
  // we've seen this interrupt, which the following line does.
  // this may race with the device writing new entries to
  // the "used" ring, in which case we may process the new
  // completion entries in this interrupt, and have nothing to do
  // in the next interrupt, which is harmless.
  uint32 int_status = safe_read_reg(VIRTIO_MMIO_INTERRUPT_STATUS);
  safe_write_reg(VIRTIO_MMIO_INTERRUPT_ACK, int_status & 0x3);
  
  if(DEBUG_VIRTIO_INTERRUPT) {
    printf("[VIRTIO] Interrupt status: 0x%x, acknowledged\n", int_status);
  }

  __sync_synchronize();

  // the device increments disk.used->idx when it
  // adds an entry to the used ring.
  int processed = 0;
  while(disk.used_idx != disk.used->idx){
    __sync_synchronize();
    int id = disk.used->ring[disk.used_idx % NUM].id;

    if(DEBUG_VIRTIO_INTERRUPT) {
      printf("[VIRTIO] Processing completion for descriptor %d\n", id);
    }

    // Validate descriptor ID
    if(id >= NUM) {
      printf("[ERROR] VirtIO: invalid descriptor ID %d\n", id);
      break;
    }

    if(disk.info[id].status != 0) {
      printf("[ERROR] VirtIO operation failed: status=%d\n", disk.info[id].status);
      acquire(&virtio_stats.lock);
      virtio_stats.failed_ops++;
      release(&virtio_stats.lock);
      panic("virtio_disk_intr status");
    }

    struct buf *b = disk.info[id].b;
    if(!b) {
      printf("[ERROR] VirtIO: null buffer for descriptor %d\n", id);
    } else {
      b->disk = 0;   // disk is done with buf
      wakeup(b);
      
      if(DEBUG_VIRTIO_INTERRUPT) {
        printf("[VIRTIO] Woke up buffer for block %d\n", b->blockno);
      }
    }

    disk.used_idx += 1;
    processed++;
    
    // Prevent infinite loop in case of device issues
    if(processed > NUM) {
      printf("[WARNING] VirtIO: processed too many completions\n");
      break;
    }
  }

  if(DEBUG_VIRTIO_INTERRUPT) {
    printf("[VIRTIO] Interrupt handled, processed %d completions\n", processed);
  }

  release(&disk.vdisk_lock);
}

// Test function for VirtIO disk
void
virtio_disk_test(void)
{
  printf("[VIRTIO] Starting disk test...\n");
  
  // Allocate a test buffer
  struct buf *test_buf = kalloc();
  if(!test_buf) {
    printf("[ERROR] Failed to allocate test buffer\n");
    return;
  }
  
  memset(test_buf, 0, sizeof(struct buf));
  test_buf->blockno = 1;  // Test block 1
  test_buf->disk = 0;
  
  // Write test pattern
  for(int i = 0; i < BSIZE; i++) {
    test_buf->data[i] = i & 0xff;
  }
  
  printf("[VIRTIO] Writing test pattern to block 1...\n");
  virtio_disk_rw(test_buf, 1);  // Write
  
  // Clear buffer and read back
  memset(test_buf->data, 0, BSIZE);
  printf("[VIRTIO] Reading back from block 1...\n");
  virtio_disk_rw(test_buf, 0);  // Read
  
  // Verify pattern
  int errors = 0;
  for(int i = 0; i < BSIZE; i++) {
    if(test_buf->data[i] != (i & 0xff)) {
      errors++;
    }
  }
  
  if(errors == 0) {
    printf("[VIRTIO] Disk test PASSED\n");
  } else {
    printf("[VIRTIO] Disk test FAILED: %d byte errors\n", errors);
  }
  
  kfree(test_buf);
}