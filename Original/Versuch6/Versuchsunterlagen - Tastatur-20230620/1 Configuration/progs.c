//-------------------------------------------------
//          TestTask: Configuration
//-------------------------------------------------

#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/atomic.h>
#include <avr/io.h>
#include <stdint.h>
#include <string.h>
#include "lcd.h"
#include "os_input.h"
#include "os_scheduler.h"
#include "os_core.h"
#include "keyb_usart.h"

#if VERSUCH < 6
    #warning "Please fix the VERSUCH-define"
#endif

//---- Adjust here what to test -------------------
#define TEST_DEFINES 1
#define TEST_USARTREGS 1
//----------------------------------------------------------------------------------

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

#if !(TEST_DEFINES || TEST_USARTREGS)
#warning "Please select a test"
#endif

#define USARTREGMASK_A 0x01
#define USARTREGMASK_B 0b10010100
#define USARTREGMASK_C 0xFF

#define CHUNK_SIZE 64
#define TOTAL_LEN_LEN 8

#define ghb(expr) ((uint8_t) ((uint16_t)expr >> 8))
#define glb(expr) ((uint8_t) ((uint16_t)expr))

typedef uint8_t Hash[32];

bool compare(const Hash expected, const uint8_t plain[], size_t plain_size);
void hash(Hash hash, const void *input, size_t len);

REGISTER_AUTOSTART(program1)
void program1(void) {
    bool errorDefines = false, errorUSARTRegs = false;

    #if TEST_DEFINES
    {
        lcd_writeProgString(PSTR("Defines"));

        const uint8_t plain[] = {
            ghb(&KEYB_DATA_PORT ), glb(&KEYB_DATA_PORT ), ghb(&KEYB_DATA_DDR ), glb(&KEYB_DATA_DDR ), ghb(&KEYB_DATA_PIN ), glb(&KEYB_DATA_PIN ),
            ghb(&KEYB_CLOCK_PORT), glb(&KEYB_CLOCK_PORT), ghb(&KEYB_CLOCK_DDR), glb(&KEYB_CLOCK_DDR), ghb(&KEYB_CLOCK_PIN), glb(&KEYB_CLOCK_PIN),
            ghb(&KEYB_POWER_PORT), glb(&KEYB_POWER_PORT), ghb(&KEYB_POWER_DDR), glb(&KEYB_POWER_DDR), ghb(&KEYB_POWER_PIN), glb(&KEYB_POWER_PIN),
            KEYB_DATA_BIT, KEYB_CLOCK_BIT
        };

        static const Hash expected = {0xa3, 0x0e, 0x72, 0x22, 0x95, 0xa8, 0x26, 0xab, 0xdc, 0x94, 0x09, 0x58, 0x55, 0x9f, 0x9e, 0xd1, 0xe4, 0x5b, 0xbc, 0x9c, 0x8b, 0x63, 0x69, 0xa2, 0xd2, 0x7d, 0xdf, 0x58, 0x25, 0x4a, 0xdb, 0x31};

        errorDefines |= !compare(expected, plain, sizeof(plain) / sizeof(plain[0]));
    }
    #endif

    #if TEST_USARTREGS
    {
        lcd_writeProgString(PSTR("USART Registers"));

        keyb_usart_init();

        const uint8_t plain[] = {
            0x50, 0x53, 0x50, 0x20, 0x69, 0x73, 0x20, 0x6c, 0x6f, 0x76, 0x65,
            (UCSR0A & USARTREGMASK_A), (UCSR0B & USARTREGMASK_B), (UCSR0C & USARTREGMASK_C), (UCSR0B & USARTREGMASK_B), (UCSR0A & USARTREGMASK_A),
            0x50, 0x53, 0x50, 0x20, 0x69, 0x73, 0x20, 0x6c, 0x69, 0x66, 0x65
        };

        static const Hash expected = {0xf0, 0xcf, 0xaf, 0x42, 0xdb, 0x9b, 0xc5, 0x37, 0xf5, 0xbc, 0x7a, 0x95, 0xd8, 0x6c, 0x8f, 0x6a, 0x3b, 0xd7, 0x96, 0xe5, 0x76, 0xb3, 0x67, 0xf8, 0xcb, 0xc7, 0x76, 0xff, 0x84, 0x92, 0x8c, 0xc6};

        errorUSARTRegs |= !compare(expected, plain, sizeof(plain) / sizeof(plain[0]));
    }
    #endif

    if (errorDefines || errorUSARTRegs) {
        if (!errorUSARTRegs) {
            TEST_FAILED("Defines");
        } else if (!errorDefines) {
            TEST_FAILED("           USART Registers");
        } else {
            TEST_FAILED("Defines,   USART Registers");
        }
        HALT;
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

bool compare(const Hash expected, const uint8_t plain[], size_t plain_size) {
	Hash result = {0};
	Hash acc = {0};

	uint16_t hash_count = 501;
	hash(acc, (void*) plain, plain_size);
	for (uint16_t i = 0; i < hash_count; i++) {
		// avoid memcpy
		if (i % 2) {
			hash(acc, (void*) result, sizeof(Hash));
		} else {
			hash(result, (void*) acc, sizeof(Hash));
		}

		// update progress at some rate
		if (i % (hash_count / 30) == 0) {
			uint8_t percent = (i * 100) / hash_count;
			lcd_goto(2, 2);
			if (percent < 10) lcd_writeProgString(PSTR(" "));
			lcd_writeDec(percent);
			lcd_writeProgString(PSTR("%"));
		}
	}

	lcd_goto(2, 1);
	lcd_writeProgString(PSTR("100%"));

    bool error = false;
	for (uint8_t i = 0; i < sizeof(Hash); i++) {
		if (result[i] != expected[i]) {
            error = true;
			break;
		}
	}

	lcd_line2();
	if (!error) {
		lcd_writeProgString(PSTR(" OK "));
    } else {
		lcd_writeProgString(PSTR(" ERROR"));
    }
	delayMs(10 * DEFAULT_OUTPUT_DELAY);
	lcd_clear();
    return !error;
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

static void init_buf_state(struct buffer_state * state, const void *input, size_t len) {
	state->p = input;
	state->len = len;
	state->total_len = len;
	state->single_one_delivered = 0;
	state->total_len_delivered = 0;
}

static int calc_chunk(uint8_t chunk[CHUNK_SIZE], struct buffer_state *state) {
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

void hash(Hash hash, const void *input, size_t len) {
    static uint32_t h_orig[8] = { 0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19 };
    static uint32_t h[8];
    memcpy(h, h_orig, sizeof(h));
	int i, j;

	static uint8_t chunk[CHUNK_SIZE];
    memset(chunk, 0, sizeof(chunk));

	struct buffer_state state;

	init_buf_state(&state, input, len);

	while (calc_chunk(chunk, &state)) {
		static uint32_t ah[8];
		static uint32_t w[64];
		memset(ah, 0, sizeof(ah));
		memset(w, 0, sizeof(w));
		const uint8_t *p = chunk;

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

		for (i = 0; i < 8; i++) {
		    ah[i] = h[i];
        }

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

		for (i = 0; i < 8; i++) {
		    h[i] += ah[i];
        }
	}

	for (i = 0, j = 0; i < 8; i++) {
		hash[j++] = (uint8_t) (h[i] >> 24);
		hash[j++] = (uint8_t) (h[i] >> 16);
		hash[j++] = (uint8_t) (h[i] >> 8);
		hash[j++] = (uint8_t) h[i];
	}
}
