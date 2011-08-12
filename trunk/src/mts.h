/* Copyright (c) 1997, 2005 Newcode Technology, Inc.  All rights reserved.  */ 
#ifndef MTS_MTS_H
#define MTS_MTS_H 1

#ifdef __cplusplus
extern "C" {
#endif

/* create np number of virtual heaps. also controlled by environment
 * variable MTS_INIT_THREAD_HEAPS=N
 * Note: in order to force the number of heaps to be less than the number
 * of processors, the env variable has to be used. calling the 
 * mts_init_thread_heaps function is too late.
 */
extern void mts_init_thread_heaps( int np );

/* set the minimum chunk of memory that MTS grabs from the OS */
extern void mts_sys_alloc_size ( long size );

/* returns the number of virtual heaps. 
 * Note: the initial N value is 1. Only after the first 
 * thread is run will this function report the final number of heaps which
 * is either the env variable MTS_INIT_THREAD_HEAPS or the result of calling 
 * mts_init_thread_heaps or the number of cpu's if it is greater than 5
*/
extern int mts_num_heaps ();

/* return the index of the heap that the current thread is using */
extern int mts_which_heap();


/* heapsize functions return the size of the virtual heap(s):
 *
 * mts_heapsizes:		sum of all heap memory gotten from the OS
 * mts_current_heapsize:	size of virtual heap this thread is using
 * mts_heapsize:		size of a particular heap indexed 
 *				0 to (mts_num_heaps() -1)
*/
extern unsigned long mts_heapsizes();
extern unsigned long mts_current_heapsize ();
extern unsigned long mts_heapsize ( int heap );

/* heapbytecount functions return current malloc'ed memory. 
 * NOTE: more than one thread can be bound to a heap, therefor these stats
 * can reflect the allocations of more than one single thread.
 *
 * mts_heapbytecounts:		sum of all currently malloced memory 
 * mts_current_heapbytecount:	malloc'ed memory in heap that thread is using 
 * mts_heapbytecount:		malloc'ed memory of a particular heap indexed 
 *				0 to (mts_num_heaps() -1)
*/
extern unsigned long mts_heapbytecounts ();
extern unsigned long mts_current_heapbytecount();
extern unsigned long mts_heapbytecount ( int heap );

struct mts_stats {			
   unsigned long heap_size;	        /* total heap size as requested from OS */
   unsigned long byte_count;		/* current byte usage */
   unsigned long small_malloc;		/* number of small (0 to 256) malloc calls */
   unsigned long medium_malloc;		/* number of medium ( 256 to 32k) malloc calls */
   unsigned long large_malloc;		/* number of large (greater than 32k) malloc calls */
   unsigned long small_free;		/* number of small free calls */
   unsigned long medium_free;		/* number of medium free calls */
   unsigned long large_free;		/* number of large free calls */
   unsigned long missed_free_count; /* free has to switch heaps count */
   unsigned long realloc_count;		/* number of realloc calls */
   unsigned long missed_realloc_count;  /* realloc has to switch heaps for free */
   unsigned long sched_yield_malloc;  /* missed malloc lock count */
   unsigned long sched_yield_free;  /* missed free lock count */
   };

/* return stats for all heaps */
extern int mts_total_heap_stats (struct mts_stats *);

/* return stats for a specific heap */
extern int mts_heap_stats ( struct mts_stats *stats, int h );

/**************************

  mts_error_callback is designed for applications that want to
  log mts errors. 
  if mts_error_callback is set, no errors fatal or otherwise are handled
  by mts internally. They are all forwarded to the callback function.
  It is strongly advised that the error callback function not invoke
  malloc/free itself.

values for error_:

   0 - warning

   1 - non fatal error
   2 - error writing output
   3 - system call error
   4 - pool related errors
   5 - pool size exceeded limit

  -1 - bad free, non fatal

   mts will call _exit on the following errors

  -2 - fatal error
  -3 - fatal error writing output
  -4 - fatal system call error

****************************/

typedef void (*mts_error_callback) (int error_, char* string, int string_length);
void mts_set_error_callback (mts_error_callback);

/* these are deprecated functions that call the new versions */
extern void mts_set_sys_alloc_size( long min_size, long max_size, int reset );

extern unsigned long mts_heap_size();		/* virtual memory */
extern unsigned long mts_heap_total_size();	/* virtual memory */
extern unsigned long mts_heap_size_used();	/* malloc memory in use */
extern unsigned long mts_heap_total_size_used();/* malloc memory in use */

/* end depricated functions */

/****************************************************************************

MTS Pools Interface

void * (MTSMemHandler *)(void)

Calls that fail to allocate memory will call the function contained in this 
pointer.  If the pointer is NULL, then no call is made. The initial value will 
depend on a configuration variable in mts_config.h.

enum MTSstatus { MTSerror, MTSok };

Some of the calls return a status. The number of different status codes will 
likely change over time to allow greater precision in error reporting.

MTSPool *MTSPoolCreate(void)

This creates a memory pool.  We do not expect to know how large the pool is or 
the way it will be used.  Pools will reconfigure themselves as they grow and 
shrink. If the pool cannot be allocated, a NULL is returned.

MTSstatus MTSPoolDestroy(MTSPool*);

Delete a pool and free its resources. Further use of the Pool pointer in MTS 
calls is illegal.  MTSPoolFreeAll is implicitly called before the pool is 
destroyed. If the Pool pointer is illegal, it may or may not be detected in 
the optimized version of the library. 
The debug version of the library will catch an illegal pool pointer.

MTSstatus MTSPoolFreeAll(MTSPool *)

Free all the memory associated with the pool. If the argument is invalid, MTS 
returns MTSerror. In the optimized version of the library, an illegal Pool 
pointer may or may not be detected. 

size_t MTSPoolTotalSize(MTSPool *)

Return the current amount of memory allocated in the size of the memory pool. 
If the pool pointer is invalid, 0 is returned. 

Note: Pools can grow to the available size of memory allowed by a process. 
Therefore there is no notion of "available size" of a pool.

void *MTSPoolAlloc(MTSPool *, size_t, bool)

Return a block of memory in the pool with the given size.  If the third 
parameter is true, this memory is guaranteed to be set to be all 0's 
initially. If memory cannot be allocated, NULL is returned and the contents of 
MTSMemHandler is called (which must be a pointer to a void returning, void 
argument function). If MTSMemHandler is not set, the routine simply returns 
NULL. A size of 0 will cause the minimum size block to be returned.

void *MTSPoolRealloc(MTSPool *, void *, size_t, bool)

Reallocate a block of memory in a pool.  The block is extended (or reduced) in 
place if possible. Otherwise new memory is allocated from within the pool and 
the memory pointed to by the second parameter is copied to this new area. If 
the last parameter is true, the memory is effectively set to 0's before the 
copy is made (this clearly has no effect when reallocating smaller pieces). As 
above, if the reallocation cannot be done, NULL is returned and the contents 
of MTSMemHandler is called if set.

MTSstatus MTSPoolFree(MTSPool *, void *)

Free the block given. If the pointer is invalid, return error.  
Note, that you can call free (v) on the pointer implying that a thread
could allocate in a pool and another thread might just call free on those
allocations. 
 
size_t MTSPoolSize(MTSPool*, void *)

Return the size of the memory block given as the parameter.  If the pointer is 
determined to be invalid then return 0. 

MTSPool *MTSPoolGet(void *)

The pool associated with a given piece of memory is returned. If the memory 
does not belong to a pool, NULL is returned.

Notes
In order to ensure the same levels of performance in this release of MTS as 
previous releases, the standard allocator is going to be largely unchanged. 
This prevents default pools from being associated with malloc/calloc.  It is 
probably advisable to make all of your memory allocations through pools and 
create default pools on your own.
It is unnecessary to specify pool page size or expected object size with 
pools; they will "self-adjust" to be as optimal as possible given the 
distribution of object sizes requested from the pool.

***************************************************************************/

typedef enum MTSstatus { MTSerror, MTSok } MTSstatus;
typedef struct mts_thread_heap MTSPool;

char (*MTSMemHandler)       ();
MTSPool *MTSPoolCreate      ();
MTSstatus MTSPoolDestroy    (MTSPool*);
MTSstatus MTSPoolFreeAll    (MTSPool*);
size_t	 MTSPoolTotalSize   (MTSPool*);
void *MTSPoolAlloc          (MTSPool*, size_t, int);
void *MTSPoolAllocNL        (MTSPool*, size_t);
void *MTSPoolRealloc        (MTSPool*, void *, size_t, int);
MTSstatus MTSPoolFree       (MTSPool*, void*);
MTSstatus MTSPoolFreeNL     (MTSPool*, void*);
unsigned int MTSPoolSize    (MTSPool*, void*);
MTSPool *MTSPoolGet         (void*);

#ifdef __cplusplus
}
#endif


#endif
