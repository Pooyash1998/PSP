/*
 * os_memory.c
 *
 * Created: 5/20/2023 3:25:08 PM
 *  Author: Pooya
 */ 
#include "os_memory.h"
#include "os_memory_strategies.h"
#include "util.h"
#include "os_core.h"

// Function Headers 
MemAddr findMemory_to_Allocate(Heap *heap, size_t size, AllocStrategy allocStrat);

//Writes a value from 0x0 to 0xF to the lower nibble of the given address.(in allocation table)
void setLowNibble (Heap const *heap, MemAddr addr, MemValue value)
{
	//if (addr < heap->mapStart || addr > (heap->mapSize + heap->mapStart))
		//os_error("Fehler: Heap under/overflow");
	// get the whole byte value at the given address
	MemValue byteVal = heap->driver->read(addr);
	//clear the lower nibble
	byteVal &= 0xF0;
	// set the lower nibble
	byteVal |= value;
	
	heap->driver->write(addr, byteVal);
}
//Writes a value from 0x0 to 0xF to the higher nibble of the given address. (in allocation table)
void setHighNibble (Heap const *heap, MemAddr addr, MemValue value)
{
	//if (addr < heap->mapStart || addr > (heap->mapSize + heap->mapStart))
		//os_error("Fehler: Heap under/overflow");
	// calculate the higher nibble value
	MemValue nibVal = (value << 4);
	// get the whole byte value at the given address
	MemValue byteVal = heap->driver->read(addr);
	//clear the higher nibble 
	byteVal &= 0x0F;
	// set the higher nibble
	byteVal |= nibVal;
	
	heap->driver->write(addr, byteVal);
}
//Reads the value of the lower nibble of the given address.(in allocation table)
MemValue getLowNibble (Heap const *heap, MemAddr addr)
{	
	//if (addr < heap->mapStart || addr > (heap->mapSize + heap->mapStart))
		//os_error("Fehler: Heap under/overflow");
	// get the whole byte value at the given address
	MemValue byteVal = heap->driver->read(addr);
	// remove the higher nibble
	byteVal &= 0x0F;
	return byteVal;
}
//	Reads the value of the higher nibble of the given address(in allocation table).
MemValue getHighNibble (Heap const *heap, MemAddr addr)
{
	//if (addr < heap->mapStart || addr > (heap->mapSize + heap->mapStart))
		//os_error("Fehler: Heap under/overflow");
	// get the whole byte value at the given address
	MemValue byteVal = heap->driver->read(addr);
	// remove the lower nibble
	byteVal = (byteVal >> 4);
	return byteVal;
}
//This function is used to set a heap map entry on a specific heap.
/*
Parameters:
heap	The heap on whos map the entry is supposed to be set
addr	The address in USE space for which the corresponding map entry shall be set
value	The value that is supposed to be set onto the map (valid range: 0x0 - 0xF)
*/
void setMapEntry (Heap const *heap, MemAddr addr, MemValue value)
{
	if ((addr >= heap->useStart) && (addr < (heap->useStart + heap->useSize))){
	MemAddr offset = addr - heap->useStart;
	if (offset % 2 == 0){
		MemAddr mapAdd = heap->mapStart + (offset / 2);
		return setHighNibble(heap,mapAdd,value);
	}
	else {
		MemAddr mapAdd = heap->mapStart + ((offset - 1) / 2);
		return setLowNibble(heap,mapAdd,value);
	}
  }
  else
 os_error("Fehler: outside user area");
}
//Function used to get the value of a single map entry, this is made public so the allocation strategies can use it.
/*
Parameters:
heap	The heap from whos map the entry is supposed to be fetched
addr	The address in USE space for which the corresponding map entry shall be fetched

Returns:
The value that can be found on the heap map entry that corresponds to the given USE space address
*/
MemValue os_getMapEntry (Heap const *heap, MemAddr addr)
{
	if ((addr >= heap->useStart) && (addr < (heap->useStart + heap->useSize))){
		
	MemAddr offset = addr - heap->useStart;
	if (offset % 2 == 0){
		MemAddr mapAdd = heap->mapStart + (offset / 2);
		return getHighNibble(heap,mapAdd);
	}
	else {
		MemAddr mapAdd = heap->mapStart + ((offset - 1) / 2);
		return getLowNibble(heap,mapAdd);
	}
  }
  else{
	  os_error("Fehler: outside user area"); ///=================================PROBLEM!!!!!!!!!!!
	  return 0;
  }
}
//Get the address of the first byte of chunk. 
/*This function is used to determine where a chunk starts if a given address might not point to 
the start of the chunk but to some place inside of it.
Parameters:
heap	The heap the chunk is on hand in
addr	The address that points to some byte of the chunk
*/ 
MemAddr os_getFirstByteOfChunk (Heap const *heap, MemAddr addr)
{
	MemValue chk = os_getMapEntry(heap,addr);	
	if(chk != 0)
	{
		while (addr > os_getUseStart(heap) && os_getMapEntry(heap, addr) == 0xF) 
			addr--;
	}
	else
	 while (addr > os_getUseStart(heap) && os_getMapEntry(heap, addr - 1) == 0x0) 
		addr--;
	return addr;	
}
ProcessID getOwnerOfChunk(Heap const *heap, MemAddr addr)
{
	return os_getMapEntry(heap,os_getFirstByteOfChunk(heap,addr));
}
//	Get the size of a chunk on a given address.
//Takes a use-pointer and computes the length of the chunk. 
//This only works for occupied chunks. The size of free chunks is always 0.
uint16_t os_getChunkSize (Heap const *heap, MemAddr addr)
{
	MemAddr headAddr = os_getFirstByteOfChunk(heap,addr);
	uint16_t size = 1;
	if(os_getMapEntry(heap,headAddr) == 0)
		return 0;
		
	while((headAddr+size < os_getUseSize(heap)+os_getUseStart(heap)) && os_getMapEntry(heap, headAddr + size) == 0x0F)
			size++;
	
	return size;
	
}
//	Frees the chunk iff it is owned by the given owner.
/*
Frees a chunk of allocated memory on the medium given by the driver. 
This function checks if the call has been made by someone with the right to do it 
(i.e. the process that owns the memory or the OS).
This function is made in order to avoid code duplication and is called by several functions that, in some way,
free allocated memory such as os_freeProcessMemory/os_free...
addr: An address inside of the chunk (not necessarily the start). 
*/
void os_freeOwnerRestricted (Heap *heap, MemAddr addr, ProcessID owner)
{
	os_enterCriticalSection();
	if(getOwnerOfChunk(heap,addr) != owner){
		os_error("Fehler: free not authorized");
		os_leaveCriticalSection();
	}
	size_t size = os_getChunkSize(heap,addr);
	MemAddr chnkadd = os_getFirstByteOfChunk(heap,addr);
	for(size_t i = 0 ; i < size; i++)
		{
			setMapEntry(heap, (chnkadd + i), 0x0);
		}
	os_leaveCriticalSection();	
}
// Function used to allocate private memory.
MemAddr os_malloc (Heap *heap, size_t size)
{
	os_enterCriticalSection();
	MemAddr addr = findMemory_to_Allocate(heap, size, os_getAllocationStrategy(heap));
	if(addr == 0)
	{
		//Already showed an Error in findMemory_to_Allocate()
		os_leaveCriticalSection();
		return 0;
	}
	// assign the Corresponding Nibbles to the procesID 
	setMapEntry(heap, addr, os_getCurrentProc());
	// set other Nibbles depending on size
	for(size_t i = 1 ; i< size ; i++)
	{
		setMapEntry(heap, (addr + i), 0x0F);
	}
	os_leaveCriticalSection();
	return addr;
}
// Function used by processes to free their own allocated memory.
//addr:	An address inside of the chunk (not necessarily the start). 
void os_free (Heap *heap, MemAddr addr)
{
	os_freeOwnerRestricted(heap,addr,os_getCurrentProc());
}
//Get the size of the heap-map. 
size_t os_getMapSize (Heap const *heap)
{
	return heap->mapSize;
}
//Get the size of the usable heap.
size_t os_getUseSize (Heap const *heap)
{
	return heap->useSize;
}
//	Get the start of the heap-map.
MemAddr os_getMapStart (Heap const *heap)
{
	return heap->mapStart;
}
//	Get the start of the usable heap.
MemAddr os_getUseStart (Heap const *heap)
{
	return heap->useStart;
}
//Changes the memory management strategy. More...
void os_setAllocationStrategy (Heap *heap, AllocStrategy allocStrat)
{
	if (allocStrat == OS_MEM_NEXT && heap->strategy!= OS_MEM_NEXT)
	{
		os_init_NextFit(heap);
	}
	heap->strategy = allocStrat;
}
//	Returns the current memory management strategy.
AllocStrategy os_getAllocationStrategy (Heap const *heap)
{
	return heap->strategy;
}
//	Function that realises the garbage collection.
// When called, every allocated memory chunk of the given process is freed
void os_freeProcessMemory (Heap *heap, ProcessID pid)
{
	os_enterCriticalSection();
	for (MemAddr addr = os_getUseStart(heap); addr < (os_getUseStart(heap)+os_getUseSize(heap)); addr++)
	{
		if(os_getMapEntry(heap, addr) == pid){
			os_freeOwnerRestricted(heap,addr,pid);
		}
	}
	os_leaveCriticalSection();
}
// finds a proper Heap-Chunk depending on the AllocationStrategy and returns the address of the first Byte
MemAddr findMemory_to_Allocate(Heap *heap, size_t size, AllocStrategy allocStrat){
	switch(allocStrat)
	{
		case OS_MEM_FIRST :
			return os_Memory_FirstFit(heap,size);
			break;
		case OS_MEM_NEXT :
			return os_Memory_NextFit(heap,size);
			break;
		case OS_MEM_BEST :
			return os_Memory_BestFit(heap,size);
			break;
		case OS_MEM_WORST :
			return os_Memory_WorstFit(heap,size);
			break;			
	}
	os_error("Fehler:Allocation failed");
	return 0;
}
