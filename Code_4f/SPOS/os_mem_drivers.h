/*
 * os_mem_drivers.h
 *
 * Created: 5/20/2023 2:56:19 PM
 *  Author: Pooya
 */ 

// Die Kommentare stammen alle aus Doxygen
#ifndef OS_MEM_DRIVERS_H_
#define OS_MEM_DRIVERS_H_

#include <inttypes.h>
#include "defines.h"
#include <stddef.h>

//Realizes a pointer to the MemDriver intSRAM__. 
#define  intSRAM  (&intSRAM__)
//Realizes a pointer to the MemDriver extSRAM__.
#define  extSRAM  (&extSRAM__)

//----------------------------------------------------------------------------
// Typedefs (from Doxygen)
//----------------------------------------------------------------------------
//Type used instead of uint8_t* pointers to avoid direct dereferencing.
typedef uint16_t MemAddr;
//Type for a single value (used instead of uint8_t to increase readability)
typedef uint8_t MemValue;
//Type of a memory driver initialisation function.
typedef void MemoryInitHnd(void);
//Type of a memory driver read function.
typedef MemValue MemoryReadHnd(MemAddr addr);
//Type of a memory driver write function.
typedef void MemoryWriteHnd(MemAddr addr,MemValue value);
//The data structure for a memory driver such as intSRAM.
typedef struct
{
	//funktionspointer
	MemoryInitHnd *memInit;
	MemoryReadHnd *read;
	MemoryWriteHnd *write;
	//charakteristische Konstanten
	const MemAddr memstart;
	const size_t memsize;
}MemDriver;


//----------------------------------------------------------------------------
// Variables
//----------------------------------------------------------------------------
//This specific MemDriver is initialized in os_mem_drivers.c.
//we make it extern and initialize it in .c file to make it globally accessible in the Project
extern MemDriver intSRAM__;
//This specific MemDriver is initialised in os_mem_drivers.c. 
extern MemDriver extSRAM__;

//----------------------------------------------------------------------------
// Function headers
//----------------------------------------------------------------------------
// Initialize all memory devices
void initMemoryDevices (void);
//Activates the external SRAM as SPI slave.
void select_memory();
//Deactivates the external SRAM as SPI slave.
void deselect_memory();
//Sets the operation mode of the external SRAM.
void set_operation_mode(uint8_t mode);
//Transmits a 24bit memory address to the external SRAM.
void transfer_address(MemAddr addr);


#endif /* OS_MEM_DRIVERS_H_ */