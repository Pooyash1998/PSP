#ifndef TLCD_BUTTON_H
#define TLCD_BUTTON_H

#include <stdint.h>

#include "tlcd_parser.h"
#include <stdbool.h>

#define MAX_BUTTONS 10

typedef struct{
	uint16_t x;
	uint16_t y;
}position;

typedef struct {
	position u_l_pos;
	position d_r_pos;
	uint8_t colorID;
	uint8_t btn_code;
	char label;
}Button;

typedef void (*ButtonCallback)(uint8_t downcode, position pos);




void tlcd_addButtonWithChar(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t colorID, uint8_t downCode, char c);

void tlcd_drawButtons();

uint8_t tlcd_handleButtons(TouchEvent event);

// Callback function
void tlcd_setButtonCallback(ButtonCallback *btnCallback);

#endif