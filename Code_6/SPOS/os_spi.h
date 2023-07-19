/*
 * os_spi.h
 *
 * Created: 5/28/2023 4:18:19 PM
 *  Author: Pooya
 */ 


#ifndef OS_SPI_H_
#define OS_SPI_H_

#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include "util.h"

//Configures relevant I/O registers/pins and initializes the SPI module.
void os_spi_init (void);

/*Performs a SPI send This method blocks until the data exchange is completed. 
Additionally, this method returns the byte received from the slave */
uint8_t os_spi_send (uint8_t data);

//Performs a SPI read This method blocks until the exchange is completed.
uint8_t os_spi_receive ();


#endif /* OS_SPI_H_ */