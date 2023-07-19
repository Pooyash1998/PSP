#include "tlcd_button.h"
#include "tlcd_graphic.h"
#include "tlcd_parser.h"
#include "defines.h"

Button buttonBuffer[MAX_BUTTONS];
// Number of elements currently in the array
uint8_t btn_count = 0;

// Global variable to store the Button callback function
ButtonCallback g_btn_Callback = NULL;

/*!
 *  Add a button to the screen and internal logic to be handled by the handleButtons function.
 *  In addition the button will present a character as label.
 *  \param x1 X-coordinate of the starting point
 *  \param y1 Y-coordinate of the starting point
 *  \param x2 X-coordinate of the ending point
 *  \param y2 Y-coordinate of the ending point
 *  \param color The color index of the color the button should be drawn in
 *  \param downCode The code to be send if the button is pressed
 *  \param c The character to display on the button
 */
void tlcd_addButtonWithChar(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t colorID, uint8_t downCode, char c) 
{
	if(btn_count < MAX_BUTTONS)
	{
    buttonBuffer[btn_count].btn_code = downCode;
	buttonBuffer[btn_count].colorID = colorID;
	buttonBuffer[btn_count].u_l_pos.x = x1;
	buttonBuffer[btn_count].u_l_pos.y = y1;
	buttonBuffer[btn_count].d_r_pos.x = x2;
	buttonBuffer[btn_count].d_r_pos.y = y2;
	buttonBuffer[btn_count].label = c;
	btn_count ++;
	}
}
/*!
 *  Draw the buttons onto the screen.
 *  This function should be called whenever the screen was cleared.
 */
void tlcd_drawButtons() 
{
	if(btn_count > 0)
	{	
		uint16_t mid_x = 0;
		uint16_t mid_y = 0;
		for(uint8_t i = 0 ; i < btn_count ;i++)
		{
			tlcd_drawBox(buttonBuffer[i].u_l_pos.x, buttonBuffer[i].u_l_pos.y,
							buttonBuffer[i].d_r_pos.x, buttonBuffer[i].d_r_pos.y, buttonBuffer[i].colorID);
			
			mid_x = (buttonBuffer[i].u_l_pos.x + buttonBuffer[i].d_r_pos.x) / 2 ;
			mid_y =	(buttonBuffer[i].u_l_pos.y + buttonBuffer[i].d_r_pos.y) / 2 ;	
			tlcd_drawChar(mid_x, mid_y, buttonBuffer[i].label);				
		}
	}
}
/*!
 *  Check an event against all buttons for collision and execute button function if a button was pressed.
 *  Will return 1 if the event was inside a button and a 0 otherwise.
 *  \param event The touch event to handle
 *  \return 1 if event was handled, 0 otherwise
 */
uint8_t tlcd_handleButtons(TouchEvent event) 
{
	uint16_t x_t = event.x;
	uint16_t y_t = event.y;
    for(uint8_t i = 0 ; i < btn_count ;i++)
	{
		uint16_t x1 = buttonBuffer[i].u_l_pos.x;
		uint16_t y1 = buttonBuffer[i].u_l_pos.y;
		uint16_t x2 = buttonBuffer[i].d_r_pos.x;
		uint16_t y2 = buttonBuffer[i].d_r_pos.y;
		// check if touch event is inside the btn
		if( ((x1 <= x_t) && (x_t <= x2)))
			{
				if(((y1 <= y_t) && (y_t <= y2)))
				{
					position pos = {.x = x_t , .y = y_t}; //#problem Source
					g_btn_Callback(buttonBuffer[i].btn_code, pos);
					return 1;
				}
			}	
	}
    return 0;
}
// Callback function
void tlcd_setButtonCallback(ButtonCallback *btnCallback)
{
	if(btnCallback == NULL)
	g_btn_Callback = NULL;
	else
	g_btn_Callback = *btnCallback;
}