/*
 * os_memory.h
 *
 * Created: 5/20/2023 3:20:06 PM
 *  Author: Pooya
 */ 


#ifndef OS_MEMORY_H_
#define OS_MEMORY_H_

#include "os_mem_drivers.h"
#include "os_memheap_drivers.h"
#include "os_scheduler.h"

#define SH_COMMANDS		0xB
#define SH_READ1	    0xB
#define SH_READ2		0xC
#define SH_WRITE		0xD
#define SH_CLOSED		0xE

//Function used to allocate private memory.
MemAddr os_malloc(Heap *heap, size_t size);
//	Function used by processes to free their own allocated memory
void os_free(Heap *heap, MemAddr addr);
//Frees the chunk iff it is owned by the given owner.
void os_freeOwnerRestricted (Heap *heap, MemAddr addr, ProcessID owner);
//	Function used to get the value of a single map entry, this is made public so the allocation strategies can use it.
MemValue os_getMapEntry (Heap const *heap, MemAddr addr);
//Function that realizes the garbage collection.
void os_freeProcessMemory (Heap *heap, ProcessID pid);
//	Get the size of the heap-map.
size_t os_getMapSize (Heap const *heap);
//Get the size of the usable heap. 
size_t os_getUseSize (Heap const *heap);
//	Get the start of the heap-map.
MemAddr os_getMapStart (Heap const *heap);
// Get the start of the usable heap.
MemAddr os_getUseStart (Heap const *heap);
//	Get the size of a chunk on a given address.
uint16_t os_getChunkSize (Heap const *heap, MemAddr addr);
//	Get the address of the first byte of chunk.
MemAddr os_getFirstByteOfChunk (Heap const *heap, MemAddr addr);
// Changes the memory management strategy.
void os_setAllocationStrategy (Heap *heap, AllocStrategy allocStrat);
//Returns the current memory management strategy.
AllocStrategy os_getAllocationStrategy (Heap const *heap);
//this searches for a new memory chunk with the given size
MemAddr findMemory_to_Allocate(Heap *heap, size_t size, AllocStrategy allocStrat);
//Function to efficiently reallocate memory.
MemAddr os_realloc (Heap *heap, MemAddr addr, uint16_t size);
// this Function rearranges the frames when any changes occur in allocated Memories by a Process
void os_updateFrames(ProcessID pid,Heap *heap, MemAddr addr, uint16_t size);
//to copy and move the data to a bigger new chunk 
void moveChunk(Heap *heap,MemAddr oldAddr,MemAddr newAddr,uint16_t size);
//just for malloc
void os_FrameSetMalloc(ProcessID pid, MemAddr addr,uint16_t size);
//Function used to allocate shared memory
MemAddr os_sh_malloc(Heap *heap, size_t size);
//Function used by processes to free shared memory.
void os_sh_free(Heap *heap, MemAddr *addr);
//Function used to write to shared memory.
void os_sh_write(Heap const *heap, MemAddr const *ptr, uint16_t offset, MemValue const *dataSrc, uint16_t length);
//Function used to read from shared memory.
void os_sh_read (Heap const *heap, MemAddr const *ptr, uint16_t offset, MemValue *dataDest, uint16_t length);
//Function that should be private but is used by some Testtasks.
MemAddr os_sh_readOpen(Heap const *heap, MemAddr const *ptr);
//Function that should be private but is used by some Testtasks.
MemAddr os_sh_writeOpen (Heap const *heap, MemAddr const *ptr);
//Function that should be private but is used by some Testtasks. 
void os_sh_close (Heap const *heap, MemAddr addr);

#endif /* OS_MEMORY_H_ */