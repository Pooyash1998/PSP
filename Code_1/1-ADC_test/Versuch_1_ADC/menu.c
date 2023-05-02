#include "menu.h"
#include "os_input.h"
#include "bin_clock.h"
#include "lcd.h"
#include "led.h"
#include "adc.h"
#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>

/*!
 *  Hello world program.
 *  Shows the string 'Hello World!' on the display.
 */
void helloWorld(void) {
    // Repeat until ESC gets pressed
    while (1)
    {
		lcd_clear();
		lcd_writeProgString(PSTR("Hallo Welt!"));
		_delay_ms(500);
		lcd_clear();
		_delay_ms(500);
		//check for the ESC button
		if(os_getInput() == (1 << 3))
			break;
    }
}

/*!
 *  Shows the clock on the display and a binary clock on the led bar.
 */
void displayClock(void) {
   initLedBar();
   while(1)
   {
	   uint8_t hours = getTimeHours();
	   uint8_t min = getTimeMinutes();
	   uint8_t sec = getTimeSeconds();
	   uint16_t mili =getTimeMilliseconds();
	   
	   uint16_t clockVal = 0;
	   clockVal |= (sec & 0b111111);
	   clockVal |= ((min & 0b111111) << 6 );
	   clockVal |= ((hours & 0b1111) << 12);
	   setLedBar(clockVal);
	   
	   lcd_clear();
	   //schreibe die Uhrzeit in HH Fromat 
	   if (hours < 10) {
		   lcd_writeChar('0'); // schreibe fuehrende Null
	   }
	   lcd_writeDec(hours); 
	   lcd_writeChar(':');
	   
	   //schreibe die Minuten in MM Fromat
	   if (min < 10) {
		   lcd_writeChar('0'); // schreibe fuehrende Null
	   }
	   lcd_writeDec(min);
	   lcd_writeChar(':');
	   
	   //schreibe die Sekunden in SS Fromat
	   if (sec < 10) {
		   lcd_writeChar('0'); // schreibe fuehrende Null
	   }
	   lcd_writeDec(sec);
	   lcd_writeChar(':');
	   
	   //schreibe die Milis in mmm Fromat
	   if (mili < 10)
		   lcd_writeProgString(PSTR("00")); // schreibe fuehrende Null
	   else if(mili < 100)
		   lcd_writeProgString(PSTR("0"));
	   lcd_writeDec(mili);
	   
	   //check for the ESC button
	   if(os_getInput() == (1 << 3)){
	   PORTA = 0xFF; //leds wieder ausschalten
	   PORTD = 0xFF;
	   break;
	   }
	   _delay_ms(100); // so schnell kann doch sich das LCD nicht aendern wir brauchen eine kleine delay 
   }
}

/*!
 *  Shows the stored voltage values in the second line of the display.
 */
void displayVoltageBuffer(uint8_t displayIndex) {
   lcd_erase(2);
   lcd_line2();
   
   if((displayIndex + 1) < 10) //fuhrende Null
	    lcd_writeProgString(PSTR("00"));
   else if((displayIndex + 1) < 100)
		lcd_writeProgString(PSTR("0"));	
		   
   lcd_writeDec(displayIndex + 1); // we want to have the list starting from 1 to 100
   lcd_writeProgString(PSTR("/100: "));
   lcd_writeVoltage(getStoredVoltage(displayIndex), 1023, 5);	   
}

/*!
 *  Shows the ADC value on the display and on the led bar.
 */
void displayAdc(void) {
	initLedBar();
    initAdc(); 
    lcd_clear();
	int8_t  zaelindex = 0;
	uint16_t ledValue = 0;
	uint16_t adcResult= 0;
	//untere Zeile initialisieren
	lcd_line2();
	lcd_writeProgString(PSTR("000/100: ---"));
    while(1)
    {	
		//initLedBar(); //check if needed
		lcd_erase(1);
		lcd_line1();
		lcd_writeProgString(PSTR("Voltage: "));
		adcResult = getAdcValue();
	    //param voltage           Binary voltage value.
	    //param valueUpperBound   Upper bound of the binary voltage value (i.e. 1023 for 10-bit value).
	    //param voltUpperBound    Upper bound of the float voltage value (i.e. 5 for 5V).
	    lcd_writeVoltage(adcResult, 1023, 5);
		
		//***********LED BARGRAPH***********************
		if(adcResult == 0)
		setLedBar(adcResult);
		
	  if(adcResult !=0){ //Wenn voltage is 0 avoid endless loop while calculation ledValue]
			
		while(adcResult>= 68)
		{
			ledValue = ledValue << 1 ; //PINA0 ignorieren
			ledValue |= 2 ;
			adcResult -= 68;			
		}
		setLedBar(ledValue); 
		}
		ledValue = 0;
		//************ Menu ****************
		if(os_getInput() == (1 << 0)) //Enter
		{
	       storeVoltage();
		}
		else if(os_getInput() == (1 << 1)) //Down
		{
			_delay_ms(100);
			if(zaelindex > 0)
			{
				 zaelindex--;
			}
			
			if(getBufferIndex() != 0) //if buffer not empty
			 displayVoltageBuffer(zaelindex);
			
			
		}
		else if(os_getInput() == (1 << 2)) //up
		{
			_delay_ms(100);
			if(zaelindex < getBufferIndex() -1 )
			{	zaelindex++;
				
			}
			
			if(getBufferIndex() !=0 ) //if buffer not empty
			displayVoltageBuffer(zaelindex);

			
		}
	    //check for the ESC button
	    else if(os_getInput() == (1 << 3)){
		    break;
	    }
		_delay_ms(100);
    }
   
	//free(bufferstart) ??
   
}

/*! \brief Starts the passed program
 *
 * \param programIndex Index of the program to start.
 */
void start(uint8_t programIndex) {
    // Initialize and start the passed 'program'
    switch (programIndex) {
        case 0:
            lcd_clear();
            helloWorld();
            break;
        case 1:
            activateLedMask = 0xFFFF; // Use all LEDs
            initLedBar();
            initClock();
            displayClock();
            break;
        case 2:
            activateLedMask = 0xFFFE; // Don't use LED 0
            initLedBar();
            initAdc();
            displayAdc();
            break;
        default:
            break;
    }

    // Do not resume to the menu until all buttons are released
    os_waitForNoInput();
}

/*!
 *  Shows a user menu on the display which allows to start subprograms.
 */
void showMenu(void) {
    uint8_t pageIndex = 0;

    while (1) {
        lcd_clear();
        lcd_writeProgString(PSTR("Select:"));
        lcd_line2();

        switch (pageIndex) {
            case 0:
                lcd_writeProgString(PSTR("1: Hello world"));
                break;
            case 1:
                lcd_writeProgString(PSTR("2: Binary clock"));
                break;
            case 2:
                lcd_writeProgString(PSTR("3: Internal ADC"));
                break;
            default:
                lcd_writeProgString(PSTR("----------------"));
                break;
        }

        os_waitForInput();
        if (os_getInput() == 0b00000001) { // Enter
            os_waitForNoInput();
            start(pageIndex);
        } else if (os_getInput() == 0b00000100) { // Up
            os_waitForNoInput();
            pageIndex = (pageIndex + 1) % 3;
        } else if (os_getInput() == 0b00000010) { // Down
            os_waitForNoInput();
            if (pageIndex == 0) {
                pageIndex = 2;
            } else {
                pageIndex--;
            }
        }
    }
}
