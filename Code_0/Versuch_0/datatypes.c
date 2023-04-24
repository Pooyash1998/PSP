/*
 * datatypes.c
 *
 * Created: 4/22/2023 7:05:44 PM
 *  Author: Pooya
 */ 
#include "datatypes.h"
#include <stdint.h>
#include "lcd.h"
void loop()
{
	 uint8_t result = 0;
	for ( uint8_t i=5; i>= 0; i--){
		 result += i * 2 + 2;
	}
	lcd_writeProgString(PSTR("Loop finished:"));
	lcd_writeDec(result);
	
	/* for Debugging use volatile because i is moved to register for easy access we cant 
	watch it. the other problem is here when i==0 and forloop decrements i jumps at 255
	because we have no negative numbers in unsigned int. remove "u" to solve the problem :) */ 
}
void convert()
{
	volatile uint8_t target = 200;
	volatile uint16_t source = target + 98;
	target = source;
	/* bei der Zuweisung springt target auf 42 da es nur 8 bit hat und 
	hoechstens die Zahl 2^8 =256 darstellen kann. 298 - 256 = 42 Ueberlauf */
}
void shift()
{
	/* code 1
	
	uint32_t result = 1 << 31;
	lcd_write32bitHex(result);
	*/
	
	// code 2
	uint32_t result = (uint32_t) 1 << 31;
	lcd_write32bitHex(result);
	
}