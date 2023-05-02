#include "led.h"
#include <avr/io.h>

uint16_t activateLedMask = 0xFFFE; //16LEDs for UP4 setze auf 0xFFFE

/*!
 *  Initializes the led bar. Note: All PORTs will be set to output.
 */
void initLedBar(void) {
	DDRA |= activateLedMask & 0xFF;
	DDRD |= (activateLedMask >> 8) & 0xFF;
	
	//PORTA |= activateLedMask & 0xFF; //alle Pullup einsetzen ausser A0
	PORTA |= 0xFF; // wir wollen sowieso Pullpup-Widerstand bei A0 
	
    PORTD = (activateLedMask >> 8) & 0xFF;
}

/*!
 *  Sets the passed value as states of the led bar (1 = on, 0 = off).
 */
void setLedBar(uint16_t value) {	
   
   uint8_t selected_bit; // enthaelt den Wert von jedem Bit aus "value"
   for(uint8_t i=0 ; i<=7 ; i++)  //die ersten 8 LEDS mit portA verbunden
   {
	   selected_bit = (value & (1 << i)) >> i;
	   // Falls PinA0 fuers Unterprogramm drei nicht verfuegbar ist mache nix 
	   if((activateLedMask & (1 << i)) == 0) 
	   {	
		   continue;
	   }
	   
	   if (selected_bit == 0) {
		   PORTA |=  (1 << i); // Wenn Bit 0 ist, setze das entsprechende Bit in PORTA auf 1 d.h. LED aus
		   } 
	   else {
	       PORTA &= ~(1 << i); // Wenn Bit 1 ist, setze das entsprechende Bit in PORTA auf 1 d.h. LED an
	   }
   }
   //analog fuer portD 
   for(uint8_t j=8 ; j<=15 ; j++)
   {
	   selected_bit = (value & (1 << j)) >> j; 
	   if (selected_bit == 0) {
		   PORTD |= (1 << (j - 8)); //value ist 16 bit aber unser PORT hat nur 8 bits
	   }
	   else {
		   PORTD &= ~ (1 << (j - 8)); 
	   }
	   
   }
   
}
