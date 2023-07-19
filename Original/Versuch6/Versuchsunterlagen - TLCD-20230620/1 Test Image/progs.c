//-------------------------------------------------
//          TestTask: Test Image
//-------------------------------------------------

#include <avr/pgmspace.h>
#include <util/atomic.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdint.h>
#include <string.h>
#include "tlcd_core.h"
#include "lcd.h"
#include "util.h"
#include "os_input.h"
#include "os_core.h"
#include "tlcd_graphic.h"

#if VERSUCH < 6
    #warning "Please fix the VERSUCH-define"
#endif

//---- Adjust here what to test -------------------
#define CHUNK_SIZE 64
#define TOTAL_LEN_LEN 8
//-------------------------------------------------

#ifndef WRITE
    #define WRITE(str) lcd_writeProgString(PSTR(str))
#endif
#define TEST_PASSED \
    do { ATOMIC { \
        lcd_clear(); \
        WRITE("  TEST PASSED   "); \
    } } while (0)
#define TEST_FAILED(reason) \
    do { ATOMIC { \
        lcd_clear(); \
        WRITE("FAIL  "); \
        WRITE(reason); \
    } } while (0)
#ifndef CONFIRM_REQUIRED
    #define CONFIRM_REQUIRED 1
#endif

// BITMASK DEFINES
#define TLCD_DDR_BITMASK 0xFC
#define TLCD_PORT_BITMASK 0xFC

// ERROR DEFINES
#define DDR_PORT_SPCR_ERROR 0x01
#define RESET_ERROR 0x04
#define WRITE_ERROR 0x05
#define PATTERN_ERROR 0x06
#define READ_ERROR 0x07

// FORWARD DECLARATION
void defineColors();
void setColor(uint8_t colorIndex);
void setFontColor(uint8_t color);

void drawCircle(uint8_t rad, uint8_t x0, uint8_t y0);
void drawBar(uint16_t posX, uint16_t posY, uint16_t width, uint16_t height);
void drawGrid();

void writeString(unsigned char* str, uint8_t color, uint16_t x, uint16_t y);
void writeChar(unsigned char c, uint16_t x, uint16_t y);

void hash(uint8_t hash[32], const void *input, size_t len);

REGISTER_AUTOSTART(program1)
void program1(void) {
	uint8_t error = 0;
	lcd_clear();
    lcd_writeProgString(PSTR("Phase 1:"));
    delayMs(1000);

	tlcd_init();

	lcd_clear();
	lcd_writeProgString(PSTR("Checking... "));

	uint8_t plain[] = {0x50, 0x53, 0x50, 0x20, 0x69, 0x73, 0x20, 0x6c, 0x6f, 0x76, 0x65, TLCD_DDR & TLCD_DDR_BITMASK, TLCD_PORT & TLCD_PORT_BITMASK, SPCR, 0x50, 0x53, 0x50, 0x20, 0x69, 0x73, 0x20, 0x6c, 0x69, 0x66, 0x65};
	uint8_t expected[32] = {0x39, 0x03, 0xe6, 0xb3, 0xfc, 0xfa, 0x45, 0x92, 0xe5, 0x5a, 0xc2, 0xf9, 0x1b, 0xf2, 0x51, 0xba, 0x19, 0x0b, 0xc7, 0x4c, 0x09, 0x43, 0xc6, 0x30, 0x24, 0x3b, 0x41, 0xcf, 0x8b, 0x76, 0x66, 0x77};

	uint8_t result[32];
	uint8_t acc[32];
	memset((void *) result, 0, 32);
	memset((void *) acc, 0, 32);

	uint16_t hash_count = 501;
	hash(acc, (void*) plain, sizeof(plain)/sizeof(plain[0]));
	for(uint16_t i = 0; i < hash_count; i++) {
		// avoid memcpy
		if (i % 2) {
			hash(acc, (void*) result, 32);
		} else {
			hash(result, (void*) acc, 32);
		}

		// update progress at some rate
		if (i % (hash_count/20) == 0) {
			uint8_t percent = (i * 100) / hash_count;
			lcd_goto(1, 13);
			if (percent < 10)
				lcd_writeProgString(PSTR("0"));
			lcd_writeDec(percent);
			lcd_writeProgString(PSTR("%"));
		}
	}

	lcd_goto(1, 13);
	lcd_writeProgString(PSTR("100%"));

	lcd_line2();
	for(uint8_t i = 0; i < 32; i++) {
		if(result[i] != expected[i]) {
			os_error("DDR, PORT or SPCR wrong");
			error |= (1 << DDR_PORT_SPCR_ERROR);
			break;
		}
	}

	lcd_writeProgString(PSTR("OK"));
	delayMs(2000);

    // Phase 2
    lcd_clear();
    lcd_writeProgString(PSTR("Phase 2:"));
    delayMs(1000);

    lcd_clear();
    lcd_writeProgString(PSTR("CHECKING BUFFER: "));
    lcd_line2();

    uint8_t i = 0;
    uint8_t l = 100;
    tlcd_writeNextBufferElement(i);
    tlcd_resetBuffer();
    if (tlcd_hasNextBufferElement()) {
        os_error("Non-Empty Buffer after reset");
        error |= (1 << RESET_ERROR);
    }
    for (i = 0; i <= l; i++) {
        tlcd_writeNextBufferElement(i);
        if (!tlcd_hasNextBufferElement()) {
            os_error("Cannot write E");
            error |= (1 << WRITE_ERROR);
        }
    }
    for (i = 0; i <= l; i++) {
        if (i != tlcd_readNextBufferElement()) {
            os_error("Pattern Mismatch");
            error |= (1 << PATTERN_ERROR);
        }
    }
    if (tlcd_hasNextBufferElement()) {
        os_error("Non-Empty Buffer after read");
        error |= (1 << READ_ERROR);
    }

    lcd_writeProgString(PSTR("OK"));
    delayMs(2000);


    // Phase 3
    lcd_clear();
    lcd_writeProgString(PSTR("Phase 3:"));
    lcd_line2();
    delayMs(1000);

    // Define touch area (whole tlcd)
    tlcd_defineTouchArea(0, 0, 480, 272);

    // Define colors
    defineColors();

    // Draw grid (30 on x and 17 on y each cell of size 16x16)
    drawGrid();

    // Draw a circle of color grey (1)
    setColor(1);
    drawCircle(128, 480 / 2, 272 / 2);

    // Draw rectangular color bar
    uint16_t offset = 0;
    uint8_t x;
    for (x = 1; x <= 8; x++) {
        setColor(x);
        drawBar(128 + offset, 32, 28, 64);
        offset = 28 * x;
    }

    // Draw black,darkgray,gray,white color bars
    setColor(8);
    drawBar(128, 96, 56, 48);
    setColor(1);
    drawBar(128 + 56, 96, 56, 48);
    setColor(9);
    drawBar(128 + 2 * 56, 96, 56, 48);
    setColor(10);
    drawBar(128 + 3 * 56, 96, 56, 48);

    // Draw white, black, white
    setColor(10);
    drawBar(128, 144, 56, 24);
    setColor(8);
    drawBar(128 + 56, 144, 112, 24);
    setColor(10);
    drawBar(128 + 3 * 56, 144, 56, 24);

    // White, black, PSP, orange, grey
    setColor(10);
    drawBar(128, 168, 56, 24);
    setColor(8);
    drawBar(128 + 56, 168, 112, 24);
    uint16_t xCor = 128 + 100;
    uint16_t yCor = 160;
    setFontColor(6);
    unsigned char psp[] = {0x1B, 0x5A, 0x4C, LOW(xCor), HIGH(xCor), LOW(yCor), HIGH(yCor), 0x50, 0x53, 0x50, 0x00};
    tlcd_sendCommand(psp, sizeof(psp) / sizeof(psp[0]));
    setColor(12);
    drawBar(128 + 3 * 56, 168, 44, 24);
    setColor(1);
    drawBar(128 + 3 * 56 + 56 - 12, 168, 12, 24);

    // White
    setColor(10);
    drawBar(128, 192, 224, 16);

    // Magenta and bluegrey
    setColor(11);
    drawBar(128, 208, 168, 16);
    setColor(12);
    drawBar(128, 224, 168, 16);
    setColor(1);
    drawBar(128 + 3 * 56, 208, 56, 32);

    // End
    lcd_clear();
    if (error & (DDR_PORT_SPCR_ERROR | RESET_ERROR | WRITE_ERROR | PATTERN_ERROR | READ_ERROR)) {
        ATOMIC {
            TEST_FAILED("Errors");
            HALT;
        }
    }

    // SUCCESS
    #if CONFIRM_REQUIRED
    lcd_clear();
    lcd_writeProgString(PSTR("  PRESS ENTER!  "));
    os_waitForInput();
    os_waitForNoInput();
    #endif
    TEST_PASSED;
    lcd_line2();
    lcd_writeProgString(PSTR(" WAIT FOR IDLE  "));
    delayMs(DEFAULT_OUTPUT_DELAY * 6);
}

void drawCircle(uint8_t rad, uint8_t x0, uint8_t y0) {
    int x = rad;
    int y = 0;
    // Decision criterion divided by 2 evaluated at x=r, y=0
    int decisionOver2 = 1 - x;

    while (y <= x) {
        tlcd_drawPoint(x + x0,  y + y0);  // Octant 1
        tlcd_drawPoint(y + x0,  x + y0);  // Octant 2
        tlcd_drawPoint(-x + x0,  y + y0); // Octant 4
        tlcd_drawPoint(-y + x0,  x + y0); // Octant 3
        tlcd_drawPoint(-x + x0, -y + y0); // Octant 5
        tlcd_drawPoint(-y + x0, -x + y0); // Octant 6
        tlcd_drawPoint(x + x0, -y + y0);  // Octant 7
        tlcd_drawPoint(y + x0, -x + y0);  // Octant 8
        y++;
        if (decisionOver2 <= 0) {
            // Change in decision criterion for y -> y+1
            decisionOver2 += 2 * y + 1;
        } else {
            x--;
            // Change for y -> y+1, x -> x-1
            decisionOver2 += 2 * (y - x) + 1;
        }
    }
}

void defineColors() {
    /*
     * Color:
     * grey (1), yellow (2), lightblue (3), green (4), purple (5),
     * red (6), blue (7), black (8), darkgrey (9), white (10),
     * magenta (11), blue-grey (12), orange (13)
     */
    unsigned char grey[] = {0x1B, 0x46, 0x50, 1, 175, 175, 175};
    tlcd_sendCommand(grey, 7);

    unsigned char yellow[] = {0x1B, 0x46, 0x50, 2, 255, 255, 0};
    tlcd_sendCommand(yellow, 7);

    unsigned char lightblue[] = {0x1B, 0x46, 0x50, 3, 0, 255, 255};
    tlcd_sendCommand(lightblue, 7);

    unsigned char green[] = {0x1B, 0x46, 0x50, 4, 0, 255, 0};
    tlcd_sendCommand(green, 7);

    unsigned char purple[] = {0x1B, 0x46, 0x50, 5, 255, 0, 255};
    tlcd_sendCommand(purple, 7);

    unsigned char red[] = {0x1B, 0x46, 0x50, 6, 255, 0, 0};
    tlcd_sendCommand(red, 7);

    unsigned char blue[] = {0x1B, 0x46, 0x50, 7, 0, 0, 255};
    tlcd_sendCommand(blue, 7);

    unsigned char black[] = {0x1B, 0x46, 0x50, 8, 0, 0, 0};
    tlcd_sendCommand(black, 7);

    unsigned char darkgrey[] = {0x1B, 0x46, 0x50, 9, 111, 111, 111};
    tlcd_sendCommand(darkgrey, 7);

    unsigned char white[] = {0x1B, 0x46, 0x50, 10, 255, 255, 255};
    tlcd_sendCommand(white, 7);

    unsigned char magenta[] = {0x1B, 0x46, 0x50, 11, 255, 0, 143};
    tlcd_sendCommand(magenta, 7);

    unsigned char bluegrey[] = {0x1B, 0x46, 0x50, 12, 0, 143, 255};
    tlcd_sendCommand(bluegrey, 7);

    unsigned char orange[] = {0x1B, 0x46, 0x50, 13, 255, 143, 0};
    tlcd_sendCommand(orange, 7);
}

void setColor(uint8_t colorIndex) {
    const unsigned char cmd[] = {0x1B, 0x46, 0x47, colorIndex, 255};
    tlcd_sendCommand(cmd, sizeof(cmd) / sizeof(cmd[0]));
}

void drawBar(uint16_t posX, uint16_t posY, uint16_t width, uint16_t height) {
    uint16_t i;
    for (i = 0; i < width; i++) {
        tlcd_drawLine(posX + i, posY, posX + i, posY + height);
    }
}

void drawGrid() {
    setColor(1);
    uint16_t x, y;
    for (x = 0; x <= 30; x++) {
        tlcd_drawLine(x * 16, 0, x * 16, 272);
    }

    for (y = 0; y <= 17; y++) {
        tlcd_drawLine(0, y * 16, 482, y * 16);
    }
}

void writeString(unsigned char* str, uint8_t color, uint16_t x, uint16_t y) {
    setFontColor(color);
    size_t len = strlen((const char*)str);
    for (uint8_t i = 0; i < len; i++) {
        writeChar(*(str + i), x, y);
        x += 10;
    }
}

void writeChar(unsigned char c, uint16_t x, uint16_t y) {
    unsigned char cmd[] = {0x1B, 0x5A, 0x4C, LOW(x), HIGH(x), LOW(y), HIGH(y), c, 0x00};
    tlcd_sendCommand(cmd, sizeof(cmd) / sizeof(cmd[0]));
}

void setFontColor(uint8_t color) {
    unsigned char fontColor[] = {0x1B, 0x46, 0x5A, color, 255};
    tlcd_sendCommand(fontColor, 5);
}

static const uint32_t k[] = {
	0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
	0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
	0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
	0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
	0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
	0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
	0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
	0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

struct buffer_state {
	const uint8_t * p;
	size_t len;
	size_t total_len;
	int single_one_delivered;
	int total_len_delivered;
};

static inline uint32_t right_rot(uint32_t value, unsigned int count) {
	return value >> count | value << (32 - count);
}

static void init_buf_state(struct buffer_state * state, const void * input, size_t len) {
	state->p = input;
	state->len = len;
	state->total_len = len;
	state->single_one_delivered = 0;
	state->total_len_delivered = 0;
}

static int calc_chunk(uint8_t chunk[CHUNK_SIZE], struct buffer_state * state) {
	size_t space_in_chunk;

	if (state->total_len_delivered) {
		return 0;
	}

	if (state->len >= CHUNK_SIZE) {
		memcpy(chunk, state->p, CHUNK_SIZE);
		state->p += CHUNK_SIZE;
		state->len -= CHUNK_SIZE;
		return 1;
	}

	memcpy(chunk, state->p, state->len);
	chunk += state->len;
	space_in_chunk = CHUNK_SIZE - state->len;
	state->p += state->len;
	state->len = 0;

	if (!state->single_one_delivered) {
		*chunk++ = 0x80;
		space_in_chunk -= 1;
		state->single_one_delivered = 1;
	}

	if (space_in_chunk >= TOTAL_LEN_LEN) {
		const size_t left = space_in_chunk - TOTAL_LEN_LEN;
		size_t len = state->total_len;
		int i;
		memset(chunk, 0x00, left);
		chunk += left;

		chunk[7] = (uint8_t) (len << 3);
		len >>= 5;
		for (i = 6; i >= 0; i--) {
			chunk[i] = (uint8_t) len;
			len >>= 8;
		}
		state->total_len_delivered = 1;
	} else {
		memset(chunk, 0x00, space_in_chunk);
	}

	return 1;
}

void hash(uint8_t hash[32], const void * input, size_t len) {
	uint32_t h[] = { 0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19 };
	int i, j;

	uint8_t chunk[64];

	struct buffer_state state;

	init_buf_state(&state, input, len);

	while (calc_chunk(chunk, &state)) {
		uint32_t ah[8];

		uint32_t w[64];
		const uint8_t *p = chunk;

		memset(w, 0x00, sizeof w);
		for (i = 0; i < 16; i++) {
			w[i] = (uint32_t) p[0] << 24 | (uint32_t) p[1] << 16 |
				(uint32_t) p[2] << 8 | (uint32_t) p[3];
			p += 4;
		}

		for (i = 16; i < 64; i++) {
			const uint32_t s0 = right_rot(w[i - 15], 7) ^ right_rot(w[i - 15], 18) ^ (w[i - 15] >> 3);
			const uint32_t s1 = right_rot(w[i - 2], 17) ^ right_rot(w[i - 2], 19) ^ (w[i - 2] >> 10);
			w[i] = w[i - 16] + s0 + w[i - 7] + s1;
		}

		for (i = 0; i < 8; i++)
			ah[i] = h[i];

		for (i = 0; i < 64; i++) {
			const uint32_t s1 = right_rot(ah[4], 6) ^ right_rot(ah[4], 11) ^ right_rot(ah[4], 25);
			const uint32_t ch = (ah[4] & ah[5]) ^ (~ah[4] & ah[6]);
			const uint32_t temp1 = ah[7] + s1 + ch + k[i] + w[i];
			const uint32_t s0 = right_rot(ah[0], 2) ^ right_rot(ah[0], 13) ^ right_rot(ah[0], 22);
			const uint32_t maj = (ah[0] & ah[1]) ^ (ah[0] & ah[2]) ^ (ah[1] & ah[2]);
			const uint32_t temp2 = s0 + maj;

			ah[7] = ah[6];
			ah[6] = ah[5];
			ah[5] = ah[4];
			ah[4] = ah[3] + temp1;
			ah[3] = ah[2];
			ah[2] = ah[1];
			ah[1] = ah[0];
			ah[0] = temp1 + temp2;
		}

		for (i = 0; i < 8; i++)
			h[i] += ah[i];
	}

	for (i = 0, j = 0; i < 8; i++)
	{
		hash[j++] = (uint8_t) (h[i] >> 24);
		hash[j++] = (uint8_t) (h[i] >> 16);
		hash[j++] = (uint8_t) (h[i] >> 8);
		hash[j++] = (uint8_t) h[i];
	}
}
