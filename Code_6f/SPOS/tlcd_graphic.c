#include <stdint.h>
#include "tlcd_core.h"
#include "tlcd_graphic.h"


/*!
 *  Define a touch area for which the TLCD will send touch events
 *  \param x1 X-coordinate of the starting point
 *  \param y1 Y-coordinate of the starting point
 *  \param x2 X-coordinate of the ending point
 *  \param y2 Y-coordinate of the ending point
 */
void tlcd_defineTouchArea(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) 
{
    unsigned char cmd[] = {ESC_BYTE , A_BYTE , H_BYTE , 
		LOW(x1) , HIGH(x1) , LOW(y1) , HIGH(y1) , LOW(x2) , HIGH(x2) ,LOW(y2) , HIGH(y2)};
	tlcd_sendCommand(cmd, sizeof(cmd)); 
}

/*!
 *  This function draws a point (x1,y1) on the display.
 *  \param x1 The position on the x-axis.
 *  \param y1 The position on the y-axis.
 */
void tlcd_drawPoint(uint16_t x1, uint16_t y1) 
{
    unsigned char cmd[] = {ESC_BYTE, G_BYTE, P_BYTE, 
		LOW(x1), HIGH(x1), LOW(y1), HIGH(y1)};
	tlcd_sendCommand(cmd, sizeof(cmd));	
}

/*!
 *  This function draws a line in graphic mode from the point (x1,y1) to (x2,y2)
 *  \param x1 X-coordinate of the starting point
 *  \param y1 Y-coordinate of the starting point
 *  \param x2 X-coordinate of the ending point
 *  \param y2 Y-coordinate of the ending point
 */
void tlcd_drawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) 
{
    unsigned char cmd[] = {ESC_BYTE, G_BYTE, D_BYTE,
		LOW(x1), HIGH(x1), LOW(y1), HIGH(y1), LOW(x2), HIGH(x2),LOW(y2), HIGH(y2)};
    tlcd_sendCommand(cmd, sizeof(cmd));
}

/*!
 *  Draw a colored box at given coordinates
 *  \param x1 X-coordinate of the starting point
 *  \param y1 Y-coordinate of the starting point
 *  \param x2 X-coordinate of the ending point
 *  \param y2 Y-coordinate of the ending point
 */
void tlcd_drawBox(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t color) 
{
   unsigned char cmd[] = {ESC_BYTE, R_BYTE, F_BYTE,
		LOW(x1), HIGH(x1), LOW(y1), HIGH(y1), LOW(x2), HIGH(x2),LOW(y2), HIGH(y2), color};
   tlcd_sendCommand(cmd, sizeof(cmd));
}

/*!
 *  Change the size of lines being drawn
 *  \param size The new size for the lines
 */
void tlcd_changePenSize(uint8_t size) 
{
	if(size <= 15)
	{
		unsigned char cmd[] = {ESC_BYTE, G_BYTE, Z_BYTE, size, size};
		tlcd_sendCommand(cmd, sizeof(cmd));	
	}
}

/*!
 *  Define the color at a given index. Not all "bits" are used, refer to data sheet.
 *  \param colorID The color index to change.
 *  \param The color to set.
 */
void tlcd_defineColor(uint8_t colorID, Color color) 
{
	if(colorID > 0 && colorID <= 32)
	{
		unsigned char cmd[] = {ESC_BYTE, F_BYTE, P_BYTE, colorID, color.red, color.green, color.blue};
		tlcd_sendCommand(cmd, sizeof(cmd));
	}
}

/*!
 *  Change the color lines are drawn in
 *  \param color Index of the color lines should be drawn in
 */
void tlcd_changeDrawColor(uint8_t colorID) 
{
	//by color is actually meant "colorID"  
   if(colorID > 0 && colorID <= 32)
   {
	   // 255 is no change for bg_color which is irrelevant here
	   unsigned char cmd[] = {ESC_BYTE, F_BYTE, G_BYTE, colorID, 255};
	   tlcd_sendCommand(cmd, sizeof(cmd));
   }
}

/*!
 *  Draw a character c at position (x1,y1).
 *  \param c Character to draw
 *  \param x1 X coordinate
 *  \param x1 Y coordinate
 */
void tlcd_drawChar(uint16_t x1, uint16_t y1, char c) 
{
	if(c != 0)
	{
    unsigned char cmd[] = {ESC_BYTE, Z_BYTE, C_BYTE, LOW(x1), HIGH(x1), LOW(y1), HIGH(y1), c , 0};
	tlcd_sendCommand(cmd, sizeof(cmd));	
	}
}

/*! 
 *  Enable or disable the Cursor.
 *  \param enable Enable(1) or disable(0) the cursor.
 */
void tlcd_setCursor(uint8_t enabled) 
{
	unsigned char cmd[] = {ESC_BYTE, T_BYTE, C_BYTE, enabled};
	tlcd_sendCommand(cmd, sizeof(cmd));  	   
}

/*!
 *  This function clears the display (fills hole display with background color).
 */
void tlcd_clearDisplay() 
{
   unsigned char cmd[] = {ESC_BYTE, D_BYTE, L_BYTE};
   tlcd_sendCommand(cmd, sizeof(cmd));
}
/*!
 *  Draw a String beginning at position (x1,y1).
 *  \param s text to draw
 *  \param x1 X coordinate
 *  \param x1 Y coordinate
 */
void tlcd_drawText(uint16_t x1, uint16_t y1, char* s) 
{
	size_t textLen = sizeof(s) / sizeof(char); 
	size_t totsize = textLen + 8;
    unsigned char cmd[totsize];
	
	cmd[0] = ESC_BYTE; cmd[1] = Z_BYTE; cmd[2] = L_BYTE; cmd[3] = LOW(x1); cmd[4] = HIGH(x1);
	cmd[5] = LOW(y1); cmd[6] = HIGH(y1);	
	
	for(uint8_t i = 0 ; i < textLen ; i++)
	{
		cmd[7 + i] = *(s + i);
	}	
	tlcd_sendCommand(cmd, sizeof(cmd));
}

