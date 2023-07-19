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

#define INT_TOTAL_HEAPSIZE		((AVR_MEMORY_SRAM / 2) - HEAPOFFSET)
#define INT_MAP_SIZE			(INT_TOTAL_HEAPSIZE/3)
#define INT_USE_SIZE			(INT_MAP_SIZE*2)

#define EXT_TOTAL_HEAPSIZE       0xFFFF
#define EXT_MAP_SIZE			(EXT_TOTAL_HEAPSIZE/3)
#define EXT_USE_SIZE			(EXT_MAP_SIZE*2)
//Name for the Heaps
 PROGMEM const char intHeapName[] = "Int_SRAM_heap";
 PROGMEM const char extHeapName[] = "Ext_SRAM_heap";
 
//initializing the intHeap structure 
Heap intHeap__ = {.driver = intSRAM , 
	.mapSize = INT_MAP_SIZE ,
    .mapStart = AVR_SRAM_START + HEAPOFFSET,
	.useSize =  INT_USE_SIZE,
	.useStart = AVR_SRAM_START + HEAPOFFSET + INT_MAP_SIZE,
	.name = intHeapName,
	.strategy = OS_MEM_FIRST};
//initializing the extHeap structure	
Heap extHeap__ = {.driver = extSRAM ,
	.mapSize = EXT_MAP_SIZE ,
	.mapStart = 0 ,
	.useSize = EXT_USE_SIZE ,
	.useStart = EXT_MAP_SIZE ,
	.name = extHeapName,
.strategy = OS_MEM_FIRST};

void os_initHeaps()
{	
	//alle Bytes(Nibbles) mit 0 initialisiren
	//internal HEAP
	for(MemAddr i = intHeap->mapStart ; i < (intHeap->mapStart + intHeap->mapSize); i++)
		intHeap->driver->write(i,0);
	
	
	//DUE TO #TLCD
	/*
	//External HEAP
	for(MemAddr i = extHeap->mapStart ; i < (extHeap->mapStart + extHeap->mapSize); i++)
		extHeap->driver->write(i,0);
		*/
}

Heap *os_lookupHeap(uint8_t index)
{
	// returns the pointer to the Heap with the given index (intHeap has index 0)
	if(index==0) 
		return intHeap;	
	if(index==1)
		return extHeap;	
	//default
	return 0;	
}
size_t os_getHeapListLength()
{
	//it should return how many Heaps exist
	//return 2;
	return 1; //#TLCD
}
uint8_t os_getIntOrExt(Heap *heap)
{
	if(heap->name == intHeapName)
		return 0;
	if(heap->name == extHeapName)
		return 1;
	//in case of any Error
	return 100;		
}

