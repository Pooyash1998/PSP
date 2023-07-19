/*! \file
 *  \brief Scheduling module for the OS.
 *
 *  Contains the scheduler and process switching functionality for the OS.
 *
 *  \author   Lehrstuhl Informatik 11 - RWTH Aachen
 *  \date     2013
 *  \version  2.0
 */

#ifndef _OS_SCHEDULER_H
#define _OS_SCHEDULER_H

#include <stdbool.h>

#include "defines.h"
#include "os_process.h"

//----------------------------------------------------------------------------
// Types
//----------------------------------------------------------------------------

//! The enum specifying which scheduling strategies exist
typedef enum SchedulingStrategy {
    OS_SS_EVEN,
    OS_SS_RANDOM,
    OS_SS_RUN_TO_COMPLETION,
    OS_SS_ROUND_ROBIN,
    OS_SS_INACTIVE_AGING,
	OS_SS_MULTI_LEVEL_FEEDBACK_QUEUE
} SchedulingStrategy;

//----------------------------------------------------------------------------
// Function headers
//----------------------------------------------------------------------------

//! Get a pointer to the process structure by process ID
Process* os_getProcessSlot(ProcessID pid);

//! Starts the scheduler
void os_startScheduler(void);

//! Executes a process by instantiating a program
ProcessID os_exec(Program program, Priority priority);

//! Returns the number of programs
uint8_t os_getNumberOfRegisteredPrograms(void);

//! Initializes scheduler arrays
void os_initScheduler(void);

//! Returns the currently active process
ProcessID os_getCurrentProc(void);

//! Sets the scheduling strategy
void os_setSchedulingStrategy(SchedulingStrategy strategy);

//! Gets the current scheduling strategy
SchedulingStrategy os_getSchedulingStrategy(void);

//! Calculates the checksum of the stack for the corresponding process of pid.
StackChecksum os_getStackChecksum(ProcessID pid);

//! Hilfsfunktion gibt uns die Prozess-ID,des naechsten Prozesses der ausgefuehrt werden soll.
ProcessID chooseNextProcess(Process processes[], ProcessID current);

//!Used to kill a running process and clear the corresponding process slot.
bool os_kill(ProcessID pid);

//resets all the information in MemAllocInfo for a specific process
void os_removeUsedMemInfo(ProcessID pid);

//Used to give away remaining processing time and call the scheduler immediately.
void os_yield();
/*----------------------------------
	just a few simple setter and Getter
	for the attributes of MemAllocInfo
  ----------------------------------
*/
MemAddr os_getallocFrameStart(ProcessID pid);
void os_setallocFrameStart(ProcessID pid,MemAddr addr);
MemAddr os_getallocFrameEnd(ProcessID pid);
void os_setallocFrameEnd(ProcessID pid,MemAddr addr);
bool os_getusedIntHeap(ProcessID pid);
void os_setusedIntHeap(ProcessID pid,bool b);
bool os_getusedExtHeap(ProcessID pid);
void os_setusedExtHeap(ProcessID pid,bool b);

//----------------------------------------------------------------------------
// Critical section management
//----------------------------------------------------------------------------

//! Enters a critical code section
void os_enterCriticalSection(void);

//! Leaves a critical code section
void os_leaveCriticalSection(void);

#endif
