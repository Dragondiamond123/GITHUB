// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

// Debug flags
#define DEBUG_KALLOC 0
#define DEBUG_MEMORY_STATS 0
#define POISON_FREE_PAGES 1
#define POISON_ALLOC_PAGES 1

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
  uint64 free_pages;
  uint64 total_allocated;
  uint64 total_freed;
  uint64 current_allocated;
} kmem;

void
kinit()
{
  initlock(&kmem.lock, "kmem");
  kmem.freelist = 0;
  kmem.free_pages = 0;
  kmem.total_allocated = 0;
  kmem.total_freed = 0;
  kmem.current_allocated = 0;
  
  freerange(end, (void*)PHYSTOP);
  
  if(DEBUG_KALLOC) {
    printf("[DEBUG] kinit: initialized memory allocator\n");
    printf("[DEBUG] kinit: kernel end = 0x%p, PHYSTOP = 0x%p\n", end, PHYSTOP);
    printf("[DEBUG] kinit: initial free pages = %d\n", kmem.free_pages);
  }
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  
  if(DEBUG_KALLOC) {
    printf("[DEBUG] freerange: start=0x%p end=0x%p\n", pa_start, pa_end);
  }
  
  if(!pa_start || !pa_end) {
    printf("[ERROR] freerange: null pointer\n");
    return;
  }
  
  if(pa_start >= pa_end) {
    printf("[ERROR] freerange: invalid range\n");
    return;
  }
  
  p = (char*)PGROUNDUP((uint64)pa_start);
  uint64 pages_freed = 0;
  
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE) {
    kfree(p);
    pages_freed++;
  }
  
  if(DEBUG_KALLOC) {
    printf("[DEBUG] freerange: freed %d pages\n", pages_freed);
  }
}

// Free the page of physical memory pointed at by pa,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if(!pa) {
    if(DEBUG_KALLOC) {
      printf("[ERROR] kfree: null pointer\n");
    }
    return;
  }

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP) {
    printf("[ERROR] kfree: invalid address 0x%p\n", pa);
    printf("  - page aligned: %s\n", ((uint64)pa % PGSIZE) == 0 ? "yes" : "no");
    printf("  - >= end: %s\n", (char*)pa >= end ? "yes" : "no");
    printf("  - < PHYSTOP: %s\n", (uint64)pa < PHYSTOP ? "yes" : "no");
    panic("kfree");
  }

  if(DEBUG_KALLOC) {
    printf("[DEBUG] kfree: freeing page at 0x%p\n", pa);
  }

  // Fill with junk to catch dangling refs.
  if(POISON_FREE_PAGES) {
    memset(pa, 1, PGSIZE);
  }

  r = (struct run*)pa;

  acquire(&kmem.lock);
  
  // Check for double free by scanning free list
  if(DEBUG_KALLOC) {
    struct run *check = kmem.freelist;
    while(check) {
      if(check == r) {
        printf("[ERROR] kfree: double free detected at 0x%p\n", pa);
        panic("double free");
      }
      check = check->next;
    }
  }
  
  r->next = kmem.freelist;
  kmem.freelist = r;
  kmem.free_pages++;
  kmem.total_freed++;
  if(kmem.current_allocated > 0) {
    kmem.current_allocated--;
  }
  
  release(&kmem.lock);
  
  if(DEBUG_MEMORY_STATS && (kmem.total_freed % 100 == 0)) {
    printf("[STATS] kfree: total freed = %d, current allocated = %d\n", 
           kmem.total_freed, kmem.current_allocated);
  }
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.freelist;
  if(r) {
    kmem.freelist = r->next;
    kmem.free_pages--;
    kmem.total_allocated++;
    kmem.current_allocated++;
  }
  release(&kmem.lock);

  if(r) {
    if(POISON_ALLOC_PAGES) {
      memset((char*)r, 5, PGSIZE); // fill with junk
    }
    
    if(DEBUG_KALLOC) {
      printf("[DEBUG] kalloc: allocated page at 0x%p\n", r);
    }
    
    if(DEBUG_MEMORY_STATS && (kmem.total_allocated % 100 == 0)) {
      printf("[STATS] kalloc: total allocated = %d, free pages = %d\n", 
             kmem.total_allocated, kmem.free_pages);
    }
  } else {
    printf("[ERROR] kalloc: out of memory! free_pages = %d\n", kmem.free_pages);
    if(DEBUG_KALLOC) {
      print_memory_stats();
    }
  }
  
  return (void*)r;
}

// Print memory allocation statistics
void
print_memory_stats(void)
{
  acquire(&kmem.lock);
  printf("=== Memory Allocation Statistics ===\n");
  printf("Free pages: %d\n", kmem.free_pages);
  printf("Total allocated: %d pages\n", kmem.total_allocated);
  printf("Total freed: %d pages\n", kmem.total_freed);
  printf("Currently allocated: %d pages\n", kmem.current_allocated);
  printf("Memory usage: %d KB\n", kmem.current_allocated * PGSIZE / 1024);
  
  // Calculate total memory
  uint64 total_pages = ((uint64)PHYSTOP - (uint64)end) / PGSIZE;
  printf("Total available: %d pages (%d KB)\n", 
         total_pages, total_pages * PGSIZE / 1024);
  
  if(total_pages > 0) {
    printf("Usage percentage: %d%%\n", 
           (kmem.current_allocated * 100) / total_pages);
  }
  
  release(&kmem.lock);
  printf("==================================\n");
}

// Get current memory usage
uint64
get_free_memory(void)
{
  uint64 free_pages;
  acquire(&kmem.lock);
  free_pages = kmem.free_pages;
  release(&kmem.lock);
  return free_pages * PGSIZE;
}

// Check if we're running low on memory
int
low_memory(void)
{
  acquire(&kmem.lock);
  int low = kmem.free_pages < 10; // Less than 10 pages = low memory
  release(&kmem.lock);
  return low;
}

// Validate that a pointer looks like it could be a valid kalloc'd page
int
valid_kalloc_ptr(void *pa)
{
  if(!pa) return 0;
  if(((uint64)pa % PGSIZE) != 0) return 0;
  if((char*)pa < end) return 0;
  if((uint64)pa >= PHYSTOP) return 0;
  return 1;
}