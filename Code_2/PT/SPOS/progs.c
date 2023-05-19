//-------------------------------------------------
//          TestTask: Scheduling Strategies
//-------------------------------------------------

#include <stdlib.h>
#include <stdbool.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include <util/delay.h>

#include "lcd.h"
#include "util.h"
#include "os_scheduler.h"
#include "os_scheduling_strategies.h"

#if VERSUCH < 2
    #warning "Please fix the VERSUCH-define"
#endif

//---- Adjust here what to test -------------------
#define TEST_SS_EVEN                1//(VERSUCH > 2)
#define TEST_SS_RANDOM              1//(VERSUCH > 2)
#define TEST_SS_ROUND_ROBIN         1//(VERSUCH > 2) // Optional in Versuch 2
#define TEST_SS_INACTIVE_AGING      1//(VERSUCH > 2) // Optional in Versuch 2
#define TEST_SS_RUN_TO_COMPLETION   1//(VERSUCH > 2) // Optional in Versuch 2
#define PHASE_1                     1
#define PHASE_2                     1
#define PHASE_3                     1
#define PHASE_4                     1
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



ISR(TIMER2_COMPA_vect);

#define CAPTURE_SIZE 32

// Array containing the correct output values for all four scheduling strategies.
const ProcessID scheduling[][CAPTURE_SIZE] PROGMEM  =  {
    [OS_SS_EVEN]              = {1, 2, 3, 1, 2, 3, 1, 2, 3, 1, 2, 3, 1, 2, 3, 1, 2, 3, 1, 2, 3, 1, 2, 3, 1, 2, 3, 1, 2, 3, 1, 2},
    [OS_SS_RANDOM]            = {1, 3, 1, 1, 3, 3, 3, 3, 1, 1, 3, 2, 3, 3, 3, 1, 3, 2, 1, 2, 1, 1, 2, 2, 1, 3, 1, 1, 1, 1, 2, 1},
    [OS_SS_ROUND_ROBIN]       = {1, 1, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 1, 1, 2, 2, 2, 2, 2, 3},
    [OS_SS_INACTIVE_AGING]    = {1, 3, 3, 3, 2, 3, 3, 3, 2, 3, 1, 3, 2, 3, 3, 3, 2, 3, 3, 1, 3, 2, 3, 3, 3, 2, 3, 3, 1, 3, 2, 3},
    [OS_SS_RUN_TO_COMPLETION] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
};

// An artificial processes array
Process processes[MAX_NUMBER_OF_PROCESSES];

volatile ProcessID capture[CAPTURE_SIZE];
volatile uint8_t captureIndex = 0;

//! Tests if strategy is implemented (default return is 0)
bool isStrategyImplemented() {
    ProcessID nextId = 0;

    // Copy current os_processes
    for (ProcessID i = 0; i < MAX_NUMBER_OF_PROCESSES; i++) {
        processes[i] = *os_getProcessSlot(i);
    }

    // Request next processId without changing anything
    // Current process is always 1
    switch (os_getSchedulingStrategy()) {
        case OS_SS_EVEN:
            nextId = os_Scheduler_Even(processes, os_getCurrentProc());
            break;
        case OS_SS_RANDOM:
            nextId = os_Scheduler_Random(processes, os_getCurrentProc());
            break;
        case OS_SS_ROUND_ROBIN:
            nextId = os_Scheduler_RoundRobin(processes, os_getCurrentProc());
            break;
        case OS_SS_INACTIVE_AGING:
            nextId = os_Scheduler_InactiveAging(processes, os_getCurrentProc());
            break;
        case OS_SS_RUN_TO_COMPLETION:
            nextId = os_Scheduler_RunToCompletion(processes, os_getCurrentProc());
            break;
        default:
            TEST_FAILED("Invalid strategy");
            HALT;
    }

    os_resetSchedulingInformation(os_getSchedulingStrategy());

    return nextId != 0;
}

/*!
 *  Function that sets the current strategy to the given one
 *  and outputs the name of the strategy on the LCD.
 */
void setActiveStrategy(SchedulingStrategy strategy) {
    switch (strategy) {
        case OS_SS_EVEN:
            lcd_writeProgString(PSTR("Even"));
            break;
        case OS_SS_RANDOM:
            lcd_writeProgString(PSTR("Random"));
            // reset global seed to 1 (default)
            // this is just in case students called rand() beforehand
            srand(1);
            break;
        case OS_SS_ROUND_ROBIN:
            lcd_writeProgString(PSTR("RoundRobin"));
            break;
        case OS_SS_INACTIVE_AGING:
            lcd_writeProgString(PSTR("InactiveAging"));
            break;
        case OS_SS_RUN_TO_COMPLETION:
            lcd_writeProgString(PSTR("RunToCompletion"));
            break;
        default:
            break;
    }
    os_setSchedulingStrategy(strategy);
}

// Sets the active strategy, outputs it and checks if it is implemented
void setupStrategyForTest(SchedulingStrategy strategy) {
    lcd_clear();
    setActiveStrategy(strategy);
    if (!isStrategyImplemented()) {
        delayMs(8 * DEFAULT_OUTPUT_DELAY);
        TEST_FAILED("Idle returned");
        HALT;
    }
}

/*!
 *  Function that performs the given strategy for CAPTURE_SIZE steps
 *  and checks if the processes were scheduled correctly
 */
void performStrategyTest(SchedulingStrategy strategy) {
    setupStrategyForTest(strategy);

    delayMs(8 * DEFAULT_OUTPUT_DELAY);
    lcd_clear();

    // Perform scheduling test.
    // Save the id of the running process and call the scheduler.
    for (captureIndex = 0; captureIndex < CAPTURE_SIZE;) {
        capture[captureIndex++] = 1;
        TIMER2_COMPA_vect();
    }

    // Print captured schedule
    for (uint8_t i = 0; i < CAPTURE_SIZE; i++) {
        lcd_writeDec(capture[i]);
    }

    // Check captured schedule
    for (uint8_t i = 0; i < CAPTURE_SIZE; i++) {
        if (capture[i] != pgm_read_byte(&scheduling[strategy][i])) {
            // Move cursor
            lcd_goto((i / 16) + 1, (i % 16) + 1);
            // Show cursor without underlining the position
            lcd_command((LCD_SHOW_CURSOR & ~(1 << 1)) | LCD_DISPLAY_ON);
            HALT;
        }
    }

    delayMs(20 * DEFAULT_OUTPUT_DELAY);
    lcd_clear();

    lcd_writeProgString(PSTR("OK"));
    delayMs(10 * DEFAULT_OUTPUT_DELAY);
}

/*!
 *  Function that performs the given strategy and checks
 *  if all processes could be scheduled
 */
void performSchedulabilityTest(SchedulingStrategy strategy, uint8_t expectation) {
    // Ignore RunToCompletion as the programs never terminate
    if (strategy == OS_SS_RUN_TO_COMPLETION) return;

    setupStrategyForTest(strategy);

    uint8_t captured = 0;

    // Perform strategy CAPTURE_SIZE times
    for (captureIndex = 0; captureIndex < CAPTURE_SIZE;) {
        capture[captureIndex++] = 1;
        TIMER2_COMPA_vect();
    }

    // Calculate which processes were scheduled
    for (uint8_t k = 0; k < CAPTURE_SIZE; k++) {
        captured |= (1 << capture[k]);
    }

    // Check if all processes other than the idle process
    // were scheduled
    if (captured == expectation) {
        lcd_line2();
        lcd_writeProgString(PSTR("OK"));
        delayMs(10 * DEFAULT_OUTPUT_DELAY);

        // Everything fine, so we can stop the test here
        return;
    }

    delayMs(20 * DEFAULT_OUTPUT_DELAY);

	// Output the error to the user
    switch (strategy) {
        case OS_SS_EVEN             : TEST_FAILED("Even"); break;
        case OS_SS_RANDOM           : TEST_FAILED("Random"); break;
        case OS_SS_ROUND_ROBIN      : TEST_FAILED("RoundRobin"); break;
        case OS_SS_INACTIVE_AGING   : TEST_FAILED("Inac.Age."); break;
        case OS_SS_RUN_TO_COMPLETION: TEST_FAILED("RunToComp."); break;
        default: TEST_FAILED("Unknown    strategy"); break;
    }
    lcd_line2();

    // Find processes that were not scheduled (but should be)
    uint8_t notScheduled = ~captured & expectation;
    // Find processes that were scheduled (but should not be)
    uint8_t wrongScheduled = captured & ~expectation;

    // Find the first incorrect process id
    for (ProcessID k = 0; k < MAX_NUMBER_OF_PROCESSES; k++) {
        if (notScheduled & (1 << k)) {
            WRITE("Not sched.: ");
            lcd_writeDec(k);
            break;
        }
        if (wrongScheduled & (1 << k)) {
            WRITE("Falsely sched.: ");
            lcd_writeDec(k);
            break;
        }
    }

    HALT; // Wait forever
}

/*!
 *  Function that tests the given strategy on an artificial empty process array
 *  This ensures the strategy does schedule the idle process if necessary.
 */
void performScheduleIdleTest(SchedulingStrategy strategy) {
    setupStrategyForTest(strategy);

    // Ensure clean states
    for (ProcessID i = 0; i < MAX_NUMBER_OF_PROCESSES; i++) {
        processes[i] = (Process) { .state = OS_PS_UNUSED };
    }

    ProcessID result = 0;
    for (ProcessID i = 0; i < MAX_NUMBER_OF_PROCESSES && result == 0; i++) {
        switch (strategy) {
            case OS_SS_EVEN:
                result = os_Scheduler_Even(processes, i);
                break;
            case OS_SS_RANDOM:
                result = os_Scheduler_Random(processes, i);
                break;
            case OS_SS_ROUND_ROBIN:
                result = os_Scheduler_RoundRobin(processes, i);
                break;
            case OS_SS_INACTIVE_AGING:
                result = os_Scheduler_InactiveAging(processes, i);
                break;
            case OS_SS_RUN_TO_COMPLETION:
                result = os_Scheduler_RunToCompletion(processes, i);
                break;
            default: break;
        }
    }

    if (result == 0) {
        lcd_line2();
		lcd_writeProgString(PSTR("OK"));
		delayMs(10 * DEFAULT_OUTPUT_DELAY);
        return;
	}

    delayMs(20 * DEFAULT_OUTPUT_DELAY);

    // Output the error to the user
	switch (strategy) {
		case OS_SS_EVEN             : TEST_FAILED("Even"); break;
		case OS_SS_RANDOM           : TEST_FAILED("Random"); break;
		case OS_SS_ROUND_ROBIN      : TEST_FAILED("RoundRobin"); break;
		case OS_SS_INACTIVE_AGING   : TEST_FAILED("Inac.Age."); break;
		case OS_SS_RUN_TO_COMPLETION: TEST_FAILED("RunToComp."); break;
        default: TEST_FAILED("Unknown    strategy"); break;
	}
    lcd_line2();
	WRITE("Idle not sched.");
    HALT;
}

/*!
 *  Writes its own pid to the capture array
 *  and calls the ISR manually afterwards
 */
void self_registering_program(void) {
    ProcessID pid = os_getCurrentProc();
    while (1) {
        if (captureIndex < CAPTURE_SIZE) {
            capture[captureIndex++] = pid;
        }
        TIMER2_COMPA_vect();
    }
}

/*!
 * Program that deactivates the scheduler, spawns two programs
 * and performs the test
 */
REGISTER_AUTOSTART(controller_program)
void controller_program(void) {
    // Disable scheduler-timer
    cbi(TCCR2B, CS22);
    cbi(TCCR2B, CS21);
    cbi(TCCR2B, CS20);

    os_getProcessSlot(os_getCurrentProc())->priority = 2;

    os_exec(self_registering_program, 5);
    os_exec(self_registering_program, 17);

    SchedulingStrategy strategies[] = {
#if TEST_SS_EVEN
        OS_SS_EVEN,
#endif
#if TEST_SS_RANDOM
        OS_SS_RANDOM,
#endif
#if TEST_SS_ROUND_ROBIN
        OS_SS_ROUND_ROBIN,
#endif
#if TEST_SS_INACTIVE_AGING
        OS_SS_INACTIVE_AGING,
#endif
#if TEST_SS_RUN_TO_COMPLETION
        OS_SS_RUN_TO_COMPLETION,
#endif
    };

    uint8_t numStrategies = sizeof(strategies) / sizeof(SchedulingStrategy);

#if PHASE_1

    lcd_clear();
    lcd_writeProgString(PSTR("Phase 1: Strategies"));
    delayMs(20 * DEFAULT_OUTPUT_DELAY);

    // Start strategies test
    for (uint8_t k = 0; k < numStrategies; k++) {
        performStrategyTest(strategies[k]);
    }

#endif
#if PHASE_2

    lcd_clear();
    lcd_writeProgString(PSTR("Phase 2: Idle"));
    delayMs(20 * DEFAULT_OUTPUT_DELAY);

    // Start strategies test
    for (uint8_t k = 0; k < numStrategies; k++) {
        performScheduleIdleTest(strategies[k]);
    }

#endif

	// Execute programs so all process slots are in use
    os_exec(self_registering_program, DEFAULT_PRIORITY);
    os_exec(self_registering_program, DEFAULT_PRIORITY);
    os_exec(self_registering_program, DEFAULT_PRIORITY);
    os_exec(self_registering_program, DEFAULT_PRIORITY);

#if PHASE_3

    lcd_clear();
    lcd_writeProgString(PSTR("Phase 3: Schedulability All"));
    delayMs(20 * DEFAULT_OUTPUT_DELAY);

    // Start schedulability test
    for (uint8_t k = 0; k < numStrategies; k++) {
        performSchedulabilityTest(strategies[k], 0b11111110);
    }

#endif
#if PHASE_4

    lcd_clear();
    lcd_writeProgString(PSTR("Phase 4: Schedulability Partial"));
    delayMs(20 * DEFAULT_OUTPUT_DELAY);

    // Reset some processes
    os_getProcessSlot(3)->state = OS_PS_UNUSED;
    os_getProcessSlot(4)->state = OS_PS_UNUSED;
    os_getProcessSlot(7)->state = OS_PS_UNUSED;

    // Start schedulability test
    for (uint8_t k = 0; k < numStrategies; k++) {
        performSchedulabilityTest(strategies[k], 0b01100110);
    }

#endif

    // All tests passed
    TEST_PASSED;
    HALT;
}
