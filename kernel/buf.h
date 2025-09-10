struct buf {
  int valid;   // has data been read from disk?
  int disk;    // does disk "own" buf?
  uint dev;    // device number
  uint blockno; // block number on device
  struct sleeplock lock; // protects fields below
  uint refcnt;  // reference count
  struct buf *prev; // LRU cache list
  struct buf *next; // LRU cache list
  uchar data[BSIZE]; // actual data
};