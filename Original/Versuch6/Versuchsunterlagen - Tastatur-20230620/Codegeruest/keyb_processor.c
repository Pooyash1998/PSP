#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "defines.h"
#include "util.h"
#include "keyb_usart.h"
#include "keyb_scancode.h"
#include "keyb_processor.h"
#include "lcd.h"

/*!
 *  Initialization of layer 3, the input processor.
 *  Initializes all layer 3 state, e.g. the status of the various keyboard LEDs
 *  and the currently active modifiers. This is called from keyb_start.
 */
void keyb_init_input_processor() {
    #warning IMPLEMENT STH. HERE
}

/*!
 *  Set the main key processor.
 *  This handler will receive all completely processed keys regardless of type.
 *  \param charHandler  The handler that will receive all keys
 */
void keyb_set_main_processor(KeyHandler charHandler) {
    #warning IMPLEMENT STH. HERE
}

/*!
 *  This function runs after keyb_char_modifiers.
 *  Calls the both the main handler and the char handlers for specific types.
 */
void keyb_char_process() {
    #warning IMPLEMENT STH. HERE
}

/*!
 *  This function modifies keys detected in keyb_input_process.
 *  It keeps track of the currently active modifiers and
 *  translates keys further (e.g. if the SHIFT modifier is active,
 *  calls keyb_shiftChar).
 *  Also respects the current state of CAPS and NUM.
 */
void keyb_char_modifiers() {
    #warning IMPLEMENT STH. HERE
}

/*!
 *  This function takes unprocessed bytes from layer 2 and parses them into the layer 3 key buffer.
 *  Uses the translate functions from keyb_scancode.h to get key and key type for an input byte
 *  and updates the key buffer. Also sets the active modifiers.
 *  If the translations yielded a valid result, hands off to keyb_char_modifiers for further processing
 *  of the key.
 *
 * \param input The unprocessed byte from layer 2.
 */
void keyb_input_process(uint8_t input) {
    #warning IMPLEMENT STH. HERE
}

/*!
 *  With this function a simple trigger mechanism can be established. It allows to specify a hander
 *    function for every character class independently.
 *  \param keyType The keyType denotes the class of the key the caller is to be configured for.
 *  \param charHandler  The handler for the layer 3 parsed character. You may set this to NULL to deactivate
 *                        the trigger for the specified key class.
 */
void keyb_set_processor(KeyType keyType, KeyHandler charHandler) {
    #warning IMPLEMENT STH. HERE
}
