/*
 * os_memheap_drivers.h
 *
 * Created: 5/20/2023 3:14:46 PM
 *  Author: Pooya
 */ 


#ifndef OS_MEMHEAP_DRIVERS_H_
#define OS_MEMHEAP_DRIVERS_H_

#include "os_mem_drivers.h"
#include <stddef.h>

#define   intHeap  (&intHeap__)


//All available heap allocation strategies.
typedef enum
{
	OS_MEM_FIRST,
	OS_MEM_NEXT,
	OS_MEM_BEST,
	OS_MEM_WORST,
	
}AllocStrategy;

//The structure of a heap driver which consists of a low level memory driver and heap specific information such as start, size etc...
typedef struct 
{
	MemDriver *driver;
	const MemAddr useStart;
	const MemAddr mapStart;
	const size_t mapSize;
	const size_t useSize;
	AllocStrategy strategy;
    const char *name;

}Heap;


//The Heap structure for the internal Heap.
extern Heap intHeap__;

//Initialises all Heaps.
void os_initHeaps(void);
//	Needed for Taskmanager interaction.
Heap *os_lookupHeap (uint8_t index);
//Needed for Taskmanager interaction.
size_t os_getHeapListLength(void);



#endif /* OS_MEMHEAP_DRIVERS_H_ */