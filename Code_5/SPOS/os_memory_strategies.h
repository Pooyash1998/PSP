/*
 * os_memory_strategies.h
 *
 * Created: 5/20/2023 3:33:27 PM
 *  Author: Pooya
 */ 


#ifndef OS_MEMORY_STRATEGIES_H_
#define OS_MEMORY_STRATEGIES_H_

#include "os_memheap_drivers.h"


//First-fit strategy.
MemAddr os_Memory_FirstFit (Heap *heap, size_t size);
//Next-fit strategy
MemAddr os_Memory_NextFit (Heap *heap, size_t size);
//Best-fit strategy.
MemAddr os_Memory_BestFit (Heap *heap, size_t size);
//Worst-fit strategy.
MemAddr os_Memory_WorstFit (Heap *heap, size_t size);
//initializer for the latest Allocated Address
void os_init_NextFit(Heap* heap);

#endif /* OS_MEMORY_STRATEGIES_H_ */