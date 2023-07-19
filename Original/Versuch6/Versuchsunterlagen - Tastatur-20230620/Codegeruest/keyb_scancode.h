/*! \file
 *  \brief Static scan code set lookup tables and auxiliary functions
 *
 *  \author     Lehrstuhl Informatik 11 - RWTH Aachen
 *  \date       2013
 *  \version    1.1
 *  \ingroup    keyboard_group
 */

#ifndef KEYB_SCANCODE_H
#define KEYB_SCANCODE_H

#include <stdint.h>

//! Shift the passed key, i.e. return the key as if the SHIFT-key was pressed
int8_t keyb_shiftChar(uint8_t character);

//! Translate a byte from the keyboard to an actual character
uint8_t keyb_translateInputToKey(uint8_t input);

//! Translate a byte from the keyboard to the respective key type
uint8_t keyb_translateInputToKeyType(uint8_t input);

//! Modify the meaning of a num key.
void keyb_unNumChar(Key* keyBuffer);

#endif
