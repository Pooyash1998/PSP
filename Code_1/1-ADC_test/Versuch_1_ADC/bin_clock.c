#include "bin_clock.h"
#include <avr/io.h>
#include <avr/interrupt.h>

//! Global variables
uint8_t stunden ;
uint8_t minuten ;
uint8_t sekunden ;
uint16_t millisekunden ; //bis 999 daher 16bit

/*!
 * \return The milliseconds counter of the current time.
 */
uint16_t getTimeMilliseconds() {
    return millisekunden;
}

/*!
 * \return The seconds counter of the current time.
 */
uint8_t getTimeSeconds() {
	return sekunden;
}

/*!
 * \return The minutes counter of the current time.
 */
uint8_t getTimeMinutes() {
   return minuten;
}

/*!
 * \return The hour counter of the current time.
 */
uint8_t getTimeHours() {
	return stunden;
}

/*!
 *  Initializes the binary clock (ISR and global variables)
 */
void initClock(void) {
    // Set timer mode to CTC
    TCCR0A &= ~(1 << WGM00);
    TCCR0A |= (1 << WGM01);
    TCCR0B &= ~(1 << WGM02);

    // Set prescaler to 1024
    TCCR0B |= (1 << CS02) | (1 << CS00);
    TCCR0B &= ~(1 << CS01);

    // Set compare register to 195 -> match every 10ms
    OCR0A = 195;

    // Init variables
    stunden = 12;
    minuten = 55;
    sekunden = 33;
    millisekunden = 0;
	
    // Enable timer and global interrupts
    TIMSK0 |= (1 << OCIE0A);
    sei();
}

/*!
 *  Updates the global variables to get a valid 12h-time
 */
void updateClock(void) {
   if (millisekunden >= 1000) {
	   sekunden ++;
	   millisekunden -= 1000;
   }
   if (sekunden >= 60) {
	   minuten ++;
	   sekunden = 0;
   }
   if (minuten >= 60) {
	   stunden ++;
	   minuten = 0;
   }
   if (stunden >= 13) {
	   stunden -= 12;
   }
}

/*!
 *  ISR to increase second-counter of the clock
 */
ISR(TIMER0_COMPA_vect) {
     millisekunden += 10;
     updateClock();
}
