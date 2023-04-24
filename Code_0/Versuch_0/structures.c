/*
 * structures.c
 *
 * Created: 22.04.2023 19:48:44
 *  Author: uk447157
 */ 
#include "structures.h"
#include "lcd.h"
#include "button.h"
#include <stdint.h>
#include <util/delay.h>

typedef enum {
	AVAILABLE,
	BACKORDER,
	SOLD_OUT = 99
}DistributionStatus;
typedef struct {
	uint8_t manufactureId;
	uint8_t productId;
}ArticleNumber;
typedef union {
uint16_t combinedNumber;
ArticleNumber singleNumbers;	
}FullArticleNumber; 
typedef struct {
	FullArticleNumber articleNumber;
	DistributionStatus satus;
}Article;

Article bsp[] = {
	{ {0x1101}, AVAILABLE },
	{ {0x1110}, BACKORDER },
	{ {0x0101}, SOLD_OUT }
};

void displayArticles()
{
	lcd_init();
	//Durchlaufen des Arrays
	for (int i = 0; i < sizeof(bsp) / sizeof(Article); i++) {
	lcd_clear();
	lcd_writeDec(i+1);
	lcd_writeString(".Article: ");
	lcd_writeDec(bsp[i].articleNumber.singleNumbers.manufactureId); //product_ID is actually in manufactureId 
	lcd_writeChar(' ');
	lcd_writeDec(bsp[i].articleNumber.singleNumbers.productId); //manufacture_ID is in productId
	lcd_line2();
	char* s;
	switch(bsp[i].satus){
		case AVAILABLE: s ="Available";
		break;
		case BACKORDER: s ="Back ordered";
		break;
		case SOLD_OUT:  s ="Sold out";
		break;
		default: s ="error";
	}
	lcd_writeString(s);
	waitForInput(); // waiting for the Buttonpress to go next
	_delay_ms(200); // small delay to make sure the button is released for the next run
	}
	lcd_clear();
	lcd_writeString("End");
}