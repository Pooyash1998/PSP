/*
 * led.c
 *
 * Created: 4/22/2023 5:35:47 PM
 *  Author: Pooya
 */ 
#include "button.h"
#include <util/delay.h>
#include <avr/io.h>

void led_init()
{
	DDRD |= 0xFF ; //alle LEDS auf 1 setzten = Ausgang
	PORTD |= 0xFF ; //alle LEDs am Amfang sollen aus sein. 1=aus 0=an!
}

void led_fun()
{
	uint8_t buttonPressed = getInput();
	// Taster 1
	if ( buttonPressed == 1 << 0 ) 
	{  
		PORTD ^= (1 << 0) ;  //invert the bit for the first LED
	}
	// Taster 2
	if ( buttonPressed == 1 << 1 ) 
	{
		// Verschiebung der LEDs nach links um 1
		// auch wenn die linkeste an ist, sollte danach die rechteste angeleuchtet werden.
		uint8_t lastLedState = PORTD & 0b10000000 ;
		// vei der Verschiebung wird 0 erzeugt d.h. angeschaltete LEDs
		uint8_t invertedPortD = ~PORTD;
		invertedPortD = invertedPortD << 1;
		PORTD = ~invertedPortD;
		
		// nun die letzte und erste LED situation
		if(lastLedState == 0)   //if the last one was on
			PORTD &= 0b11111110 ; //then make the first one on
		
	}
	// Taster 3
	if ( buttonPressed == 1 << 2 ) 
	{
		PORTD = ~ PORTD;
	}
	else if (buttonPressed != 0) //mehr als 1 btn ist gepresst 
	{
		waitForNoInput();
	}
	// Delay
	_delay_ms(10);
}