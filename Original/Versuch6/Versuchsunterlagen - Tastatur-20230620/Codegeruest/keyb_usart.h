/*! \file
 *  \brief Keyboard USART driver.
 *
 *  Contains the first and second layer to communicate with a standard
 *    keyboard via PS/2.
 *
 *  Layer overview
 *    (physical)      1:  Bidirectional, physical communication with the keyboard via currency potentials.
 *    (transport)     2:  Basic protocol to talk and listen to the keyboard. Received values not parsed.
 *                          Primitives:
 *                           keyb_receive
 *                           keyb_transmit
 *    (presentation)  3:  Rudimentary layer to decode scan code encrypted messages from the keyboard.
 *                        This includes character classification (via types) and character identification.
 *                          Primitives:
 *                            keyb_input_process
 *    (application)   4:  Overlay processing (e.g. printing). This is an application layer (not implemented here).
 *
 *  \author     Lehrstuhl Informatik 11 - RWTH Aachen
 *  \date       2013
 *  \version    1.1
 *  \ingroup    keyboard_group
 */

#ifndef KEYB_USART_H
#define KEYB_USART_H

// These are port and pin definitions to make the actual communication
// better maintainable.

// The following defines should be used in keyb_usart_init

// Registers used to configure the DATA pin
#define KEYB_DATA_PORT      #warning SPECIFY ME
#define KEYB_DATA_DDR       #warning SPECIFY ME
#define KEYB_DATA_PIN       #warning SPECIFY ME
#define KEYB_DATA_BIT       #warning SPECIFY ME

// Registers used to configure the CLOCK pin
#define KEYB_CLOCK_PORT     #warning SPECIFY ME
#define KEYB_CLOCK_DDR      #warning SPECIFY ME
#define KEYB_CLOCK_PIN      #warning SPECIFY ME
#define KEYB_CLOCK_BIT      #warning SPECIFY ME

// Registers used to power the keyboard
#define KEYB_POWER_PORT     #warning SPECIFY ME
#define KEYB_POWER_DDR      #warning SPECIFY ME
#define KEYB_POWER_PIN      #warning SPECIFY ME

// Commands the keyboard will understand
#define KEYB_COMMAND_RESET          0xFF
#define KEYB_COMMAND_SCANCODESET    0xF0
#define KEYB_COMMAND_SETALLKEYS_TMB 0xFA
#define KEYB_COMMAND_UPDATE_LEDS    0xED
#define KEYB_RESPONSE_ACKNOWLEDGE   0xFA
#define KEYB_RESPONSE_SELFTEST      0xAA

// Internal meta characters used to represent non ASCII control commands
// with printable values.
#define KEYB_KEY_CTRL_ESC       0x45
#define KEYB_KEY_CTRL_TAB       0x54
#define KEYB_KEY_CTRL_CAPS      0x43
#define KEYB_KEY_CTRL_SHIFT     0x53
#define KEYB_KEY_CTRL_CTRL      0x4c
#define KEYB_KEY_CTRL_APPS      0x4f
#define KEYB_KEY_CTRL_ALT       0x41
#define KEYB_KEY_CTRL_MENU      0x4d
#define KEYB_KEY_CTRL_ENTER     0xc4
#define KEYB_KEY_CTRL_BACK      0x42
#define KEYB_KEY_CTRL_INSERT    0x49
#define KEYB_KEY_CTRL_DELETE    0x44
#define KEYB_KEY_CTRL_HOME      0x48
#define KEYB_KEY_CTRL_END       0x46
#define KEYB_KEY_CTRL_PAGEUP    0x55
#define KEYB_KEY_CTRL_PAGEDOWN  0x57
#define KEYB_KEY_CTRL_PRINT     0x51
#define KEYB_KEY_CTRL_ROLL      0x52
#define KEYB_KEY_CTRL_PAUSE     0x50
#define KEYB_KEY_CTRL_NUM       0x4e
#define KEYB_KEY_CTRL_ENTERPAD  0xd6
#define KEYB_KEY_CTRL_NONE      0x5a
#define KEYB_KEY_ARROW_LEFT     0x7b
#define KEYB_KEY_ARROW_DOWN     0x7c
#define KEYB_KEY_ARROW_RIGHT    0x7d
#define KEYB_KEY_ARROW_UP       0x7e
#define KEYB_KEY_FKEY(F)        ('0'+F-1)

/*!
 *  The recognized key classes used to identify a character prior to being fully parsed by layer 3.
 *  \see keyb_translateInputToKeyType
 */
typedef enum {
    KEYB_KT_UNKNOWN         = 0xF0, //!< Unknown key type. Returned for invalid input.
    KEYB_KT_PRINTABLE       = 1,    //!< printable character from main key block
    KEYB_KT_NUMPRINTABLE    = 2,    //!< printable character from numpad
    KEYB_KT_LCONTROL        = 3,    //!< left control key
    KEYB_KT_RCONTROL        = 4,    //!< right control key
    KEYB_KT_FKEY            = 5,    //!< F-key (F1, F2, .., F12)
    KEYB_KT_ARROW           = 6     //!< arrow keys
} KeyType;
#define KEYB_KEY_TYPES 7

/*!
 *  The various modifiers that can affect other keys when active.
 *  Note that these are flags, so a Key can have multiple active KeyModifiers.
 */
typedef enum {
    KEYB_MOD_LALT   = 1 << 0, //!< Left ALT key
    KEYB_MOD_RALT   = 1 << 1, //!< Right ALT key
    KEYB_MOD_CTRL   = 1 << 2, //!< CTRL (left and right)
    KEYB_MOD_LSHIFT = 1 << 3, //!< SHIFT (left)
    KEYB_MOD_RSHIFT = 1 << 4  //!< SHIFT (right)
} KeyModifier;

/*!
 *  Meta information about a key
 */
typedef enum {
    KEYB_KM_MAKE, //!< MAKE denotes a key that was pressed
    KEYB_KM_BREAK //!< BREAK denotes a key that was released
} KeyMeta;

/*!
 *  A layer 3 pseudo-parsed key.
 */
typedef struct {
    KeyType         type;     //!< type from keyb_translateInputToKeyType
    uint8_t         key;      //!< key from keyb_translateInputToKey after translations
    KeyMeta         meta;     //!< either KEYB_KM_MAKE or KEYB_KM_BREAK
    KeyModifier     modifier; //!< a combination of KeyModifiers
} Key;

//! Start the keyboard
void keyb_start(void);

//! Initialize the lowlevel communication interface and power supply to keyboard
void keyb_usart_init(void);

//! Set the status of the LEDs on the keyboard.
void keyb_updateLEDs(bool num, bool caps, bool roll);

#endif
