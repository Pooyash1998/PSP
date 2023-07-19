/*
 * os_spi.c
 *
 * Created: 5/28/2023 4:21:09 PM
 *  Author: Pooya
 */ 
#include "os_spi.h"

void os_spi_init()
{
	//initialize Ports MOSI(PB5), SCK(PB7), /SS(PB4) as output and others remain input
	DDRB |= (1 << 5) | (1 << 7) | (1 << 4);
	// deselect chip 
	PORTB |= (1<<4);
	// enable SPI as Master and set the config Register bits
	// SPE(6) = 1 ; MSTR(4) = 1;
	SPCR = (1<<6) | (1 << 4);
	/* Setting the Configurations for 23LC1024 in Control Register
	   SPR0(0) = 0 ; SPR1(1) = 0 ; SPI2X(0 in status reg) = 1  
	   ---------resulting in SPI-Freq = f_cpu/2 = 10MHz------------- 
	   DORD(5) = 0 (MSB first)
	   CPOL(3) = 0 (Active High)
	   CPHA(2) = 0 (Leading Edge)
	*/
	SPSR |= (1 << 0);
	
}

/*Performs a SPI send This method blocks until the data exchange is completed.
Additionally, this method returns the byte received from the slave */
uint8_t os_spi_send(uint8_t data)
{
	uint8_t buff;
	// Start transmission (instruction)
	SPDR = data;
	// Wait for transmission complete
	while(!(SPSR & (1<<SPIF)));
	//received data from Slave
	buff = SPDR;
	return buff;
	
	
}
/*Performs a SPI read This method blocks until the exchange is completed by
Sending a Dummy-Byte 0xFF to start the Process */
uint8_t os_spi_receive()
{
	SPDR = 0xFF;
	while(!(SPSR & (1<<SPIF)));
	return(SPDR);
}