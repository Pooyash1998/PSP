//-------------------------------------------------
//          Keyboard: Text editor
//-------------------------------------------------

#include "lcd.h"
#include "os_scheduler.h"
#include "keyb_processor.h"

void FKeyHandler(Key key) {
    // We don't care if the user releases an F key
    if (key.meta != KEYB_KM_MAKE) {
        return;
    }
    int8_t f = key.key - '0';
    int8_t i;
    lcd_clear();
    
    // F12
    if (f == 11) {
        lcd_writeProgString(PSTR("special chars:"));
        lcd_writeChar('\n');
        for (i = 0; i < 6; i++) {
            lcd_writeChar(i);
        }
    } else {
        f %= 8;
        for (i = 0; i < 32; i++) {
            lcd_writeChar(32 * f + i);
        }
    }
}

void AllKeyHandler(Key keyBuffer) {
    static int8_t lastWritten = ' ';

    if (keyBuffer.meta == KEYB_KM_BREAK) {
        return;
    }

    switch (keyBuffer.type) {
        case KEYB_KT_NUMPRINTABLE:
        case KEYB_KT_PRINTABLE:
            lcd_writeChar(lastWritten = keyBuffer.key);
            break;

        case KEYB_KT_LCONTROL:
        case KEYB_KT_RCONTROL:
            switch (keyBuffer.key) {
                case KEYB_KEY_CTRL_ESC:
                    lcd_clear();
                    break;
                case KEYB_KEY_CTRL_TAB:
                    for (uint8_t i = 0; i < 4; i++) {
                        lcd_writeChar(' ');
                    }
                    break;
                case KEYB_KEY_CTRL_INSERT:
                    lcd_writeChar(lastWritten);
                    break;
                case KEYB_KEY_CTRL_DELETE:
                    lcd_writeChar(' ');
                    lcd_back();
                    break;
                case KEYB_KEY_CTRL_BACK:
                    lcd_back();
                    lcd_writeChar(' ');
                    lcd_back();
                    break;
                case KEYB_KEY_CTRL_HOME:
                    lcd_home();
                    break;
                case KEYB_KEY_CTRL_END:
                    lcd_home();
                    lcd_move(0, 15);
                    break;
                case KEYB_KEY_CTRL_PAGEUP:
                    // Shift display to left
                    lcd_command(0b11000);
                    break;
                case KEYB_KEY_CTRL_PAGEDOWN:
                    // Shift display to right
                    lcd_command(0b11100);
                    break;
                case KEYB_KEY_CTRL_ENTER:
                case KEYB_KEY_CTRL_ENTERPAD:
                    lcd_writeChar('\n');
                    break;
                case KEYB_KEY_CTRL_PRINT:
                    lcd_writeChar(0);
                    break;
                default:
                    break;
            }
            break;
        case KEYB_KT_ARROW:
            switch (keyBuffer.key) {
                case KEYB_KEY_ARROW_LEFT:
                    lcd_move(0, -1);
                    break;
                case KEYB_KEY_ARROW_DOWN:
                    lcd_move(1, 0);
                    break;
                case KEYB_KEY_ARROW_RIGHT:
                    lcd_move(0, 1);
                    break;
                case KEYB_KEY_ARROW_UP:
                    lcd_move(-1, 0);
                    break;
            }
            break;

        default: ;
    }
}

/*!
 *  This process only initializes the keyboard and prints the welcome message
 *  keys are received through callbacks to FKeyHandler (for F1..F12)
 *  and AllKeyHandler (catch-all)
 */
REGISTER_AUTOSTART(textEditor)
void textEditor(void) {
    keyb_start();
    keyb_set_processor(KEYB_KT_FKEY, FKeyHandler);
    keyb_set_main_processor(AllKeyHandler);
    lcd_clear();
    
    // Enable display and cursor
    lcd_command(0b1110);
    lcd_writeProgString(PSTR("Lehrstuhl f�r      INFORMATIK 11"));
    while (true);
}
