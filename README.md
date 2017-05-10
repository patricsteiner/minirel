# MiniRel
MiniRel is a minimal single-user relational database management system (DBMS) group project for *Advanced Database* course, Spring 2017, Seoul National University, implemented in ANSI C.

## Team
- Antoine Jubin
- Paul Chabert
- Patric Steiner

# Software Architecture
The Software is composed of five layers: the buffer pool (BF) layer, the paged file (PF) layer, the heap file (HF) layer, the access method (AM) layer, and the front-end (FE) layer. The BF layer is the lowest in the layered architecture of MiniRel. The HF and AM layers are located at the same architectural level on top of the PF layer, with the FE layer being the highest level.

## Buffer Pool Layer
There are several tricky tasks in the BF layer to guarantee an effective and efficient buffer management. The following sections elaborate on our specific solution for different problems

### Finding a Victim
The two interface routines BF_GetBuf and BF_AllocBuf both require a free page in the buffer pool. If there is already a free page available, then we can just pop the head of the freelist and use this one. If there are no free pages available though, we need to find a victim. To do this, we implemented the helper function `int BF_FlushPage(LRU* lru)`. This function tries to find a victim page, a page that is in the bufferpool but not currently pinned. If one is found, the content of the page (in case it is dirty) is written to the disk. After that, the page is removed from the hashtable and added to the freelist.

### Writing Page Content to Disk
There are only two functions that actually write to the disk, namely `int BF_FlushPage(LRU* lru)` and `int BF_FlushBuf(int fd)`. To write page content to the disk, the function `pwrite(int fd, const void *buf, size_t count, off_t offset)` is used, which perfectly fits our needs. We can just pass the unix file descriptor and our pagecontent, the amount of bytes to be written (in our case PAGE_SIZE = 4096) and the offset, where we want to start writing. This offset can easily be calculated by multiplying the number of the page with PAGE_SIZE. Note that the first page of each file is always the header, indicating the amount of pages in the file. The header is always exactly of size PAGE_SIZE, regardless of how big the actual number is.

### Reading page content from disk
Reading a page from disk is handled in `int BF_GetBuf(BFreq bq, PFpage** fpage))`. In this interface routine, the function `pread(int fd, void *buf, size_t count, off_t offset)` is used, the counterpart to `pwrite` that is described in the section above.

### Error Handling
In order to cover all kind of errors, some error codes as been added in the file bfUtils.h. Also, to be more comprehensive, the function `void BF_ErroHandler(int error)` was implemented. It prints a clear message and exits the program after a certain error occurs. This function is used in PF layer, when a primitive of BF layer is called and its return code is not BFE_OK then the BF_ErrorHandler is called with the return code as parameter.

### Page Retrieval
To quickly check if a page is in the buffer pool or not, we use a hashtable that contains entries for all pages currently in the pool. This has the advantage that we do not need to scan through the LRU list for lookup but can instead get the result in constant time.
Drawback is, of course, that some addidional memory is required. The additional memory is minimal and well worth the cost though.

To retrieve a page from the hashtable, a hash out of the file descriptor (not the unix file descriptor, but the file desriptor used in PF layer) and the page number is calculated. To achieve a uniform distribution for the hashcodes, we use the universal hashfunction `h(x) = ((ax + b) mod p) mod m`, where a and b are arbitrary integers (123 and 87), p is a prime number (31), and m the size of the hashtable. x is the multiplication of file descriptor and pagenum, whereas 13 and 17 respectively are added to the two components, to avoid one of them being zero (numbers are chosen arbitrarily).

## Paged File Layer
The implementation of the Paged File Layer is pretty straight forward, nothing too fancy going on here. We just use the underlying functions in the BF layer to implement the specified functionality.

One point worth mentioning though is, that for every file created in PF layer, its header is written to the very first page of the file. In order to achieve this the page 0 is allocated as soon as a file is created and immediately written using the `memcpy` function.
Consequently each file has always at least one page (pagenumber 0).


## Heap File LAyer

### File organization
Each file is now composed of the following pages :
- page 0 : Page File header, store the total number of pages in the file
- page 1 : Heap File header, store the record size, number of record per page, number of pages in the file (min 2), number of free pages in the file, and a page directory (cf. **Page Directory**)
- page 2 to n : data pages, with a bitmap, the number of occuped slots, the data

### Page Directory
To insert an entry without accessing all the pages sequentially, we create a page directory. It must keep track of data pages that still have some free slots. In order to fulfill its mission, we create an array that tell if a page is full or not, avoiding to retrieve pages that are full.

There are `PF_PAGE_SIZE - 4*sizeof(int) - sizeof(char)` pages that can be stored on the header page to retrieve data.
To avoid a size restriction, once this array is full, we add another header page instead of a data page at index `PF_PAGE_SIZE - 4*sizeof(int) - sizeof(char) + 2`.

### Datapages
We use an unpacked bitmap to store the data on every page, so the first bytes of the datapages are reserved for the bitmap, then the number of full slots.

**example:** 

bitmap:	1011 0100	0000 0000	

Occuped slots :  4 

Data : 

| record | empty | record | record | empty | record | empty | empty |empty | empty | empty| empty | empty | empty | empty | empty |



## Access Method Layer
TODO

## Frontend Layer
TODO
