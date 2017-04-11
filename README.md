# MiniRel
MiniRel is a minimal single-user relational database management system (DBMS) group project for *Advanced Database* course, Spring 2017, Seoul National University, implemented in ANSI C.

## Team
- Antoine Jubin
- Paul Chabert
- Patric Steiner

# Software Architecture
The Software is composed of five layers: the buffer pool (BF) layer, the paged file (PF) layer, the heap file (HF) layer, the access method (AM) layer, and the front-end (FE) layer. The BF layer is the lowest in the layered architecture of MiniRel. The HF and AM layers are located at the same architectural level on top of the PF layer, with the FE layer being the highest level.

## Buffer Pool Layer
### Finding a Victim
The two interface routines BF_GetBuf and BF_AllocBuf both require a free page in the buffer pool. If there is already a free page available, then we can just pop the head of the freelist and use this one. If there are no free pages available though, we need to find a victim. To do this, we implemented the helper function `int BF_FlushPage(LRU* lru)`. This function tries to find a victim page, a page that is in the bufferpool but not currently pinned. If one is found, the content of the page (in case it is dirty) is written to the disk. After that, the page is removed from the hashtable and added to the freelist.

### Writing Page Content to Disk
There are only two functions that actually write to the disk, namely `int BF_FlushPage(LRU* lru)` and `int BF_FlushBuf(int fd)`. To write page content to the disk, the function `pwrite(int fd, const void *buf, size_t count, off_t offset)` is used, which perfectly fits our needs. We can just pass the unix file descriptor and our pagecontent, the amount of bytes to be written (in our case PAGE_SIZE = 4096) and the offset, where we want to start writing. This offset can easily be calculated by multiplying the number of the page with PAGE_SIZE. Note that the first page of each file is always the header, indicating the amount of pages in the file. The header is always exactly of size PAGE_SIZE, regardless of how big the actual number is.

### Reading page content from disk
Reading a page from disk is handled in `int BF_GetBuf(BFreq bq, PFpage** fpage)`. In this interface routine, the function `pread(int fd, void *buf, size_t count, off_t offset)` is used, the counterpart to `pwrite` that is described in the section above.

## Paged File Layer
TODO

## Heap File LAyer
TODO

## Access Method Layer
TODO

## Frontend Layer
TODO
