#include "os_input.h"

#include <avr/io.h>
#include <stdint.h>

/*! \file

Everything that is necessary to get the input from the Buttons in a clean format.

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

#define BUTTON1_PIN 0 //Enter
#define BUTTON2_PIN 1 //Down
#define BUTTON3_PIN 6 //Up
#define BUTTON4_PIN 7 // ESC

uint8_t os_getInput(void) {
    uint8_t buttonMask = PINC & 0b11000011; // Read the Values of the Buttons
    uint8_t result = 0;
    // Check if button 1 is pushed
    if (!(buttonMask & (1 << BUTTON1_PIN)))  //invert because 0 means pushed and 1 means released
    {
	    result |= (1 << 0); // Set the corresponding bit in the return value
    }

    // Check if button 2 is pushed
    if (!(buttonMask & (1 << BUTTON2_PIN))) {
	    result |= (1 << 1);
    }

    // Check if button 3 is pushed
    if (!(buttonMask & (1 << BUTTON3_PIN))) {
	    result |= (1 << 2);
    }

    // Check if button 4 is pushed
    if (!(buttonMask & (1 << BUTTON4_PIN))) {
	    result |= (1 << 3);
    }

    return result; // Return the result
}

/*!
 *  Initializes DDR and PORT for input
 */
void os_initInput() {
    //set the relevant bits and invert them because 0 means Eingabe
    DDRC &= ~((1 << BUTTON1_PIN) | (1 << BUTTON2_PIN) | (1 << BUTTON3_PIN) | (1 << BUTTON4_PIN));
    //now we set the pullupWiderstands (auf 1)
    PORTC |= (1 << BUTTON1_PIN) | (1 << BUTTON2_PIN) | (1 << BUTTON3_PIN) | (1 << BUTTON4_PIN);
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
