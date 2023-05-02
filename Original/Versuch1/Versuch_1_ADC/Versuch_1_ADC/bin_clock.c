#include "bin_clock.h"
#include <avr/io.h>
#include <avr/interrupt.h>

//! Global variables
#warning IMPLEMENT STH. HERE

/*!
 * \return The milliseconds counter of the current time.
 */
uint16_t getTimeMilliseconds() {
    #warning IMPLEMENT STH. HERE
}

/*!
 * \return The seconds counter of the current time.
 */
uint8_t getTimeSeconds() {
    #warning IMPLEMENT STH. HERE
}

/*!
 * \return The minutes counter of the current time.
 */
uint8_t getTimeMinutes() {
    #warning IMPLEMENT STH. HERE
}

/*!
 * \return The hour counter of the current time.
 */
uint8_t getTimeHours() {
    #warning IMPLEMENT STH. HERE
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
    #warning IMPLEMENT STH. HERE

    // Enable timer and global interrupts
    TIMSK0 |= (1 << OCIE0A);
    sei();
}

/*!
 *  Updates the global variables to get a valid 12h-time
 */
void updateClock(void) {
    #warning IMPLEMENT STH. HERE
}

/*!
 *  ISR to increase second-counter of the clock
 */
ISR(TIMER0_COMPA_vect) {
    #warning IMPLEMENT STH. HERE
}
