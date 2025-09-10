#include "types.h"

// Debug configuration
#define DEBUG_STRING_OPS 0  // Set to 1 for verbose debugging
#define DEBUG_MEMORY_OPS 1  // Set to 1 for memory operation debugging
#define ENABLE_BOUNDS_CHECK 1  // Enable bounds checking
#define ENABLE_NULL_CHECK 1    // Enable null pointer checking

// String operation statistics
static struct {
  uint64 memset_calls;
  uint64 memcmp_calls;
  uint64 memmove_calls;
  uint64 memcpy_calls;
  uint64 strncmp_calls;
  uint64 strncpy_calls;
  uint64 safestrcpy_calls;
  uint64 strlen_calls;
  uint64 null_pointer_errors;
  uint64 bounds_errors;
} string_stats = {0};

// Helper function to validate pointers
static int
validate_ptr(const void *ptr, const char *func_name)
{
  if(ENABLE_NULL_CHECK && ptr == 0) {
    printf("[ERROR] %s: null pointer\n", func_name);
    string_stats.null_pointer_errors++;
    return 0;
  }
  return 1;
}

// Helper function to validate memory bounds
static int
validate_bounds(uint n, uint max_size, const char *func_name)
{
  if(ENABLE_BOUNDS_CHECK && n > max_size) {
    printf("[WARNING] %s: size %d exceeds reasonable limit %d\n", 
           func_name, n, max_size);
    string_stats.bounds_errors++;
    return 0;
  }
  return 1;
}

void*
memset(void *dst, int c, uint n)
{
  string_stats.memset_calls++;
  
  if(!validate_ptr(dst, "memset"))
    return dst;
    
  if(!validate_bounds(n, 1024*1024, "memset"))  // 1MB limit
    return dst;
  
  if(DEBUG_MEMORY_OPS) {
    printf("[DEBUG] memset: dst=0x%p c=0x%x n=%d\n", dst, c & 0xff, n);
  }
  
  char *cdst = (char *) dst;
  int i;
  for(i = 0; i < n; i++){
    cdst[i] = c;
  }
  
  if(DEBUG_MEMORY_OPS && n > 1000) {
    printf("[DEBUG] memset: completed large operation (%d bytes)\n", n);
  }
  
  return dst;
}

int
memcmp(const void *v1, const void *v2, uint n)
{
  const uchar *s1, *s2;
  
  string_stats.memcmp_calls++;
  
  if(!validate_ptr(v1, "memcmp") || !validate_ptr(v2, "memcmp"))
    return 0;
    
  if(!validate_bounds(n, 1024*1024, "memcmp"))
    return 0;

  if(DEBUG_MEMORY_OPS) {
    printf("[DEBUG] memcmp: v1=0x%p v2=0x%p n=%d\n", v1, v2, n);
  }

  s1 = v1;
  s2 = v2;
  uint compared = 0;
  while(n-- > 0){
    if(*s1 != *s2) {
      int result = *s1 - *s2;
      if(DEBUG_MEMORY_OPS) {
        printf("[DEBUG] memcmp: difference at offset %d: 0x%x vs 0x%x (result=%d)\n", 
               compared, *s1, *s2, result);
      }
      return result;
    }
    s1++, s2++;
    compared++;
  }

  if(DEBUG_MEMORY_OPS) {
    printf("[DEBUG] memcmp: buffers equal (%d bytes)\n", compared);
  }

  return 0;
}

void*
memmove(void *dst, const void *src, uint n)
{
  const char *s;
  char *d;

  string_stats.memmove_calls++;
  
  if(!validate_ptr(dst, "memmove") || !validate_ptr(src, "memmove"))
    return dst;
    
  if(!validate_bounds(n, 1024*1024, "memmove"))
    return dst;

  if(n == 0)
    return dst;

  if(DEBUG_MEMORY_OPS) {
    printf("[DEBUG] memmove: dst=0x%p src=0x%p n=%d\n", dst, src, n);
  }
  
  // Check for overlap
  char *dst_end = (char*)dst + n;
  const char *src_end = (const char*)src + n;
  int overlap = ((char*)dst < src_end) && (dst_end > (char*)src);
  
  if(DEBUG_MEMORY_OPS && overlap) {
    printf("[DEBUG] memmove: overlapping regions detected\n");
  }
  
  s = src;
  d = dst;
  if(s < d && s + n > d){
    // Copy backwards to handle overlap
    s += n;
    d += n;
    while(n-- > 0)
      *--d = *--s;
    if(DEBUG_MEMORY_OPS) {
      printf("[DEBUG] memmove: used backward copy\n");
    }
  } else {
    // Copy forwards
    while(n-- > 0)
      *d++ = *s++;
    if(DEBUG_MEMORY_OPS) {
      printf("[DEBUG] memmove: used forward copy\n");
    }
  }

  return dst;
}

// memcpy exists to placate GCC. Use memmove.
void*
memcpy(void *dst, const void *src, uint n)
{
  string_stats.memcpy_calls++;
  
  if(DEBUG_MEMORY_OPS) {
    printf("[DEBUG] memcpy: redirecting to memmove (dst=0x%p src=0x%p n=%d)\n", 
           dst, src, n);
  }
  
  // Check if regions might overlap - memcpy assumes they don't
  if(ENABLE_BOUNDS_CHECK && dst && src) {
    char *dst_end = (char*)dst + n;
    const char *src_end = (const char*)src + n;
    int overlap = ((char*)dst < src_end) && (dst_end > (char*)src);
    
    if(overlap) {
      printf("[WARNING] memcpy: potential overlap detected, using memmove\n");
    }
  }
  
  return memmove(dst, src, n);
}

int
strncmp(const char *p, const char *q, uint n)
{
  string_stats.strncmp_calls++;
  
  if(!validate_ptr(p, "strncmp") || !validate_ptr(q, "strncmp"))
    return 0;
    
  if(!validate_bounds(n, 1024, "strncmp"))
    return 0;

  if(DEBUG_STRING_OPS) {
    printf("[DEBUG] strncmp: p=0x%p q=0x%p n=%d\n", p, q, n);
  }

  uint pos = 0;
  while(n > 0 && *p && *p == *q) {
    n--, p++, q++, pos++;
  }
  
  if(n == 0) {
    if(DEBUG_STRING_OPS) {
      printf("[DEBUG] strncmp: reached limit, strings equal up to %d chars\n", pos);
    }
    return 0;
  }
  
  int result = (uchar)*p - (uchar)*q;
  if(DEBUG_STRING_OPS) {
    printf("[DEBUG] strncmp: difference at pos %d: '%c'(0x%x) vs '%c'(0x%x) = %d\n", 
           pos, *p ? *p : '\\0', *p, *q ? *q : '\\0', *q, result);
  }
  
  return result;
}

char*
strncpy(char *s, const char *t, int n)
{
  char *os;

  string_stats.strncpy_calls++;
  
  if(!validate_ptr(s, "strncpy") || !validate_ptr(t, "strncpy"))
    return s;
    
  if(n < 0) {
    printf("[ERROR] strncpy: negative length %d\n", n);
    return s;
  }
  
  if(!validate_bounds(n, 1024, "strncpy"))
    return s;

  if(DEBUG_STRING_OPS) {
    printf("[DEBUG] strncpy: s=0x%p t=0x%p n=%d\n", s, t, n);
  }

  os = s;
  int copied = 0;
  while(n-- > 0 && (*s++ = *t++) != 0) {
    copied++;
  }
  
  // Pad with zeros
  int padded = 0;
  while(n-- > 0) {
    *s++ = 0;
    padded++;
  }
  
  if(DEBUG_STRING_OPS) {
    printf("[DEBUG] strncpy: copied %d chars, padded %d zeros\n", copied, padded);
  }
  
  return os;
}

// Like strncpy but guaranteed to NUL-terminate.
char*
safestrcpy(char *s, const char *t, int n)
{
  char *os;

  string_stats.safestrcpy_calls++;
  
  if(!validate_ptr(s, "safestrcpy") || !validate_ptr(t, "safestrcpy"))
    return s;
    
  if(n <= 0) {
    printf("[ERROR] safestrcpy: invalid length %d\n", n);
    return s;
  }
  
  if(!validate_bounds(n, 1024, "safestrcpy"))
    return s;

  if(DEBUG_STRING_OPS) {
    printf("[DEBUG] safestrcpy: s=0x%p t=0x%p n=%d\n", s, t, n);
  }

  os = s;
  int copied = 0;
  while(--n > 0 && (*s++ = *t++) != 0) {
    copied++;
  }
  *s = 0;  // Always null-terminate
  
  if(DEBUG_STRING_OPS) {
    printf("[DEBUG] safestrcpy: copied %d chars, null-terminated\n", copied);
  }
  
  // Check if source string was truncated
  if(n == 0 && *t != 0) {
    printf("[WARNING] safestrcpy: source string truncated\n");
  }
  
  return os;
}

int
strlen(const char *s)
{
  int n;

  string_stats.strlen_calls++;
  
  if(!validate_ptr(s, "strlen"))
    return 0;

  if(DEBUG_STRING_OPS) {
    printf("[DEBUG] strlen: s=0x%p\n", s);
  }

  for(n = 0; s[n]; n++) {
    // Safety check to prevent infinite loops on non-terminated strings
    if(ENABLE_BOUNDS_CHECK && n > 4096) {
      printf("[ERROR] strlen: string too long (>4096 chars), possible missing null terminator\n");
      return n;
    }
  }
  
  if(DEBUG_STRING_OPS) {
    printf("[DEBUG] strlen: length = %d\n", n);
  }
  
  return n;
}

// Additional utility functions for debugging

// Print string operation statistics
void
print_string_stats(void)
{
  printf("=== String Operation Statistics ===\n");
  printf("memset: %d calls\n", string_stats.memset_calls);
  printf("memcmp: %d calls\n", string_stats.memcmp_calls);
  printf("memmove: %d calls\n", string_stats.memmove_calls);
  printf("memcpy: %d calls\n", string_stats.memcpy_calls);
  printf("strncmp: %d calls\n", string_stats.strncmp_calls);
  printf("strncpy: %d calls\n", string_stats.strncpy_calls);
  printf("safestrcpy: %d calls\n", string_stats.safestrcpy_calls);
  printf("strlen: %d calls\n", string_stats.strlen_calls);
  printf("Errors - null ptr: %d, bounds: %d\n", 
         string_stats.null_pointer_errors, string_stats.bounds_errors);
  printf("=================================\n");
}

// Reset statistics
void
reset_string_stats(void)
{
  string_stats.memset_calls = 0;
  string_stats.memcmp_calls = 0;
  string_stats.memmove_calls = 0;
  string_stats.memcpy_calls = 0;
  string_stats.strncmp_calls = 0;
  string_stats.strncpy_calls = 0;
  string_stats.safestrcpy_calls = 0;
  string_stats.strlen_calls = 0;
  string_stats.null_pointer_errors = 0;
  string_stats.bounds_errors = 0;
}

// Safe string comparison that handles null pointers
int
safe_strcmp(const char *s1, const char *s2)
{
  if(!s1 && !s2) return 0;
  if(!s1) return -1;
  if(!s2) return 1;
  
  while(*s1 && *s2 && *s1 == *s2) {
    s1++;
    s2++;
  }
  return *s1 - *s2;
}

// Memory dump utility for debugging
void
memdump(const void *ptr, uint size, const char *label)
{
  if(!validate_ptr(ptr, "memdump"))
    return;
    
  printf("=== Memory Dump: %s ===\n", label ? label : "unknown");
  printf("Address: 0x%p, Size: %d bytes\n", ptr, size);
  
  const unsigned char *p = (const unsigned char *)ptr;
  for(uint i = 0; i < size; i++) {
    if(i % 16 == 0) {
      printf("\n%04x: ", i);
    }
    printf("%02x ", p[i]);
  }
  printf("\n====================\n");
}