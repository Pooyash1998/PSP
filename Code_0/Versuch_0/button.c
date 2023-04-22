#include "button.h"
#include "lcd.h"

#include <avr/io.h>
#include <stdint.h>
#include <util/delay.h>

/*! \file

Everything that is necessary to get the input from the Buttons in a clean format.

*/

/*!
 *  A simple "Getter"-Function for the Buttons on the evaluation board.\n
 *
 *  \returns The state of the button(s) in the lower bits of the return value.\n
 *  Example: 1 Button:  -pushed:   00000001
 *                      -released: 00000000
 *           4 Buttons: 1,3,4 -pushed: 00001101
 */
// Define the Mappings for button pins
#define BUTTON1_PIN 0
#define BUTTON2_PIN 1
#define BUTTON3_PIN 6
#define BUTTON4_PIN 7

uint8_t getInput(void) {
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
void initInput(void) {
	//set the relevant bits and invert them because 0 means Eingabe
	DDRC &= ~((1 << BUTTON1_PIN) | (1 << BUTTON2_PIN) | (1 << BUTTON3_PIN) | (1 << BUTTON4_PIN)); 
	//now we set the pullupWiderstands (auf 1)
	PORTC |= (1 << BUTTON1_PIN) | (1 << BUTTON2_PIN) | (1 << BUTTON3_PIN) | (1 << BUTTON4_PIN);
	  
}

/*!
 *  Endless loop as long as at least one button is pressed.
 */
void waitForNoInput(void) {
	while (getInput() != 0) 
	{
	  //endlose Schleife
	}
}

/*!
 *  Endless loop until at least one button is pressed.
 */
void waitForInput(void) {
	while (getInput() == 0)
	{
	  //endlose Schleife
	}
}

/*!
 *  Tests all button functions
 */
void buttonTest(void) {
    // Init everything
    initInput();
    lcd_init();
    lcd_clear();
    lcd_writeProgString(PSTR("Initializing\nInput"));
    _delay_ms(1000);
    lcd_clear();
    if ((DDRC & 0b11000011) != 0) {
        lcd_writeProgString(PSTR("DDR wrong"));
        while (1);
    } else {
        lcd_writeProgString(PSTR("DDR OK"));
        _delay_ms(1000);
    }

    lcd_clear();
    if ((PORTC & 0b11000011) != 0b11000011) {
        lcd_writeProgString(PSTR("Pullups wrong"));
        while (1);
    } else {
        lcd_writeProgString(PSTR("Pullups OK"));
        _delay_ms(1000);
    }

    // Test all 4 buttons
    for (uint8_t i = 0; i < 4; i++) {
        lcd_clear();
        lcd_writeProgString(PSTR("Press Button "));
        lcd_writeDec(i + 1);
        uint8_t pressed = 0;
        while (!pressed) {
            waitForInput();
            uint8_t buttonsPressed = getInput();
            if (buttonsPressed != 1 << i) { // Not the right button pressed so show which ones were pressed and wait for release
                lcd_line2();
                lcd_writeProgString(PSTR("Pressed: "));
                lcd_writeHexByte(buttonsPressed);
                waitForNoInput();
                lcd_line2();
                lcd_writeProgString(PSTR("                "));
            } else { // Right button pressed so wait for release and continue to next one
                pressed = 1;
                lcd_clear();
                lcd_writeProgString(PSTR("Release Button "));
                lcd_writeDec(i + 1);
                waitForNoInput();
            }
        }
    }
    lcd_clear();
    lcd_writeProgString(PSTR("Test passed"));
    //while (1);
	_delay_ms(1000);
}
