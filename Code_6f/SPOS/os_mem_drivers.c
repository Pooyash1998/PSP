/*
 * os_mem_drivers.c
 *
 * Created: 5/20/2023 3:10:00 PM
 *  Author: Pooya
 */ 

#include "os_mem_drivers.h"
#include "defines.h"
#include "os_scheduler.h"
#include "os_spi.h"
#include "util.h"
#include <avr/io.h>
#include <avr/interrupt.h>


#define READ_Instr 0x03
#define WRITE_Instr 0x02
#define WRMR_Instr 0x01
#define RDMR_Instr 0x05
#define BYTE_MODE 0x00
#define PAGE_MODE 0x80
#define SEQ_MODE 0x40

/*
Pseudo-function to initialize the internal SRAM Actually, there is nothing to be done when initializing the internal SRAM,
but we create this function, because MemDriver expects one for every memory device.
*/
void initSRAM_internal()
{
	//nothing to do when using internalRam.
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
Private function to write a value to the internal SRAM 
It will not check if its call is valid. This has to be done on a higher level.
*/
void writeSRAM_internal(MemAddr addr, MemValue value)
{
	*((uint8_t*)addr) = value;
	
}
/* Initialization of the external SRAM board.
 This function performs actions such as setting the respective Data Direction Register etc..
*/
void initSRAM_external(void)
{
	os_spi_init();
	set_operation_mode(BYTE_MODE);
}
/*Private function to read a single byte to the external SRAM 
 It will not check if its call is valid. This has to be done on a higher level.
*/
MemValue readSRAM_external (MemAddr addr)
{
	os_enterCriticalSection();
	select_memory();
	os_spi_send(READ_Instr);
	transfer_address(addr);
	MemValue retData = os_spi_receive();
	deselect_memory();
	os_leaveCriticalSection();
	return retData;
}
/*Private function to write a single byte to the external SRAM 
  It will not check if its call is valid. This has to be done on a higher level.
*/
void writeSRAM_external(MemAddr addr, MemValue value)
{
	os_enterCriticalSection();
	select_memory();
	os_spi_send(WRITE_Instr);
	transfer_address(addr);
	os_spi_send(value);
	deselect_memory();
	os_leaveCriticalSection();
}
//Activates the external SRAM as SPI slave.
void select_memory()
{
	//set /SS(PB4) low -> /CS low
	PORTB &= ~(1 << 4);
}
//Deactivates the external SRAM as SPI slave.
void deselect_memory()
{
	//set /SS(PB4) high -> /CS high
	PORTB |= (1 << 4);
}
//Sets the operation mode of the external SRAM.
void set_operation_mode(uint8_t mode)
{
	os_enterCriticalSection();
	select_memory();
	// Set the Operation mode by 23LC1024 to Byte
	// instruction Write Mode Register WRDR (0000 0001)
	// Data to Mode Register e.g. 00000000 for Byte Mode
	os_spi_send(WRMR_Instr); //instruction
	os_spi_send(mode); //Mode
	deselect_memory();
	os_leaveCriticalSection();
}
//Transmits a 24bit memory address to the external SRAM.
void transfer_address(MemAddr addr)
{
	/*the first 7 MSB are ignored by extSRAM and we dont use the 8th either 
	so at the end we have 24-8= 16 addresses
	1- send 00000000
	2- send the HighByte of addr
	3- send the LowByte of addr
	*/
	os_spi_send(0x0);
	os_spi_send(addr >> 8);
	os_spi_send(addr & 0xFF);
}
/*
Function that needs to be called once in order to initialize all used memories such as the internal SRAM etc...
*/
void initMemoryDevices()
{
	//initSRAM_external(); DUE TO #TLCD
	initSRAM_internal();
}

//initializing the intSRAM ( extern defined in .h)
MemDriver intSRAM__ = {.memInit = initSRAM_internal , .read = readSRAM_internal , .write = writeSRAM_internal ,
						.memstart = AVR_SRAM_START , .memsize = AVR_MEMORY_SRAM};
						
//initializing the extSRAM ( extern defined in .h)	
/* use only half the Size 128/2Kib = 2^16 = 65536 Bytes = 0x10000 
because we only have 16 bits the biggest value we can use/save is SIZE_MAX 2^15-1 = 65535 
which is 0xFFFF. so our Addresses would be from 0 - 0xFFFE. we can use the last Address but then it doesnt match the 
size we are setting as constant*/
MemDriver extSRAM__ = {.memInit = initSRAM_external , .read = readSRAM_external , .write = writeSRAM_external ,
						.memstart = 0 , .memsize = 0xFFFF};
					