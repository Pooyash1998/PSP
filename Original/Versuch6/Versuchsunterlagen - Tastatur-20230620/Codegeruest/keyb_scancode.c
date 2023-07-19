#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "util.h"
#include "keyb_usart.h"
#include "keyb_scancode.h"

/*!
 *  These char arrays are used as lookup tables for the translation of
 *  unparsed values from the keyboard. Only scan code set 3 is supported.
 *  Notice that the interpretation is completely coded within these tables. All functions
 *  in this module simply apply these lookup tables in an apt way.
 *  Therefore, implementing further scan code sets could be done by defining new lookup tables.
 */

// 0123456789ABCDEF

/*!
 *  Decode the scan code set into a pseudo meaningful char set.
 *  This translation only makes sense with the appropriate types.
 */
const char PROGMEM keyb_tb_scancodeset3[] = "\
_______0E____T^1\
_LS<Cq12_Aysaw23\
_cxde434_ vftr55\
_nbhgz66_Amju787\
_,kio098_.-l�p�9\
__�#��:QLS�+#_;R\
|{P~DFBI_1}47WHU\
0,2568N/_�3-+9*_\
____-______OOM__";
#define KEYB_TB_SCANCODESET3_LEN 0x90

/*!
 *  Decode the scan code set into the types appropriate to the pseudo char set.
 */
const char PROGMEM keyb_tb_scancodeset3_types[] = "\
       53    315\
 3313115 3111115\
 1111115 1111115\
 1111115 4111115\
 1111115 1111115\
  11115444411 54\
66464444 2622444\
22222242 422222 \
    2      344  ";
#define KEYB_TB_SCANCODESET3_TYPES_LEN 0x90

/*!
 *  Shift the keys according to the standard layout.
 *  Applying this lookup multiple times will switch through the different key meanings.
 *  e.g. shift('q') will be 'Q', and shift('Q') == shift(shift('q')) will be '@'.
 *  This translation does not come with an extra type shifter, since the type is not affected
 *  by the shift.
 */
const char PROGMEM keyb_tb_universalShifter[] = "\
                \
                \
   '    []~*;_:{\
=!\"�$%&/()  >}|\\\
             �  \
 @            � \
 ABCDEFGHIJKLMNO\
PQRSTUVWXYZ     \
                \
                \
                \
    `           \
                \
               ?";
#define KEYB_TB_UNIVERSALSHIFTER_LEN 0xE0

/*!
 *  Invert the numpad values to pseudo command representations.
 *  This translation is only meaningful with the appropriate type translation.
 */
const char PROGMEM keyb_tb_universalNumpadInverter[] = "\
                \
                \
          *+D- /\
IF|W{Z}H~U      \
                \
                \
                \
                \
                \
                \
                \
                \
                \
      �         ";
#define KEYB_TB_UNIVERSALNUMPADINVERTER_LEN 0xE0

/*!
 *  This translation generates the appropriate types for the universalNumpadInverter.
 */
const char PROGMEM keyb_tb_universalNumpadInverter_types[] = "\
                \
                \
          2242 2\
4464646464      \
                \
                \
                \
                \
                \
                \
                \
                \
                \
      4         ";
#define KEYB_TB_UNIVERSALNUMPADINVERTER_TYPES_LEN 0xE0

/*!
 *  A function that shifts the passed char.
 *  Applying this function once will modify the passed value like the SHIFT-key would.
 *  Applying this function another time will return a value that would be expected if Alt Graph was pressed.
 *
 *  \param character The (class 1) character to shift.
 *  \return the character after shifting it.
 */
int8_t keyb_shiftChar(uint8_t character) {
    if (character < KEYB_TB_UNIVERSALSHIFTER_LEN) {
        return pgm_read_byte(keyb_tb_universalShifter + character);
    }
    return ' ';
}

/*!
 *  A function to modify the value of the numpad.
 *  The default value from the numpad keys will be the value you would expect if the NUM mode is _ON_.
 *  After applying this function the numpad key will have the meaning as if the NUM mode were _OFF_.
 *
 *  \param keyBuffer  Pointer to the key that should be modified. Make sure type and key are set correctly.
 */
void keyb_unNumChar(Key* keyBuffer) {
    if (keyBuffer->key < KEYB_TB_UNIVERSALNUMPADINVERTER_LEN) {
        // The translation may only be initiated for proper values that come from the numpad
        if (keyBuffer->type == KEYB_KT_NUMPRINTABLE || keyBuffer->key == KEYB_KEY_CTRL_ENTERPAD) {
            keyBuffer->type = pgm_read_byte(keyb_tb_universalNumpadInverter_types + ((unsigned char)(keyBuffer->key))) - '0';
            keyBuffer->key = pgm_read_byte(keyb_tb_universalNumpadInverter + ((unsigned char)(keyBuffer->key)));
        }
    }
}

/*!
 *  Takes a byte produced by layer 2 and translates it to the appropriate key.
 *  \param input    The input to be translated.
 *  \returns        The appropriate key.
 */
uint8_t keyb_translateInputToKey(uint8_t input) {
    if (input < KEYB_TB_SCANCODESET3_LEN) {
        return pgm_read_byte(keyb_tb_scancodeset3 + input);
    }
    return '_';
}

/*!
 *  Takes a byte produced by layer 2 and translates it to the appropriate key type.
 *  \param input    The input to be translated.
 *  \returns        The appropriate key type.
 */
uint8_t keyb_translateInputToKeyType(uint8_t input) {
    if (input < KEYB_TB_SCANCODESET3_TYPES_LEN) {
        return pgm_read_byte(keyb_tb_scancodeset3_types + input) - '0';
    }
    return '_';
}
