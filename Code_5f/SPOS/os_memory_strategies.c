/*
 * os_memory_strategies.c
 *
 * Created: 5/20/2023 3:35:01 PM
 *  Author: Pooya
 */ 
#include "os_memory_strategies.h"
#include "os_memory.h"

MemAddr last_allocated_addr; // to keep track of allocated Adresses for Next-Fit
//initializer for this Address
void os_init_NextFit(Heap* heap)
{
	last_allocated_addr = os_getUseStart(heap);
}
//First-fit strategy.
MemAddr os_Memory_FirstFit (Heap *heap, size_t size)
{
	uint16_t freeSpace = 0;
	MemAddr addr;
	os_enterCriticalSection();
	for (addr = os_getUseStart(heap); addr < (os_getUseStart(heap)+os_getUseSize(heap)); addr++){
		if(os_getMapEntry(heap, addr) == 0) 
		{
			freeSpace ++;
			if(freeSpace == size) break;
		}
		else freeSpace = 0;
	}
	
	os_leaveCriticalSection();
	if(freeSpace < size) 
		return 0;
	
	return addr - (size - 1);
}
//Next-fit strategy
MemAddr os_Memory_NextFit (Heap *heap, size_t size)
{
	uint16_t freeSpace = 0;
	MemAddr addr;
	os_enterCriticalSection();
	if(last_allocated_addr > os_getUseStart(heap))
	{
		for (addr = last_allocated_addr; addr < os_getUseStart(heap)+os_getUseSize(heap); addr++){
			if(os_getMapEntry(heap, addr) == 0)
			{
				freeSpace ++;
				if(freeSpace == size)
				 {
					addr = addr - (size - 1);
					break;
				 }
			}
			else freeSpace = 0;
		}
		
		if(freeSpace < size)
			addr = os_Memory_FirstFit(heap,size);
	}
	else 
	{
		addr = os_Memory_FirstFit(heap,size);
	}
	last_allocated_addr = addr + size;
	os_leaveCriticalSection();
	return addr;
}
//Best-fit strategy.
MemAddr os_Memory_BestFit (Heap *heap, size_t size)
{
	
	MemAddr addr;
	uint16_t bestSize = os_getUseSize(heap) + 10;
	uint16_t currSize = 0 ;
	MemAddr currAddr = 0;
	MemAddr bestAddr = 0;
	uint16_t freeSpace = 0;
	os_enterCriticalSection();
		for (addr = os_getUseStart(heap); addr < (os_getUseStart(heap)+os_getUseSize(heap)); addr++){
			if(os_getMapEntry(heap, addr) == 0)
			{
				freeSpace ++;
				if(freeSpace >= size)
				{
					 currAddr = addr - (freeSpace-1);
					 currSize = freeSpace;
				}	
			}
			else {
				if(currSize!=0 && currAddr!=0){ 
					if(currSize == size){ os_leaveCriticalSection(); return currAddr;} //Vorzeitiges beenden der Suche
					else if(currSize < bestSize){
						bestSize = currSize;
						bestAddr = currAddr;
					}
				}
				freeSpace=0;
				currAddr = 0; currSize = 0;
			}
		}
		
		
		// in case of big free chunk till the end of Heap
		// the last instructions in for-loop will be not executed!
		if(currSize!=0 && currAddr!=0){
			if(currSize == size){ os_leaveCriticalSection(); return currAddr;} //Vorzeitiges beenden der Suche
			else if(currSize < bestSize){
				bestSize = currSize;
				bestAddr = currAddr;
			}
		}
		
		if (bestAddr == 0) { // not a single fitting Chuck found
		  return 0;
		}
			
		os_leaveCriticalSection();
		return bestAddr;
}

//Worst-fit strategy.
MemAddr os_Memory_WorstFit (Heap *heap, size_t size)
{
	
	MemAddr addr;
	uint16_t biggestSize = 0;
	uint16_t currSize = 0 ;
	MemAddr currAddr = 0;
	MemAddr biggestAddr = 0;
	uint16_t freeSpace = 0;
	uint16_t treshold = os_getUseSize(heap) / 2 ;
	
	os_enterCriticalSection();
	for (addr = os_getUseStart(heap); addr < (os_getUseStart(heap)+os_getUseSize(heap)); addr++){
		if(os_getMapEntry(heap, addr) == 0)
		{
			freeSpace ++;
			if(freeSpace >= size)
			{
				currAddr = addr - (freeSpace-1);
				currSize = freeSpace;
			}
			if(currSize >= treshold){ os_leaveCriticalSection(); return currAddr;} //Vorzeitiges beenden der Suche
		}
		else {
			if(currSize!=0 && currAddr!=0){
				
				 if(currSize > biggestSize){
					biggestSize = currSize;
					biggestAddr = currAddr;
				}
			}
			freeSpace=0;
			currAddr = 0; currSize = 0;
		}
	}
	// in case of big free chunk till the end of Heap 
	// the last instructions in for-loop will be not executed!
	if(currSize!=0 && currAddr!=0){
			
		 if(currSize > biggestSize){
			biggestSize = currSize;
			biggestAddr = currAddr;
		}
	}
	
	if (biggestAddr == 0) { // not a single fitting Chuck found
		return 0;
	}
	
	os_leaveCriticalSection();
	return biggestAddr;
}
