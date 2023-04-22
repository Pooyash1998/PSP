#include "button.h"
#include "led.h"
#include "lcd.h"
#include <avr/io.h>

int main(void) {
	
	initInput();
	lcd_init();
	led_init();
	buttonTest();
	lcd_clear();
	lcd_writeString("now testing LedFUN!");
    /* Replace with your application code */
    while (1) {
		led_fun();
    }
}
