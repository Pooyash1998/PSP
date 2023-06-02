/*
 * os_mem_drivers.c
 *
 * Created: 5/20/2023 3:10:00 PM
 *  Author: Pooya
 */ 

#include "os_mem_drivers.h"
#include "defines.h"

/*
Pseudo-function to initialize the internal SRAM Actually, there is nothing to be done when initializing the internal SRAM,
but we create this function, because MemDriver expects one for every memory device.
*/
void initSRAM_internal()
{
	//nothing to do wenn using internalram.
	//the function is just a placeholder.
}
/*
Private function to read a value from the internal SRAM It will not check if its call is valid. This has to be done on a higher level.
*/
MemValue readSRAM_internal(MemAddr addr)
{
	return *((uint8_t*)addr);
}
/*
Private function to write a value to the internal SRAM It will not check if its call is valid. This has to be done on a higher level.
*/
void writeSRAM_internal(MemAddr addr, MemValue value)
{
	*((uint8_t*)addr) = value;
	
}
/*
Function that needs to be called once in order to initialise all used memories such as the internal SRAM etc...
*/
void initMemoryDevices()
{
	//idk probably for the next Versuch
}

//initializing the intSRAM ( extern defined in .h)
MemDriver intSRAM__ = {.memInit = initSRAM_internal , .read = readSRAM_internal , .write = writeSRAM_internal ,
						.memstart = AVR_SRAM_START , .memsize = AVR_MEMORY_SRAM};