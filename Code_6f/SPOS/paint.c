/*
 * paint.c
 *
 * Created: 6/22/2023 3:38:48 PM
 *  Author: Pooya
 */ 
#include "paint.h"
#include "tlcd_core.h"
#include "tlcd_graphic.h"
#include "tlcd_button.h"
#include "defines.h"
#include <util/atomic.h>

//Forward Declaration
uint8_t calculate_area(uint16_t x, uint16_t y);
uint8_t areaTocolor_red(uint16_t y, uint8_t area);
uint8_t areaTocolor_green(uint16_t y, uint8_t area);
uint8_t areaTocolor_blue(uint16_t y, uint8_t area);
void touchhandle(TouchEvent t_ev);
void btnhandle(uint8_t downcode, position pos);
void drawPallet();
//------------------------------------------------
// Global Vars
position a;
bool draged = false;
uint8_t penThickness = 1;
Color picked_col;
//------------------------------------------------
//------------------------------------------------

// CALLback Function for Touch Events
void touchhandle(TouchEvent t_ev)
{	
	uint8_t border = penThickness/2;
	if(t_ev.x > 440 - penThickness)
		t_ev.x = 440 -border;
	if(t_ev.y > 222 - penThickness)
		t_ev.y = 222 -border;	
	if(draged == true)
	{
		tlcd_drawLine(a.x,a.y,t_ev.x,t_ev.y);
		a.x = t_ev.x;
		a.y = t_ev.y;
	}
	if(t_ev.type == 2)
	{
		a.x = t_ev.x;
		a.y = t_ev.y;
		draged = true;
	}
	else
	{
		draged=false;
		if(t_ev.type == 0)
			tlcd_drawPoint(t_ev.x,t_ev.y);
	}
}
// CALLback Function for Buttons
void btnhandle(uint8_t downcode, position pos)
{
	draged = false;
	if(downcode == SIZE_UP && penThickness < 15)
	{
		tlcd_changePenSize(++penThickness);
	}
	else if(downcode == SIZE_DOWN && penThickness!=1)
	{
		tlcd_changePenSize(--penThickness);
	}
	else if(downcode == ERASE)
	{
		tlcd_clearDisplay();
		tlcd_drawButtons();
		//Draw Color Pallet (call always after the draw_btn)
		drawPallet();
		
	}
	// Color Pallet : transform position to color
	else if(downcode == COL_PALLET)
	{
		uint8_t area = calculate_area(pos.x,pos.y);
		//RED
		picked_col.red = areaTocolor_red(pos.y, area);
		//GREEN
		picked_col.green = areaTocolor_green(pos.y, area);
		//BULE
		picked_col.blue = areaTocolor_blue(pos.y, area);
		
		tlcd_defineColor(22,picked_col);
		//change the Pen Color
		tlcd_changeDrawColor(22);
		
	}
	
}
/*
it calculates the area in Pallet 
there are 6 Areas each 80px Width and a fix 50px height
the first Pallet point (up_l) is 0,222 and the second(d_r) is 480,222
the Coordinates of Areas(boxes) are:
A1 : (0,222) - (79,272)
A2 : (80,222) - (159,272)
A3:  (160,222) - (239,272)
A4:  (240,222) - (319,272)
A5:  (320,222) - (399,272)
A6:  (400,222) - (480,272)
more info on Spectrum in Document!
*/
uint8_t calculate_area(uint16_t x, uint16_t y)
{
	uint8_t area = 0;
	if((222 <= y && y <= 272)) //actually no need for that already checked in btn_handle
	{
		if(0 <= x && x <= 79)
			area = 1;
		else if(80 <= x && x <= 159)
			area = 2;
		else if(160 <= x && x <= 239)
			area = 3;
		else if(240 <= x && x <= 319)
			area = 4;
		else if(320 <= x && x <= 399)
			area = 5;
		else if(400 <= x && x <= 480)
			area = 6;
	}
	return area;
	
}
/* Linear Transformation of y-coordinates to a Red value depending on the area

*/
uint8_t areaTocolor_red(uint16_t x, uint8_t area)
{
	uint8_t r = 0;
	if(area == 1 || area == 6)
		r = 255;
	else if(area == 3 || area == 4)
		r = 0;
	else if(area == 2)
	{
		r = ((255/80)*160) - ((255/80)*x);
	}
	else if(area == 5)
	{
		r = ((255/80)*x) - ((255/80)*320);
	}
	return r;
}
/* Linear Transformation of y-coordinates to a 
green value depending on the area
*/
uint8_t areaTocolor_green(uint16_t x, uint8_t area)
{
	uint8_t gr = 0;
	if(area == 5 || area == 6)
		gr = 0;
	else if(area == 2 || area == 3)
		gr = 255;
	else if(area == 1)
	{
		gr = (255/80)*x;
	}
	else if(area == 4)
	{
		gr = ((255/80)*320) - ((255/80)*x);
	}
	return gr;
}
/* Linear Transformation of y-coordinates
 to a Blue value depending on the area
*/
uint8_t areaTocolor_blue(uint16_t x, uint8_t area)
{
	uint8_t b = 0;
	if(area == 1 || area == 2)
		b = 0;
	else if(area == 4 || area == 5)
		b = 255;
	else if(area == 3)
	{
		b = ((255/80)*x) - ((255/80)*160);
	}
	else if(area == 6)
	{
		b = ((255/80)*480) - ((255/80)*x);
	}
	return b;
}

void drawPallet()
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		uint8_t area = 0;
		Color c;
		for(uint16_t x = 1; x < 480 ; x++)
		{
			area = calculate_area(x,250);
			c.red = areaTocolor_red(x,area);
			c.green = areaTocolor_green(x,area);
			c.blue = areaTocolor_blue(x,area);
		
			tlcd_defineColor(20,c);
			tlcd_changeDrawColor(20);
			tlcd_drawLine(x,222,x,272); //#problem make it 271
		}
		tlcd_changeDrawColor(2);
	}
}
void paint()
{
	tlcd_init();
	// Define touch area (whole tlcd)
	tlcd_defineTouchArea(0, 0, 480, 272);
	EventCallback th = touchhandle;
	tlcd_setEventCallback(&th);
	ButtonCallback bh = btnhandle;
	tlcd_setButtonCallback(&bh);
	Color gray = {.red = 100 , .green= 100, .blue=100};
	Color noColor = {.red =0, .green =0, .blue=0};	
	tlcd_defineColor(30,gray);	
	tlcd_defineColor(31,noColor);
	//btns for +,-,and eraser
	tlcd_addButtonWithChar(440,10,480,50,30,SIZE_UP,'+');
	tlcd_addButtonWithChar(440,51,480,90,30,SIZE_DOWN,'-');
	tlcd_addButtonWithChar(440,91,480,130,30,ERASE,'X');
	//btn for Pallet
	tlcd_addButtonWithChar(0,222,480,272,31,COL_PALLET,0);
	tlcd_drawButtons();
	//Draw Color Pallet (call always after the draw_btn)
	drawPallet();
	tlcd_setCursor(0);
	while (1);
}