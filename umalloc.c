#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/param.h"

// Memory allocator by Kernighan and Ritchie,
// The C programming Language, 2nd ed.  Section 8.7.

typedef long Align;

union header {
  struct {
    union header *ptr;
    uint size;
  } s;
  Align x;
};

typedef union header Header;

static Header base;
static Header *freep;

void
free(void *ap)
{
  Header *bp, *p;

  // Fixed: Added null pointer check to prevent segfaults
  if(ap == 0)
    return;
    
  bp = (Header*)ap - 1;
  
  // Find the correct position to insert this block
  for(p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
    if(p >= p->s.ptr && (bp > p || bp < p->s.ptr))
      break;
  
  // Coalesce with next block if possible
  if(bp + bp->s.size == p->s.ptr){
    bp->s.size += p->s.ptr->s.size;
    bp->s.ptr = p->s.ptr->s.ptr;
  } else
    bp->s.ptr = p->s.ptr;
    
  // Coalesce with previous block if possible
  if(p + p->s.size == bp){
    p->s.size += bp->s.size;
    p->s.ptr = bp->s.ptr;
  } else
    p->s.ptr = bp;
    
  freep = p;
}

static Header*
morecore(uint nu)
{
  char *p;
  Header *hp;

  if(nu < 4096)
    nu = 4096;
  p = sbrk(nu * sizeof(Header));
  
  // Fixed: Changed to explicit cast instead of undefined SBRK_ERROR
  if(p == (char*)-1)
    return 0;
    
  hp = (Header*)p;
  hp->s.size = nu;
  free((void*)(hp + 1));
  return freep;
}

void*
malloc(uint nbytes)
{
  Header *p, *prevp;
  uint nunits;

  // Fixed: Added check for zero-byte allocation
  if(nbytes == 0)
    return 0;

  nunits = (nbytes + sizeof(Header) - 1)/sizeof(Header) + 1;
  
  // Initialize free list if first time
  if((prevp = freep) == 0){
    base.s.ptr = freep = prevp = &base;
    base.s.size = 0;
  }
  
  // Search for a block of adequate size
  for(p = prevp->s.ptr; ; prevp = p, p = p->s.ptr){
    if(p->s.size >= nunits){
      // Found a block large enough
      if(p->s.size == nunits)
        // Exactly the right size, remove from free list
        prevp->s.ptr = p->s.ptr;
      else {
        // Too big, split the block
        p->s.size -= nunits;
        p += p->s.size;
        p->s.size = nunits;
      }
      freep = prevp;
      return (void*)(p + 1);
    }
    
    // Wrapped around free list without finding adequate block
    if(p == freep)
      if((p = morecore(nunits)) == 0)
        return 0;
  }
}