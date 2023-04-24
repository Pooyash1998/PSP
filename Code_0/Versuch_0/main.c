#include "button.h"
#include "led.h"
#include "lcd.h"
#include "datatypes.h"
#include "structures.h"
#include "pointer.h"
#include <avr/io.h>

int main(void) {
	
	initInput();
	lcd_init();
	led_init();
	//buttonTest();
	//lcd_clear();
	//lcd_writeString("now testing LedFUN!");
    /* Replace with your application code */
	//loop();
	//convert();
	//shift();
	//displayArticles();
	learningPointers();
	inspectMe();
    while (1) {
		//uncomment for test
		//led_fun();  
    }
	
}
