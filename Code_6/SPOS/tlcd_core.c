#include <util/delay.h>
#include <stdbool.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include <util/atomic.h>

#include "tlcd_core.h"
#include "tlcd_parser.h"
#include "util.h"
#include "os_core.h"

tlcdBuffer intBuff ;

/*!
 *  This function configures all relevant ports,
 *  initializes the pin change interrupt, the spi
 *  module and the input buffer
 */
void tlcd_init() {
    
	//initialize Ports MOSI(PB5), SCK(PB7), /CS(PB4), RESET(PB3) as output and others remain input
    DDRB |= (1 << 3) | (1 << 5) | (1 << 7) | (1 << 4) ;
	// make sure BUF_Ind(PB2) is input, MISO(PB6) is by default always input in Atmgea as Master mode
	DDRB &= ~(1 << 2);
	// Pull-up Resistors for SBUF(PB2)
	PORTB |= (1 << 2) | (1 << 6);
	// Deselect chip
	tlcd_spi_disable()
	/* enable SPI as Master and set the config Register bits
	   SPE(6) = 1 ; MSTR(4) = 1; 
	   DORD(5), CPOL(3), CPHA(2) of TLCDs are all 0 but they're active low 
	   so we set them to 1.  
	   Setting the Configurations for TLCD (max 200kHz) in Control Register
	   SPR0(0) = 1 ; SPR1(1) = 1 ; SPI2X(0 in status reg) = 0  
	   ---------resulting in SPI-Freq = f_cpu/128 ~= 15kHz------------- 
	   DORD(5) = 1 (MSB first)
	   CPOL(3) = 1 (neutral Clock low)
	   CPHA(2) = 1 (Leading Edge)
	*/
	SPCR = (1 << 6 ) | (1 << 4) | (1 << 1) | (1 << 0) | (1 << 5) | (1 << 3) | (1 << 2);
	
    // Pin change interrupt on PORTB
    PCICR = (1 << TLCD_SEND_BUFFER_IND_INT_MSK_PORT);
    PCMSK1 = (1 << TLCD_SEND_BUFFER_IND_INT_MSK_PIN);

    // Initial reset
    tlcd_reset();

    // Initialize all buffers
    tlcd_initBuffer();
}

/*!
 *  This function resets the lcd by toggling the reset pin.
 */
void tlcd_reset() {
    PORTB &= ~(1 << TLCD_RESET_BIT);
	delayMs(10);
	PORTB |= (1 << TLCD_RESET_BIT);
}

/*!
 *  This function writes a byte to the lcd via SPI
 *  \param byte The byte to be send to the lcd
 */
uint8_t tlcd_writeByte(uint8_t byte) {
	os_enterCriticalSection();
	
	uint8_t buff;
	tlcd_spi_enable()
	_delay_us(6);
	// Start transmission
	SPDR = byte;
	// Wait for transmission complete
	while(!(SPSR & (1<<SPIF)));
	//received data from Slave
	buff = SPDR;
	tlcd_spi_disable()
	os_leaveCriticalSection();
    return buff;
}

/*!
 *  This function waits for a byte to arrive from the lcd and returns it
 *  \returns a byte read from the lcd
 */
uint8_t tlcd_readByte() {
	os_enterCriticalSection();
	uint8_t ret;
	tlcd_spi_enable()
	_delay_us(6);
	// Start transmission
	SPDR = 0xFF;
	while(!(SPSR & (1<<SPIF)));
	//received data from Slave
	ret = SPDR;
	tlcd_spi_disable()
	os_leaveCriticalSection();
	return(ret);
}

/*!
 *  This function initializes input buffer
 *  i.e. allocate memory on the internal
 *  heap and initialize head & tail
 */
void tlcd_initBuffer() {
	MemAddr m = os_malloc(intHeap, 256);
    if(m == 0 )
		os_error("coulnt init buffer!");
    intBuff.data = m;
	intBuff.head = 0;
	intBuff.tail = 0;
	intBuff.count = 0;
	intBuff.size = 256;
}

/*!
 *  This function resets the input buffer by setting head & tail to zero
 */
void tlcd_resetBuffer() {
    intBuff.count = 0;
	intBuff.head = 0;
	intBuff.tail = 0;
}

/*!
 *  Writes a given byte into the input buffer and increments the position of head
 *  \param dataByte The byte that is to be stored into the buffer
 */
void tlcd_writeNextBufferElement(uint8_t dataByte) {
	if(intBuff.count == intBuff.size)
		os_error("buff overflow!");
	else
	{
		MemAddr addr = intBuff.data + intBuff.head ;
		intHeap->driver->write(addr, dataByte);
		intBuff.head ++ ;
		if(intBuff.head == intBuff.size)
			intBuff.head = 0;
		intBuff.count ++;	
	}
}

/*!
 *  This function reads a byte from the input buffer and increments the position of tail
 */
uint8_t tlcd_readNextBufferElement() {
    uint8_t data = 0;
	if(intBuff.count == 0)
		os_error("cant read empty Buffer!");
	else
	{
		MemAddr add = intBuff.data + intBuff.tail ;
		data = intHeap->driver->read(add);
		intBuff.tail ++;
		if(intBuff.tail == intBuff.size)
			intBuff.tail = 0;
		intBuff.count --;
	}
    return data;
}

/*!
 *  Checks if there are unprocessed data
 */
uint8_t tlcd_hasNextBufferElement() {
    return intBuff.count;
}

/*!
 *  This function requests the sending buffer
 *  from the tlcd. Should only be called, if SBUF is low.
 */
void tlcd_requestData() {
	//#Problem Source remove if 
    if((PINB & (1 << TLCD_SEND_BUFFER_IND_BIT)) != 0)
		os_error("TLCD-BUFF not ready yet");
	else
	{
		uint8_t bcc = (DC2_BYTE + 1 + S_BYTE) % 256; //Sum of the values of Start,Lenbyte,<S> byte
		uint8_t resp;
		do{
			
		tlcd_writeByte(DC2_BYTE);
		tlcd_writeByte(1);
		tlcd_writeByte(S_BYTE);
		tlcd_writeByte(bcc);
		resp = tlcd_readByte();
		}while(resp != ACK);
	}	
}

/*!
 *  This function reads the complete content of the TLCD sending buffer into the
 *  local input buffer. After a complete frame has been received, the bcc is checked.
 *  In case of a checksum error, the package is ignored by resetting the input buffer
 */
void tlcd_readData() {
	uint8_t checksum = 0;   //possible overflow of checksum (where to put modulo?) 
	uint8_t cache, len;
	// receive a complete frame
	cache = tlcd_readByte();
	checksum = (checksum + cache) % 256;
	// wrong <StartByte> 
	if(cache != DC1_BYTE)
		os_error("wrong startbyte came");
	len = tlcd_readByte();
	checksum = (checksum + len) % 256;
	// now comes the Usedata 
	while(len)
	{
		cache = tlcd_readByte();
		tlcd_writeNextBufferElement(cache);
		checksum = (checksum + cache) % 256;
		len --;
	}
	// receive the bcc and compare with checksum
	cache = tlcd_readByte();
	if(checksum != cache)
	{
		tlcd_resetBuffer();
		os_error("false checksum!");
	}
}

/*!
 *  This function sends a command with a given length
 *  \param cmd The cmd to be buffered as a string
 *  \param len The length of the command
 */
void tlcd_sendCommand(const unsigned char* cmd, uint8_t len) {
	// each unsigned char is 1 Byte 
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		uint8_t resp ;
		do {
			uint8_t bcc = (DC1_BYTE + len) % 256;
			tlcd_writeByte(DC1_BYTE);
			tlcd_writeByte(len);
			for(uint8_t i = 0; i < len; i++)
			{
				tlcd_writeByte((uint8_t)*(cmd+i));
				bcc = (bcc + *(cmd + i)) % 256;
			}
			tlcd_writeByte(bcc);
			resp = tlcd_readByte();
		}while(resp != ACK);
	}
}