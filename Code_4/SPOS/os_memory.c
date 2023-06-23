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
#include <stdbool.h>

//Writes a value from 0x0 to 0xF to the lower nibble of the given address.(in allocation table)
void setLowNibble (Heap const *heap, MemAddr addr, MemValue value)
{
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
	// get the whole byte value at the given address
	MemValue byteVal = heap->driver->read(addr);
	// remove the higher nibble
	byteVal &= 0x0F;
	return byteVal;
}
//	Reads the value of the higher nibble of the given address(in allocation table).
MemValue getHighNibble (Heap const *heap, MemAddr addr)
{
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
	  os_error("Fehler: outside user area");
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
/*Get the size of a chunk on a given address.
  Takes a use-pointer and computes the length of the chunk. 
  This only works for occupied chunks. The size of free chunks is always 0.
*/
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
	if(os_getIntOrExt(heap)) // Chunk was in External Heap
			os_updateFrames(owner,heap,addr,size);
			
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
	if(os_getIntOrExt(heap)){ // Chunk was in External Heap
		os_FrameSetMalloc(os_getCurrentProc(),addr,size);
		os_setusedExtHeap(os_getCurrentProc(),true);
		}	
	else os_setusedIntHeap(os_getCurrentProc(),true);
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
//	Function that realizes the garbage collection.
// When called, every allocated memory chunk of the given process is freed
void os_freeProcessMemory (Heap *heap, ProcessID pid)
{
	os_enterCriticalSection();
	// Internal Heap
	if(os_getusedIntHeap(pid) && (!os_getIntOrExt(heap))) {
		for (MemAddr addr = os_getUseStart(heap); addr < (os_getUseStart(heap)+os_getUseSize(heap)); addr++)
		{
			if(os_getMapEntry(heap, addr) == pid){
				os_freeOwnerRestricted(heap,addr,pid);
			}
		}
		os_setusedIntHeap(pid,false);
	}
	// External Heap
	if(os_getusedExtHeap(pid) && os_getIntOrExt(heap)) {
		for (MemAddr addr = os_getallocFrameStart(pid); addr <= os_getallocFrameEnd(pid); addr++)
		{
			if(os_getMapEntry(heap, addr) == pid){
			 os_freeOwnerRestricted(heap,addr,pid);
			}
		}
		os_setusedExtHeap(pid,false);
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
/*
This is an efficient reallocation routine. It is used to resize an existing allocated chunk of memory.
If possible, the position of the chunk remains the same. It is only searched for a completely new chunk,
if everything else does not fit For a more detailed description please use the exercise document.

Parameters
heap	The heap on which the reallocation is performed
addr	One address inside of the chunk that is supposed to be reallocated
size	The size the new chunk is supposed to have
Returns
First address (in use space) of the newly allocated chunk
*/
MemAddr os_realloc(Heap *heap, MemAddr addr, uint16_t size)
{
	os_enterCriticalSection();
	ProcessID pid = os_getCurrentProc();
	if(getOwnerOfChunk(heap,addr) != pid){
		os_error("Fehler: free not authorized");
		os_leaveCriticalSection();
		return 0;
	}
	uint16_t newAddr = 0;
	addr = os_getFirstByteOfChunk(heap, addr);
	uint16_t currSize = os_getChunkSize(heap,addr);
	if(currSize == 0)
	{
		os_error("Fehler: Chunk ist frei!");
		os_leaveCriticalSection();
		return 0;
	}
//-----------------------------------------		
//-----------------------------------------
	//no size change happens!
	if(size == currSize) newAddr=addr;
	// make a smaller Chunk 
	else if(size < currSize)
	{
		for(uint16_t i= addr+currSize-1; i >= addr+size ; i--)
			setMapEntry(heap,i,0);
		os_updateFrames(pid,heap,addr,currSize);	
		newAddr = addr;
	}
	// make a bigger Chunk
	else
	{
		uint16_t diff = size - currSize;
		uint16_t af = 0; 
		//check if is any place after the Chunk
			while(os_getMapEntry(heap,addr+currSize+af)==0 && (addr + currSize+af < (os_getUseSize(heap)+os_getUseStart(heap))))
				{
					//if(af==diff)break;
					af++;
				}
		
		if(af >= diff) 
		{	
			for(MemAddr i = addr+currSize; i< addr+size; i++)
			{
				setMapEntry(heap,i,0xF);
			}	
	 		newAddr = addr;
			if(os_getIntOrExt(heap))
			{			
			  os_FrameSetMalloc(pid,newAddr,size);
			}
			os_leaveCriticalSection();
			return newAddr;
		}
		//not enough free space after
		else{
			//check if is any place before the Chunk
			uint16_t b = 0;
				while(os_getMapEntry(heap,addr-b-1)==0 && (addr-b-1 >= os_getUseStart(heap)))		
				{
					b++;
				}
			if(b>=diff)
			{
				newAddr = addr - b;
				moveChunk(heap,addr,newAddr,size);
				if(os_getIntOrExt(heap)) {
					os_updateFrames(pid,heap,newAddr,size);
				}
				os_leaveCriticalSection();
				return newAddr;
			}
			// also not enough space before
			//check if theres enough place both before AND after the chunk
			else if(b+af >= diff)
			{
				newAddr = addr - b;
				moveChunk(heap,addr,newAddr,size);
				if(os_getIntOrExt(heap)) {
					os_updateFrames(pid,heap,newAddr,size);
				}
				os_leaveCriticalSection();
				return newAddr;
			}
			//find a new place for the Chunk
			else
			{
				newAddr = findMemory_to_Allocate(heap,size,os_getAllocationStrategy(heap));
				if(newAddr != 0) {
					moveChunk(heap, addr, newAddr, size);
					if(os_getIntOrExt(heap)) os_updateFrames(pid,heap,newAddr, size);
				}
			}
		}
	}
	os_leaveCriticalSection();
	return newAddr;
}
// this Function rearranges the frames when any changes occur in allocated Memories by a Process
// The addr is the address of the first Byte of the Chunk which is being allocated or freed by another process
void os_updateFrames(ProcessID pid,Heap *heap, MemAddr addr, uint16_t size){
	MemAddr start = os_getallocFrameStart(pid);
	MemAddr end = os_getallocFrameEnd(pid);

	if(addr < start)
		start = addr;
    else if (addr + size - 1 > end)
		end = addr + size -1;
		
	// If a Chunk on the Edges is freed we have to change the Frames if its within the range do nothing 
	else
	{
		//the Chunk is freed in the following section
		
		//chunk at the lower bound
		if(addr == start){
			while(start < end && os_getMapEntry(heap,start) != pid)
				start++;
		}
		//chunk at the upper bound
		if(addr == (end - size + 1) ){
			while(start < end && os_getMapEntry(heap,end) != pid)
				end--;
		}
	}
	
	if(start == end){ //if it was the last used Memory that just freed
		os_setusedExtHeap(pid,false);
	}
		os_setallocFrameStart(pid,start);
		os_setallocFrameEnd(pid,end);
	return;
}
void moveChunk(Heap *heap,MemAddr oldAddr,MemAddr newAddr,uint16_t size)
{
	os_enterCriticalSection();
	uint16_t i = 1;
	//Move the First Byte(copy and free the old)
	setMapEntry(heap,newAddr,os_getMapEntry(heap,oldAddr));
	heap->driver->write(newAddr,heap->driver->read(oldAddr));
	heap->driver->write(oldAddr,0x0);
	setMapEntry(heap,oldAddr,0x0);
	//Move the Rest 
	while(os_getMapEntry(heap,oldAddr+i) == 0xF)
	{
		setMapEntry(heap,newAddr+i,0xF);
		heap->driver->write(newAddr+i,heap->driver->read(oldAddr+i));
		heap->driver->write(oldAddr+i,0x0);
		setMapEntry(heap,oldAddr+i,0x0);
		i++;
	}
	//update size 
	while(i < size)
	{
		setMapEntry(heap,newAddr+i,0xF);
		i++;
	}
	os_leaveCriticalSection();
}
void os_FrameSetMalloc(ProcessID pid, MemAddr addr,uint16_t size)
{
	if(os_getallocFrameStart(pid) == 0 || os_getallocFrameStart(pid) > addr) 
		os_setallocFrameStart(pid,addr);
	if(os_getallocFrameEnd(pid) < addr + size - 1)
	    os_setallocFrameEnd(pid,addr + size - 1);
}