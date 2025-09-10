// Buffer cache.
//
// The buffer cache is a linked list of buf structures holding
// cached copies of disk block contents. Caching disk blocks
// in memory reduces the number of disk reads and also provides
// a synchronization point for disk blocks used by multiple processes.
//
// Interface:
// * To get a buffer for a particular disk block, call bread.
// * After changing buffer data, call bwrite to write it to disk.
// * When done with the buffer, call brelse.
// * Do not use the buffer after calling brelse.
// * Only one process at a time can use a buffer,
//     so do not keep them longer than necessary.

#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "riscv.h"
#include "defs.h"
#include "fs.h"
#include "buf.h"

// Debug configuration
#define DEBUG_BIO 1
#define DEBUG_BIO_VERBOSE 0
#define DEBUG_BIO_STATS 1
#define ENABLE_BIO_VALIDATION 1

// Statistics tracking
struct bio_stats {
  uint64 bread_calls;
  uint64 bwrite_calls;
  uint64 brelse_calls;
  uint64 bget_hits;
  uint64 bget_misses;
  uint64 bget_evictions;
  uint64 bpin_calls;
  uint64 bunpin_calls;
  uint64 cache_full_events;
  uint64 validation_errors;
  uint64 total_disk_reads;
  uint64 total_disk_writes;
  struct spinlock stats_lock;
} bio_stats;

struct {
  struct spinlock lock;
  struct buf buf[NBUF];

  // Linked list of all buffers, through prev/next.
  // Sorted by how recently the buffer was used.
  // head.next is most recent, head.prev is least recent.
  struct buf head;
} bcache;

// Buffer validation function
static int
validate_buffer(struct buf *b, const char *caller)
{
  if (!ENABLE_BIO_VALIDATION) {
    return 1;
  }
  
  if (!b) {
    printf("[BIO ERROR] %s: null buffer pointer\n", caller);
    acquire(&bio_stats.stats_lock);
    bio_stats.validation_errors++;
    release(&bio_stats.stats_lock);
    return 0;
  }
  
  // Check if buffer is within valid range
  if (b < bcache.buf || b >= bcache.buf + NBUF) {
    printf("[BIO ERROR] %s: buffer pointer out of range: %p\n", caller, b);
    acquire(&bio_stats.stats_lock);
    bio_stats.validation_errors++;
    release(&bio_stats.stats_lock);
    return 0;
  }
  
  // Additional validation checks
  if (b->refcnt < 0) {
    printf("[BIO ERROR] %s: negative refcnt %d for buf %p\n", caller, b->refcnt, b);
    acquire(&bio_stats.stats_lock);
    bio_stats.validation_errors++;
    release(&bio_stats.stats_lock);
    return 0;
  }
  
  return 1;
}

// Print buffer cache statistics
void
print_bio_stats(void)
{
  if (!DEBUG_BIO_STATS) return;
  
  acquire(&bio_stats.stats_lock);
  printf("=== Buffer Cache Statistics ===\n");
  printf("Total operations:\n");
  printf("  bread: %d\n", bio_stats.bread_calls);
  printf("  bwrite: %d\n", bio_stats.bwrite_calls);
  printf("  brelse: %d\n", bio_stats.brelse_calls);
  printf("  bpin: %d\n", bio_stats.bpin_calls);
  printf("  bunpin: %d\n", bio_stats.bunpin_calls);
  printf("Cache performance:\n");
  printf("  hits: %d\n", bio_stats.bget_hits);
  printf("  misses: %d\n", bio_stats.bget_misses);
  printf("  evictions: %d\n", bio_stats.bget_evictions);
  printf("  hit rate: %d%%\n", 
    (bio_stats.bget_hits + bio_stats.bget_misses) ? 
    (100 * bio_stats.bget_hits) / (bio_stats.bget_hits + bio_stats.bget_misses) : 0);
  printf("Disk I/O:\n");
  printf("  reads: %d\n", bio_stats.total_disk_reads);
  printf("  writes: %d\n", bio_stats.total_disk_writes);
  printf("Errors:\n");
  printf("  validation errors: %d\n", bio_stats.validation_errors);
  printf("  cache full events: %d\n", bio_stats.cache_full_events);
  printf("==============================\n");
  release(&bio_stats.stats_lock);
}

void
binit(void)
{
  struct buf *b;

  if (DEBUG_BIO) {
    printf("[BIO] Initializing buffer cache with %d buffers\n", NBUF);
  }

  initlock(&bcache.lock, "bcache");
  initlock(&bio_stats.stats_lock, "bio_stats");
  
  // Initialize statistics
  bio_stats.bread_calls = 0;
  bio_stats.bwrite_calls = 0;
  bio_stats.brelse_calls = 0;
  bio_stats.bget_hits = 0;
  bio_stats.bget_misses = 0;
  bio_stats.bget_evictions = 0;
  bio_stats.bpin_calls = 0;
  bio_stats.bunpin_calls = 0;
  bio_stats.cache_full_events = 0;
  bio_stats.validation_errors = 0;
  bio_stats.total_disk_reads = 0;
  bio_stats.total_disk_writes = 0;

  // Create linked list of buffers
  bcache.head.prev = &bcache.head;
  bcache.head.next = &bcache.head;
  
  int initialized_count = 0;
  for (b = bcache.buf; b < bcache.buf + NBUF; b++) {
    b->next = bcache.head.next;
    b->prev = &bcache.head;
    initsleeplock(&b->lock, "buffer");
    bcache.head.next->prev = b;
    bcache.head.next = b;
    
    // Initialize buffer fields
    b->dev = 0;
    b->blockno = 0;
    b->valid = 0;
    b->refcnt = 0;
    
    initialized_count++;
  }

  if (DEBUG_BIO) {
    printf("[BIO] Initialized %d buffers successfully\n", initialized_count);
  }
}

// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.
static struct buf*
bget(uint dev, uint blockno)
{
  struct buf *b;
  uint64 start_time = 0;
  
  if (DEBUG_BIO_VERBOSE) {
    start_time = r_time();
    printf("[BIO] bget: dev=%d blockno=%d\n", dev, blockno);
  }

  acquire(&bcache.lock);

  // Is the block already cached?
  int search_count = 0;
  for (b = bcache.head.next; b != &bcache.head; b = b->next) {
    search_count++;
    if (b->dev == dev && b->blockno == blockno) {
      b->refcnt++;
      release(&bcache.lock);
      acquiresleep(&b->lock);
      
      // Update statistics
      acquire(&bio_stats.stats_lock);
      bio_stats.bget_hits++;
      release(&bio_stats.stats_lock);
      
      if (DEBUG_BIO_VERBOSE) {
        uint64 end_time = r_time();
        printf("[BIO] bget: cache hit dev=%d blockno=%d refcnt=%d (searched %d, took %d cycles)\n", 
               dev, blockno, b->refcnt, search_count, end_time - start_time);
      }
      return b;
    }
  }

  // Not cached - need to find a buffer to reuse
  acquire(&bio_stats.stats_lock);
  bio_stats.bget_misses++;
  release(&bio_stats.stats_lock);
  
  if (DEBUG_BIO_VERBOSE) {
    printf("[BIO] bget: cache miss dev=%d blockno=%d (searched %d buffers)\n", 
           dev, blockno, search_count);
  }

  // Recycle the least recently used (LRU) unused buffer.
  int lru_search_count = 0;
  for (b = bcache.head.prev; b != &bcache.head; b = b->prev) {
    lru_search_count++;
    if (b->refcnt == 0) {
      // Found an unused buffer
      if (DEBUG_BIO_VERBOSE && (b->dev != 0 || b->blockno != 0)) {
        printf("[BIO] bget: evicting dev=%d blockno=%d for dev=%d blockno=%d\n", 
               b->dev, b->blockno, dev, blockno);
      }
      
      b->dev = dev;
      b->blockno = blockno;
      b->valid = 0;
      b->refcnt = 1;
      
      // Update statistics
      acquire(&bio_stats.stats_lock);
      bio_stats.bget_evictions++;
      release(&bio_stats.stats_lock);
      
      release(&bcache.lock);
      acquiresleep(&b->lock);
      
      if (DEBUG_BIO_VERBOSE) {
        uint64 end_time = r_time();
        printf("[BIO] bget: allocated buffer dev=%d blockno=%d (LRU searched %d, took %d cycles)\n", 
               dev, blockno, lru_search_count, end_time - start_time);
      }
      return b;
    }
  }
  
  // No available buffers
  acquire(&bio_stats.stats_lock);
  bio_stats.cache_full_events++;
  release(&bio_stats.stats_lock);
  
  release(&bcache.lock);
  
  printf("[BIO ERROR] bget: no buffers available (all %d buffers in use)\n", NBUF);
  panic("bget: no buffers");
}

// Return a locked buf with the contents of the indicated block.
struct buf*
bread(uint dev, uint blockno)
{
  struct buf *b;
  uint64 start_time = 0;

  if (DEBUG_BIO) {
    start_time = r_time();
    printf("[BIO] bread: dev=%d blockno=%d\n", dev, blockno);
  }

  // Update statistics
  acquire(&bio_stats.stats_lock);
  bio_stats.bread_calls++;
  release(&bio_stats.stats_lock);

  b = bget(dev, blockno);
  
  if (!validate_buffer(b, "bread")) {
    printf("[BIO ERROR] bread: invalid buffer returned from bget\n");
    panic("bread: invalid buffer");
  }

  if (!b->valid) {
    if (DEBUG_BIO_VERBOSE) {
      printf("[BIO] bread: reading from disk dev=%d blockno=%d\n", dev, blockno);
    }
    
    virtio_disk_rw(b, 0);
    b->valid = 1;
    
    // Update disk read statistics
    acquire(&bio_stats.stats_lock);
    bio_stats.total_disk_reads++;
    release(&bio_stats.stats_lock);
  }

  if (DEBUG_BIO_VERBOSE) {
    uint64 end_time = r_time();
    printf("[BIO] bread: completed dev=%d blockno=%d valid=%d (took %d cycles)\n", 
           dev, blockno, b->valid, end_time - start_time);
  }

  return b;
}

// Write b's contents to disk. Must be locked.
void
bwrite(struct buf *b)
{
  uint64 start_time = 0;

  if (!validate_buffer(b, "bwrite")) {
    printf("[BIO ERROR] bwrite: invalid buffer\n");
    panic("bwrite: invalid buffer");
  }

  if (!holdingsleep(&b->lock)) {
    printf("[BIO ERROR] bwrite: buffer not locked dev=%d blockno=%d\n", b->dev, b->blockno);
    panic("bwrite");
  }

  if (DEBUG_BIO) {
    start_time = r_time();
    printf("[BIO] bwrite: dev=%d blockno=%d\n", b->dev, b->blockno);
  }

  // Update statistics
  acquire(&bio_stats.stats_lock);
  bio_stats.bwrite_calls++;
  bio_stats.total_disk_writes++;
  release(&bio_stats.stats_lock);

  virtio_disk_rw(b, 1);

  if (DEBUG_BIO_VERBOSE) {
    uint64 end_time = r_time();
    printf("[BIO] bwrite: completed dev=%d blockno=%d (took %d cycles)\n", 
           b->dev, b->blockno, end_time - start_time);
  }
}

// Release a locked buffer.
// Move to the head of the most-recently-used list.
void
brelse(struct buf *b)
{
  if (!validate_buffer(b, "brelse")) {
    printf("[BIO ERROR] brelse: invalid buffer\n");
    panic("brelse: invalid buffer");
  }

  if (!holdingsleep(&b->lock)) {
    printf("[BIO ERROR] brelse: buffer not locked dev=%d blockno=%d refcnt=%d\n", 
           b->dev, b->blockno, b->refcnt);
    panic("brelse");
  }

  if (DEBUG_BIO_VERBOSE) {
    printf("[BIO] brelse: dev=%d blockno=%d refcnt=%d\n", b->dev, b->blockno, b->refcnt);
  }

  // Update statistics
  acquire(&bio_stats.stats_lock);
  bio_stats.brelse_calls++;
  release(&bio_stats.stats_lock);

  releasesleep(&b->lock);

  acquire(&bcache.lock);
  b->refcnt--;
  
  if (b->refcnt < 0) {
    printf("[BIO ERROR] brelse: negative refcnt %d for dev=%d blockno=%d\n", 
           b->refcnt, b->dev, b->blockno);
    panic("brelse: negative refcnt");
  }
  
  if (b->refcnt == 0) {
    // No one is waiting for it - move to head of MRU list
    b->next->prev = b->prev;
    b->prev->next = b->next;
    b->next = bcache.head.next;
    b->prev = &bcache.head;
    bcache.head.next->prev = b;
    bcache.head.next = b;
    
    if (DEBUG_BIO_VERBOSE) {
      printf("[BIO] brelse: moved to MRU head dev=%d blockno=%d\n", b->dev, b->blockno);
    }
  }
  
  release(&bcache.lock);
}

// Pin a buffer in memory (increment reference count)
void
bpin(struct buf *b) {
  if (!validate_buffer(b, "bpin")) {
    printf("[BIO ERROR] bpin: invalid buffer\n");
    panic("bpin: invalid buffer");
  }

  if (DEBUG_BIO_VERBOSE) {
    printf("[BIO] bpin: dev=%d blockno=%d refcnt=%d->%d\n", 
           b->dev, b->blockno, b->refcnt, b->refcnt + 1);
  }

  acquire(&bcache.lock);
  b->refcnt++;
  release(&bcache.lock);

  // Update statistics
  acquire(&bio_stats.stats_lock);
  bio_stats.bpin_calls++;
  release(&bio_stats.stats_lock);
}

// Unpin a buffer (decrement reference count)
void
bunpin(struct buf *b) {
  if (!validate_buffer(b, "bunpin")) {
    printf("[BIO ERROR] bunpin: invalid buffer\n");
    panic("bunpin: invalid buffer");
  }

  if (DEBUG_BIO_VERBOSE) {
    printf("[BIO] bunpin: dev=%d blockno=%d refcnt=%d->%d\n", 
           b->dev, b->blockno, b->refcnt, b->refcnt - 1);
  }

  acquire(&bcache.lock);
  b->refcnt--;
  
  if (b->refcnt < 0) {
    printf("[BIO ERROR] bunpin: negative refcnt %d for dev=%d blockno=%d\n", 
           b->refcnt, b->dev, b->blockno);
    panic("bunpin: negative refcnt");
  }
  
  release(&bcache.lock);

  // Update statistics
  acquire(&bio_stats.stats_lock);
  bio_stats.bunpin_calls++;
  release(&bio_stats.stats_lock);
}

// Debug function to dump buffer cache state
void
bdump(void)
{
  struct buf *b;
  int used_count = 0, free_count = 0;
  
  printf("=== Buffer Cache Dump ===\n");
  acquire(&bcache.lock);
  
  for (b = bcache.head.next; b != &bcache.head; b = b->next) {
    if (b->refcnt > 0) {
      printf("USED: dev=%d block=%d refcnt=%d valid=%d\n", 
             b->dev, b->blockno, b->refcnt, b->valid);
      used_count++;
    } else {
      free_count++;
    }
  }
  
  release(&bcache.lock);
  printf("Used buffers: %d, Free buffers: %d\n", used_count, free_count);
  printf("========================\n");
  
  print_bio_stats();
}