#include <avr/interrupt.h>
#include "tlcd_core.h"
#include "tlcd_parser.h"
#include "tlcd_button.h"
#include "util.h"
#include "lcd.h"
#include "os_core.h"
#include <avr/pgmspace.h>

TouchEvent t_ev;

// Global variable to store the callback function
EventCallback globalCallback = NULL;

// Forward declarations
void tlcd_readData();
void tlcd_parseInputBuffer();
void tlcd_parseTouchEvent();
void tlcd_parseButtonEvent();
void tlcd_parseUnknownEvent();

/*!
 *  Interrupt Service Routine, which is called on Pin Change Interrupt.
 *  This ISR handles the incoming data and calls the parsing process.
 */
ISR(PCINT1_vect) {
    while((PINB & (1 << TLCD_SEND_BUFFER_IND_BIT)) == 0)
	{
		tlcd_requestData();
		tlcd_readData();
		tlcd_parseInputBuffer();
	}
}

/*!
 *  Display a TouchEvent to the Character-LCD.
 *  \param event The TouchEvent to display
 */
void tlcd_displayEvent(TouchEvent event) 
{
	lcd_clear();
	lcd_writeProgString(PSTR("Type: "));
    lcd_writeDec(event.type);
	lcd_line2();
	lcd_writeProgString(PSTR("x: "));
    lcd_writeDec(event.x);
    lcd_writeChar(',');
	lcd_writeProgString(PSTR("y: "));
    lcd_writeDec(event.y);
}

/*!
 *  This function parses the input buffer content by iterating over
 *  it packet wise and calling the specific parser functions.
 *	in the Buffer is just the Usedata and not the Frames!
 */
void tlcd_parseInputBuffer() 
{
	uint8_t cache;
	while(tlcd_hasNextBufferElement())
	{
		cache = tlcd_readNextBufferElement();
		if(cache != ESC_BYTE) 
			os_error("wrong data in Buff!");
		cache = tlcd_readNextBufferElement();
		if(cache == H_BYTE)
			tlcd_parseTouchEvent(); 
		else
			tlcd_parseUnknownEvent();	
	}
	//tlcd_resetBuffer();
}

/*!
 *  This function is called when a free touch panel event packet
 *  has been received. The content of the subframe corresponding
 *  to that event is parsed in this function and delegated to the
 *  event handler tlcd_eventHandler(WithCorrection).
 */
void tlcd_parseTouchEvent() {
	
	uint8_t length = tlcd_readNextBufferElement();
	if(length != 5 || !(tlcd_hasNextBufferElement()))
		os_error("incomplete data in buff!");
	t_ev.type = tlcd_readNextBufferElement();
	
	//set the lower Byte
	uint16_t dbyte = tlcd_readNextBufferElement();
	//read and prepare the high byte
	dbyte |= (tlcd_readNextBufferElement() << 8);
	//set the higher Byte
	t_ev.x = dbyte;
	
	//set the lower Byte
	dbyte = tlcd_readNextBufferElement();
	//read and prepare the high byte
    dbyte |= (tlcd_readNextBufferElement() << 8);
	//set the higher Byte
	t_ev.y = dbyte;
	// display Event to 16*2 LCD of Evaluation Board
	tlcd_displayEvent(t_ev);
	// call the Button Call back
	if(t_ev.type != TOUCHPANEL_DOWN)
	{
		tlcd_displayEvent(t_ev);
		uint8_t ret = 0;
		if(t_ev.type != TOUCHPANEL_DRAG)
			ret = tlcd_handleButtons(t_ev);
		//call the current call-back-function
		if(ret == 0)
			globalCallback(t_ev);
	}
}
/*!
 *  This function is called when an unknown event packet
 *  has been received and skips the payload by manipulating
 *  the tail of the input buffer.
 */
void tlcd_parseUnknownEvent() {
    uint8_t length = tlcd_readNextBufferElement();
	while (length)
	{
		tlcd_readNextBufferElement();
		length--;
	}
}

// Event Call back function for the UserInterface
void tlcd_setEventCallback (EventCallback *callback)
{
	if(callback == NULL)
		globalCallback = NULL;
	else	
		globalCallback = *callback;
}