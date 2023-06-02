/*
 * os_memheap_drivers.c
 *
 * Created: 5/20/2023 3:18:48 PM
 *  Author: Pooya
 */ 
#include "os_memheap_drivers.h"
#include "defines.h"
#include "os_memory_strategies.h"
#include <avr/pgmspace.h>

#define TOTAL_HEAPSIZE		((AVR_MEMORY_SRAM / 2) - HEAPOFFSET)
#define MAP_SIZE			(TOTAL_HEAPSIZE/3)
#define USE_SIZE			(MAP_SIZE*2)

//Name for the Heap (idk what the name should be)
 PROGMEM const char heapName[] = "SRAM_heap";
 
//initialising the intHeap structure 
Heap intHeap__ = {.driver = intSRAM , 
	.mapSize = MAP_SIZE ,
    .mapStart = AVR_SRAM_START + HEAPOFFSET,
	.useSize =  USE_SIZE,
	.useStart = AVR_SRAM_START + HEAPOFFSET + MAP_SIZE,
	.name = heapName,
	.strategy = OS_MEM_FIRST};
	
void os_initHeaps()
{
	//alle Nibbles mit 0 initialisiren
	for(MemAddr i = intHeap->mapStart ; i < (intHeap->mapStart + intHeap->mapSize); i++)
		{
			intHeap->driver->write(i,0);
		}
}

Heap *os_lookupHeap(uint8_t index)
{
	// returns the pointer to the Heap with the given index (intHeap has index 0)
	if(index==0) 
		return intHeap;	
	// COMPLETE FOR THE NEXT VESUCH	
	return 0;	
}
size_t os_getHeapListLength()
{
	//it should return how many Heaps exist
	return 1;
}

