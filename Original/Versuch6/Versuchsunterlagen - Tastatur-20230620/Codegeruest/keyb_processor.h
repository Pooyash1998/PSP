/*! \file
 *  \brief Keyboard data processor
 *
 *  Contains the third layer to interpret keyboard input.
 *  Contains functionality to register handlers for certain key types and events.
 *
 *  \author     Lehrstuhl Informatik 11 - RWTH Aachen
 *  \date       2013
 *  \version    1.1
 *  \ingroup    keyboard_group
 */

#ifndef KEYB_PROCESSOR_H
#define KEYB_PROCESSOR_H

#include "keyb_usart.h"

//! A handler for a parsed layer 3 key. This could, for example, be used to display the character on a display.
typedef void (*KeyHandler)(Key key);

//! Initializes layer 3, the input processor for data received from layer 2. Call this in keyb_start.
void keyb_init_input_processor();

//! Execute the processor for layer 2 received input.
void keyb_input_process(uint8_t input);

//! Set the processor(s) for layer 3 generated data. Main interface function to application layer.
void keyb_set_processor(KeyType keyType, KeyHandler charHandler);

//! Set the processor that will receive all layer 3 generated data, regardless of type
void keyb_set_main_processor(KeyHandler charHandler);

#endif
