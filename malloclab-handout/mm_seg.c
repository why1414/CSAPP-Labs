/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "ateam",
    /* First member's full name */
    "Harry Bovik",
    /* First member's email address */
    "bovik@cs.cmu.edu",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

/* debug macro */
// #define DEBUG
#ifdef DEBUG
    #define DEBUG_PRINT(format, ...) printf("[DEBUG] File: "__FILE__", Line: %d, "format"\r\n",__LINE__, ##__VA_ARGS__)
    #define HEAP_CHECK() mm_check()
#else
    #define DEBUG_PRINT(X,...)
    #define HEAP_CHECK()
#endif


/* $begin mallocmacros */
/* Basic constrants and macros */
#define WSIZE 4
#define DSIZE 8
#define CHUNKSIZE (1<<12) /* Extend heap by this amount (bytes) 4k (1 page) */
#define ALIGNMENT DSIZE


#define MAX(x, y) ((x) > (y) ? (x) : (y))

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)  
#define ADJUST(size) (ALIGN(size) + DSIZE)
/* Pack a size and allocated bit into a word */
#define PACK(size, alloc) ((size) | (alloc))
/* Reand and write a word at address p */
#define GET(p)      (*(unsigned int *)(p))
#define PUT(p, val) (*(unsigned int *)(p) = (val))
/* Read the size and allocated fields from address p */
#define GET_SIZE(p)  (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)
/* Given block ptr bp, compute address of its header and footer */
#define HDRP(bp)  ((char*)(bp) - WSIZE)
#define FTRP(bp)  ((char*)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)
/* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp) ((char*)(bp) + GET_SIZE(((char*)(bp) - WSIZE)))
#define PREV_BLKP(bp) ((char*)(bp) - GET_SIZE(((char*)(bp) - DSIZE)))
/* Given block ptr bp, compute address of pred and succ  */
#define PRED(bp)      ((char*)(bp))
#define SUCC(bp)      ((char*)(bp) + WSIZE)
#define PRED_BLKP(bp) (GET(PRED(bp)))
#define SUCC_BLKP(bp) (GET(SUCC(bp)))
/* compute the offset in the listp with enough space for asize */
#define LISTARR_MAX_LEN 9


/* $end mallocmacros */

/* Global variables */
static char *heap_listp = 0;  /* Pointer to first block */  
static char *listArray = 0;       /* Pointer to free block list */
/* Function prototypes for internal helper routines */
static void *extend_heap(size_t words);
static void *coalesce(void *bp);
static void *find_fit(size_t asize);
static void place(void *bp, size_t asize);
static void addBlock(void *bp);
static void deleteBlock(void *listptr);
static int Index(size_t size);
static int mm_check();
/* 
 * mm_init - initialize the malloc package.
 */

int mm_init(void)
{
    /* Create the initial empty heap */
    DEBUG_PRINT("\nmm_init ing...");
    if ((heap_listp = mem_sbrk(12*WSIZE)) == (void*)-1)
        return -1;
    /* initialize ptr list to free blocks */
    PUT(heap_listp+0*WSIZE, NULL);	//{16~31}
	PUT(heap_listp+1*WSIZE, NULL);	//{32~63}
	PUT(heap_listp+2*WSIZE, NULL);	//{64~127}
	PUT(heap_listp+3*WSIZE, NULL);	//{128~255}
	PUT(heap_listp+4*WSIZE, NULL);	//{256~511}
	PUT(heap_listp+5*WSIZE, NULL);	//{512~1023}
	PUT(heap_listp+6*WSIZE, NULL);	//{1024~2047}
	PUT(heap_listp+7*WSIZE, NULL);	//{2048~4095}
	PUT(heap_listp+8*WSIZE, NULL);	//{4096~inf}

    PUT(heap_listp + (9*WSIZE), PACK(DSIZE, 1)); /* Prologue header */ 
    PUT(heap_listp + (10*WSIZE), PACK(DSIZE, 1)); /* Prologue footer */ 
    PUT(heap_listp + (11*WSIZE), PACK(0, 1));     /* Epilogue header */
   
    listArray = heap_listp;
    heap_listp += 10*WSIZE;                     //line:vm:mm:endinit  
    
    /* Extend the empty heap with a free block of CHUNKSIZE bytes */
    if (extend_heap(CHUNKSIZE/WSIZE) == NULL) 
	    return -1;
    DEBUG_PRINT("init succeed");
    return 0;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */

inline static int Index(size_t size){
	assert(size > 15);
    int ind = 0;
	if(size >= 4096)
		return 8;
	
	size = size>>5;
	while(size){
		size = size>>1;
		ind++;
	}
	return ind;
}

void *mm_malloc(size_t size)
{
    DEBUG_PRINT("mm_malloc %d",size);
    HEAP_CHECK();
    size_t asize; /* Adjusted block size */
    size_t extendsize; /* Amount to extend heap if no fit */
    char *bp;
    if (heap_listp == 0){
        mm_init();
    }
    /* Ignore spurious requests */
    if (size == 0)
        return NULL;
    /* Adjust block size to include overhead and alignment reqs. */
    asize = ADJUST(size);
    /* Search the free list for a fit */
    if ((bp = find_fit(asize)) != NULL){
        place(bp, asize);
        return bp;
    }
    /* No fit found. Get more memory and place the block */
    extendsize = MAX(asize, CHUNKSIZE);
    if ((bp = extend_heap(extendsize/WSIZE)) == NULL)
        return NULL;
    place(bp, asize);
    DEBUG_PRINT("mm_malloc end");
    return bp;
}

static void *find_fit(size_t asize){
    /* first fit */
    DEBUG_PRINT("find_fit starting...");
    size_t idx=Index(asize);
    void *lp = listArray+idx*WSIZE;
    void *bp;
    while(idx < LISTARR_MAX_LEN){
        bp = GET(listArray+idx*WSIZE);
        while(bp){
            if(GET_SIZE(HDRP(bp)) >= asize) /* found fit block */
                return bp;
            bp = SUCC_BLKP(bp);
        }
        idx++;
    }
    return NULL;

}

static void place(void *bp, size_t asize){
    DEBUG_PRINT("place starting...");
    /* 将该block 从 listArray 中分离出来 */
    deleteBlock(bp);
    size_t size = GET_SIZE(HDRP(bp));
    if(size-asize >= 2*DSIZE){
        PUT(HDRP(bp), PACK(asize, 1));
        PUT(FTRP(bp), PACK(asize, 1));
        PUT(HDRP(NEXT_BLKP(bp)), PACK(size-asize, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size-asize, 0));
        PUT(PRED(NEXT_BLKP(bp)), NULL);
        PUT(SUCC(NEXT_BLKP(bp)), NULL);
        /*将剩余的空闲块， 插入空闲链表中*/
        addBlock(NEXT_BLKP(bp));
    }
    else {
        PUT(HDRP(bp), PACK(size, 1));
        PUT(FTRP(bp), PACK(size, 1));
    }
    DEBUG_PRINT("place end");
}

static void deleteBlock(void *bp){
    DEBUG_PRINT("delete starting... block size:%d", GET_SIZE(HDRP(bp)));
    int idx = Index(GET_SIZE(HDRP(bp)));
    void *lp = listArray+idx*WSIZE;
    /* head block of list */  
    if(PRED_BLKP(bp) == NULL){
        DEBUG_PRINT("code is here");
        PUT(lp, SUCC_BLKP(bp));
        DEBUG_PRINT("code is here");
    } 
    else {
        PUT(SUCC(PRED_BLKP(bp)), SUCC_BLKP(bp));
        DEBUG_PRINT("code is here");
    }
    if(SUCC_BLKP(bp))
        PUT(PRED(SUCC_BLKP(bp)), PRED_BLKP(bp));
    DEBUG_PRINT("delete end");
    HEAP_CHECK();
}

static void addBlock(void *bp){
    DEBUG_PRINT("add block...");
    PUT(PRED(bp), NULL);
    PUT(SUCC(bp), NULL);
    size_t asize = GET_SIZE(HDRP(bp));
    size_t idx = Index(asize);
    void *lp = listArray+idx*WSIZE;
    /* LIFO */
    if (GET(lp)){
        PUT(PRED(GET(lp)), bp);
    }
    PUT(SUCC(bp), GET(lp));
    PUT(lp, bp);
    // HEAP_CHECK();
    DEBUG_PRINT("add end");
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *bp)
{
  
    if (bp == 0)
        return;
    size_t size = GET_SIZE(HDRP(bp));
    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    PUT(PRED(bp), NULL);
    PUT(SUCC(bp), NULL);
    /* Coalesce if the previous block or next block was free */
    DEBUG_PRINT("mm_free: %d", size);
    bp = coalesce(bp);
    addBlock(bp);
}

/*
 * coalesce - Boundary tag coalescing. Return ptr to coalesced block
 */
static void *coalesce(void *bp){
    DEBUG_PRINT("coalesece ing...");
    size_t prev_alloc = GET_ALLOC(HDRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));
    /* case 1: both allocated */
    if(prev_alloc && next_alloc) {
        return bp;
    }
    /* case 2:  */
    else if(prev_alloc && !next_alloc){
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        deleteBlock(NEXT_BLKP(bp));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
    }
    /* case 3: */
    else if(!prev_alloc && next_alloc){
        size += GET_SIZE(FTRP(PREV_BLKP(bp)));
        deleteBlock(PREV_BLKP(bp));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }
    /* case 4: */
    else {
        size += GET_SIZE(HDRP(NEXT_BLKP(bp))) + 
            GET_SIZE(FTRP(PREV_BLKP(bp)));
        deleteBlock(PREV_BLKP(bp));
        deleteBlock(NEXT_BLKP(bp));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }
    return bp;
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    DEBUG_PRINT("realloc starting...");
    void *newptr;
    size_t copySize;
    size_t oldsize ;

    if (ptr == NULL)
        return mm_malloc(size);
    if (size == 0){
        mm_free(ptr);
        return 0;
    }
    /* move data */
    oldsize = GET_SIZE(HDRP(ptr));
    newptr = mm_malloc(size);
    if (newptr == NULL)
        return 0;
    copySize = -MAX(-size,-oldsize);
    memcpy(newptr,ptr,copySize);
    /*free old block*/
    mm_free(ptr);

    return newptr;
}

/* 
 * extend_heap - Extend heap with free block and return its block pointer
 */
/* $begin mmextendheap */
static void *extend_heap(size_t words) 
{
    DEBUG_PRINT("extend heap");
    void *bp;
    size_t size;

    /* Allocate an even number of words to maintain alignment */
    size = (words % 2) ? (words+1) * WSIZE : words * WSIZE; //line:vm:mm:beginextend
    if ((long)(bp = mem_sbrk(size)) == -1)  
	    return NULL;                                        //line:vm:mm:endextend

    /* Initialize free block header/footer and the epilogue header */
    PUT(HDRP(bp), PACK(size, 0));         /* Free block header */   //line:vm:mm:freeblockhdr
    PUT(FTRP(bp), PACK(size, 0));         /* Free block footer */   //line:vm:mm:freeblockftr
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1)); /* New epilogue header */ //line:vm:mm:newepihdr
    PUT(PRED(bp), NULL);
    PUT(SUCC(bp), NULL);
    /* Coalesce if the previous block was free */
    bp = coalesce(bp);
    addBlock(bp);
    return bp;                                          //line:vm:mm:returnblock
}
/* $end mmextendheap */

/* mm_check scans listArray ans check it for consistency */
static int mm_check(){
    int ret=1;
    void *bp;
    int base = 16;
    for(int i=0;i<LISTARR_MAX_LEN;i++){
        
        printf("size class: %d >=%d\n",i,base);
        bp = GET(listArray+i*WSIZE);
        while(bp){
            printf("%d-->",GET_SIZE(HDRP(bp)));
            if(SUCC_BLKP(bp))
                if(PRED_BLKP(SUCC_BLKP(bp)) != bp)
                    printf("list error!\n");
            bp = SUCC_BLKP(bp);
        }
        printf("NULL\n");
        base *=2;
    }
    return ret;
}














