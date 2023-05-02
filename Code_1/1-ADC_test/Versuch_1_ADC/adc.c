#include "adc.h"
#include <avr/io.h>
#include <stdlib.h>
#include <util/delay.h>
#include "lcd.h"

//! Global variables
uint16_t lastCaptured;

uint16_t* bufferStart;
uint8_t bufferSize = 0;
uint8_t bufferIndex = 0;

/*! \brief This method initializes the necessary registers for using the ADC module. \n
 * Reference voltage:    internal \n
 * Input channel:        PORTA0 \n
 * Conversion frequency: 156kHz
 */
void initAdc(void) {
    // Init DDRA0 as input
    DDRA &= ~(1 << PA0);

    /* 
     * REFS1:0 = 01     Select internal reference voltage
     * ADLAR   = 0      Store the result right adjusted to the ADC register
     * MUX4:0  = 00000  Use ADC0 as input channel (PA0)
     */
    ADMUX = (0 << REFS1) | (1 << REFS0);

    /*
     * ADEN    = 1      Enable ADC
     * ADSC    = 0      Used to start a conversion
     * ADATE   = 0      No continuous conversion
     * ADIF    = 0      Indicates that the conversion has finished
     * ADIE    = 0      Do not use interrupts
     * ADPS2:0 = 111    Prescaler 128 -> 20MHz / 128 = 156kHz ADC frequency
     */
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

/*! \brief Executes one conversion of the ADC and returns its value.
 *
 * \return The converted voltage (0 = 0V, 1023 = AVCC)
 */
uint16_t getAdcValue() {
	 //ersetze mit diesem Code fuer Praesenzversuch 
	//16 Werte * 1023 lang = bracuht max 15 bits 
	uint16_t capSumme = 0;
	for(uint8_t i = 1; i <= 16; i++)
	{
		// Start the conversion
    ADCSRA |= (1 << ADSC);

    // Wait until the conversion has finished
    while (ADCSRA & (1 << ADSC)) {};

    // Store the value as last captured
    capSumme += ( ADCL | ((uint16_t)ADCH << 8) );
	}
	lastCaptured = capSumme / 16;
	return lastCaptured;
	
	/*
    // Start the conversion
    ADCSRA |= (1 << ADSC);

    // Wait until the conversion has finished
    while (ADCSRA & (1 << ADSC)) {};

    // Store the value as last captured
    lastCaptured = ADCL | ((uint16_t)ADCH << 8);
	
    // Return the result
    return lastCaptured;
	*/
}

/*! \brief Returns the size of the buffer which stores voltage values.
 *
 * \return The size of the buffer which stores voltage values.
 */
uint8_t getBufferSize() {
   return bufferSize;
}

/*! \brief Returns the current index of the buffer which stores voltage values.
 *
 * \return The current index of the buffer which stores voltage values.
 */
uint8_t getBufferIndex() {
	return bufferIndex;
}

/*! \brief Stores the last captured voltage.
 *
 */
void storeVoltage(void) {
	
    if(bufferSize == 0) //check if Database not implemented yet
	{
		bufferSize = 100 ; //Platz fuer 100 Werte
		bufferStart = malloc(bufferSize * sizeof(uint16_t)); 
	}
	if(bufferIndex < bufferSize)
	{
		*(bufferStart + bufferIndex) = lastCaptured;
	}
	/*Wertebereich von Index soll von 0 bis 100 sein! 
	 puffer hat ebenfalls Speicherstellen von 0 - 99 (size 100)
	 index zeigt immer die naechste freie Stelle
	 An der letzten Stelle weiss man von Aussen nicht ob das block belegt ist oder nicht
	 daher 1 bis 100 und nicht 1 bis (size - 1) wie im Dokument */
	if(bufferIndex < bufferSize )
		bufferIndex ++;
	
}

/*! \brief Returns the voltage value with the passed index.
 *
 * \param ind   Index of the voltage value.
 * \return      The voltage value with index ind.
 */
uint16_t getStoredVoltage(uint8_t ind) {
	if(ind < bufferIndex)
	 return *(bufferStart + ind);
	else
	 return 0;
}
