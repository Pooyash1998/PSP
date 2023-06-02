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

//Function used to allocate private memory.
MemAddr os_malloc(Heap *heap, size_t size);
//	Function used by processes to free their own allocated memory
void os_free(Heap *heap, MemAddr addr);
//Frees the chunk iff it is owned by the given owner.
void os_freeOwnerRestricted (Heap *heap, MemAddr addr, ProcessID owner);
//	Function used to get the value of a single map entry, this is made public so the allocation strategies can use it.
MemValue os_getMapEntry (Heap const *heap, MemAddr addr);
//Function that realises the garbage collection.
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



#endif /* OS_MEMORY_H_ */