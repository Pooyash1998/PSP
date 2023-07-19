#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "util.h"
#include "keyb_usart.h"
#include "keyb_processor.h"

/*
 * The default UART configuration is set to receive and not to transmit.
 * In order to transmit a command to the keyboard, the receiver must be deactivated
 * to prevent it from receiving the byte just transmitted to the keyboard.
 * This workaround is required since the keyboard has just one data port for both
 * receiving and transmitting data.
 * The clock however is used to actually conduct polarity changes on the data port.
 */

// --------- private -------------------------------
bool keyb_transmit(uint8_t command);
uint16_t volatile poll_loop_watchdog;
uint8_t poll_failures;

#define POLL_EVENT(CONDITION) do { \
        poll_loop_watchdog = 60000; \
        while (!(CONDITION) && --poll_loop_watchdog); \
        poll_failures += !poll_loop_watchdog; \
    } while (0)

#define POLL_EVENT_PANIC(CONDITION, PANIC) { \
        poll_failures = 0; \
        POLL_EVENT(CONDITION); \
        if (poll_failures) { PANIC; } \
    }

/*!
 *  The interrupt service routine that handles incoming bytes from the keyboard.
 *  Received bytes are not parsed in any way and directly passed
 *  to keyb_input_process for processing in layer 3.
 */
ISR(USART0_RX_vect) {
    #warning IMPLEMENT STH. HERE
}

/*!
 *  \private
 *  Configures the built in USART module to receive.\r\n
 *  The actual configuration of the interface properties is not handled here.
 */
void keyb_activateReceiver(void) {
    #warning IMPLEMENT STH. HERE
}

/*!
 *  \private
 *  Configures the built in USART module to deactivate the receiver.\r\n
 *  The actual configuration of the interface properties is not handled here.
 */
void keyb_deactivateReceiver(void) {
    #warning IMPLEMENT STH. HERE
}

/*!
 *  Uses polling to receive a byte with the USART interface.
 *  \return The byte sent by the keyboard.
 */
uint8_t keyb_receive() {
    #warning IMPLEMENT STH. HERE
}

/*!
 *  Tries to update the keyboard's scan code set to the given value.
 *  This function is not plug and play safe! If the keyboard
 *  is disconnected or broken, the OS may freeze.
 *  After setting the new scan code set the actual scan code set will be
 *  returned (e.g. for confirmation).
 * 
 *  \param scanCodeSet A value in [0..3]. If 0, the scan code set will
 *                     not be changed. Otherwise the keyboard's
 *                     scan code set will be changed to this value.
 *  \return The actual scan code set of the keyboard.
 */
uint8_t keyb_setScanCodeSet(uint8_t scanCodeSet) {
    #warning IMPLEMENT STH. HERE
}

/*!
 *  Initializes the processor to communicate with the keyboard.
 *  Correctly sets pin directions and values to power the keyboard.
 *  Initializes the internal USART to receive data from the keyboard.
 *  Notice that this function is not plug and play safe.
 *  If the keyboard is disconnected or broken, SPOS may freeze.
 */
void keyb_usart_init(void) {
    #warning IMPLEMENT STH. HERE
}

/*!
 *  Start the communication with the keyboard.
 *  This first calls keyb_usart_init to setup the low level communication
 *  interface with the keyboard. It then configures the scan code set to use
 *  and finally initializes layer 3, the input processor layer.
 *  At the end, enables the USART interrupt to start passively receiving input
 *  from the keyboard.
 */
void keyb_start(void) {
    #warning IMPLEMENT STH. HERE
}

/*!
 *  This _private_ function is used by the keyb_transmit routine to flush
 *  a single bit and wait for the next clock signal.
 *  NEVER CALL THIS FUNCTION DIRECTLY!
 *
 *  \param bit  The bit to send to the keyboard.
 *  \return     The bit that was sent.
 */
bool keyb_send_andWaitForFallingEdge(bool bit) {
    if (bit) {
        sbi(KEYB_DATA_PORT, KEYB_DATA_BIT);
    } else {
        cbi(KEYB_DATA_PORT, KEYB_DATA_BIT);
    }

	POLL_EVENT(gbi(KEYB_CLOCK_PIN, KEYB_CLOCK_BIT));
	POLL_EVENT(!gbi(KEYB_CLOCK_PIN, KEYB_CLOCK_BIT));

    return bit;
}

/*!
 *  Transmit an unverified 8 bit command, encapsulated in a data format and
 *  wait for an acknowledge from the keyboard. This function will deactivate and
 *  reactivate the receiver on itself.
 *
 *  \param command  The command to send to the keyboard.
 *  \return         True if the byte was correctly transmitted.
 */
bool keyb_transmit(uint8_t command) {
    // Deactivate all interrupts
    uint8_t ie = SREG & (1 << SREG_I);
    cli();

    // Deactivate the receiver to avoid any sent data being mistakenly interpreted
    // as received data
    keyb_deactivateReceiver();

    do {
        // To tell the keyboard that we are going to send a command, we have to switch
        // both the clock and data pin to output
        sbi(KEYB_CLOCK_DDR, KEYB_CLOCK_BIT);
        sbi(KEYB_DATA_DDR, KEYB_DATA_BIT);

        // Hold the clock signal low for at least 100 us and then switch the date pin to low as well
        cbi(KEYB_CLOCK_PORT, KEYB_CLOCK_BIT);
        _delay_us(150);

        // Send start bit (start bit is low)
        cbi(KEYB_DATA_PORT, KEYB_DATA_BIT);

        // Set the clock signal back to 1, in order to activate the keyboard's clock generator
        sbi(KEYB_CLOCK_PORT, KEYB_CLOCK_BIT);

        // Set clock to input to synchronize to it
		cbi(KEYB_CLOCK_DDR, KEYB_CLOCK_BIT);
		sbi(KEYB_CLOCK_PORT, KEYB_CLOCK_BIT); // Pull-up

        // Send the 8 bits of the command to be sent.
        // Use the walk_bit to step through bits 0 to 7 of the command.
        // Note that the keyboard expects the data beginning with LSB.
        uint8_t volatile walk_bit = 0;

        // Initialize parity with 1 (odd parity)
        uint8_t parity = 1;

        // Wait for falling edge
		POLL_EVENT_PANIC(!gbi(KEYB_CLOCK_PIN, KEYB_CLOCK_BIT), break);

        // Send bits 0 to 7
        while (walk_bit < 8) {
            parity ^= keyb_send_andWaitForFallingEdge((command >> walk_bit++) & 1);
        }

        // Send the odd parity bit
        keyb_send_andWaitForFallingEdge(parity & 1);

        // Send the final stop bit
        sbi(KEYB_DATA_PORT, KEYB_DATA_BIT);

        // Set data to input to receive ACK bit and final rising + falling edge
		cbi(KEYB_DATA_DDR, KEYB_DATA_BIT);
		sbi(KEYB_DATA_PORT, KEYB_DATA_BIT);

        // Wait for acknowledge bit
		POLL_EVENT_PANIC(!gbi(KEYB_DATA_PIN, KEYB_DATA_BIT), break);
		POLL_EVENT_PANIC(!gbi(KEYB_CLOCK_PIN, KEYB_CLOCK_BIT), break);
		POLL_EVENT_PANIC(gbi(KEYB_DATA_PIN, KEYB_DATA_BIT) || gbi(KEYB_CLOCK_PIN, KEYB_CLOCK_BIT), break);
    } while (0);

    // We're done with the whole transmit mambo jumbo, so turn the receiver back on
    // the keyboard response (if any) can then be received with keyb_receive
    keyb_activateReceiver();

    // Restore previous state of global interrupt enable bit
    SREG |= ie;

    // The transmit was successful if no poll failures occurred
    return poll_failures == 0;
}

/*!
 *  This routine uses the keyb_transmit function to change the LED status of the
 *  keyboard. Notice that the parameter list is sorted according to the typical
 *  layout of a standard keyboard.
 *
 *  \param num  true <=> enable NUM LED
 *  \param caps true <=> enable CAPS LED
 *  \param roll true <=> enable ROLL LED
 */
void keyb_updateLEDs(bool num, bool caps, bool roll) {
    #warning IMPLEMENT STH. HERE
}
