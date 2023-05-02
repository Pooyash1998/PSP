#include "os_input.h"

#include <avr/io.h>
#include <stdint.h>

/*! \file
 * Everything that is necessary to get the input from the Buttons in a clean format.
 */


/*!
 *  A simple "Getter"-Function for the Buttons on the evaluation board.\n
 *
 *  \returns The state of the button(s) in the lower bits of the return value.\n
 *  example: 1 Button:  -pushed:   00000001
 *                      -released: 00000000
 *           4 Buttons: 1,3,4 -pushed: 000001101
 *
 */

// Define the Mappings for button pins
#define BUTTON1_PIN 0
#define BUTTON2_PIN 1
#define BUTTON3_PIN 6
#define BUTTON4_PIN 7

uint8_t os_getInput(void) {
	//der Code ist aus Versuch0 hier brauchen wir aber nur (pin1) btn_C
   uint8_t buttonMask = PINC & 0b00000010; // Read the Values of the Buttons
   uint8_t result = 0;
   // Check if button 2 is pushed
   if (!(buttonMask & (1 << BUTTON2_PIN)))  //invert because 0 means pushed and 1 means released
   {
	   result |= (1 << 1); // Set the corresponding bit in the return value
   }
/*
   // Check if button 1 is pushed
   if (!(buttonMask & (1 << BUTTON1_PIN))) {
	   result |= (1 << 0);
   }

   // Check if button 3 is pushed
   if (!(buttonMask & (1 << BUTTON3_PIN))) {
	   result |= (1 << 2);
   }

   // Check if button 4 is pushed
   if (!(buttonMask & (1 << BUTTON4_PIN))) {
	   result |= (1 << 3);
   }
*/
   return result; // Return the result
}

/*!
 *  Initializes DDR and PORT for input
 */
void os_initInput() {
	//CODE von Versuch0 benutzt ( hier nur C1 ist vorhanden!) 
	
    //set the relevant bits and invert them because 0 means Eingabe
    DDRC &= ~ (1 << BUTTON2_PIN);
    //now we set the pullupWiderstands (auf 1)
    PORTC |= (1 << BUTTON2_PIN) ;
}

/*!
 *  Endless loop as long as at least one button is pressed.
 */
void os_waitForNoInput() {
   while (os_getInput() != 0)
   {
	   //endlose Schleife
   }
}

/*!
 *  Endless loop until at least one button is pressed.
 */
void os_waitForInput() {
   while (os_getInput() == 0)
   {
	   //endlose Schleife
   }
}
