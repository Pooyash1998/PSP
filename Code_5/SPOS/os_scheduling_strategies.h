/*! \file
 *  \brief Scheduling library for the OS.
 *
 *  Contains the scheduling strategies.
 *
 *  \author   Lehrstuhl Informatik 11 - RWTH Aachen
 *  \date     2013
 *  \version  2.0
 */

#ifndef _OS_SCHEDULING_STRATEGIES_H
#define _OS_SCHEDULING_STRATEGIES_H

#include "os_scheduler.h"
#include "defines.h"

// Ringbuffer for process queueing.
typedef struct
{
	ProcessID data[MAX_NUMBER_OF_PROCESSES];
    uint8_t size;
	uint8_t head;
	uint8_t tail;
	uint8_t count;
}ProcessQueue;

//! Structure used to store specific scheduling informations such as a time slice
// This is a presence task
typedef struct 
{
	Age age[MAX_NUMBER_OF_PROCESSES];
	uint8_t timeSlice;
	uint8_t timeSlice_mlfq[MAX_NUMBER_OF_PROCESSES];	
	ProcessQueue PQueues[4];
}SchedulingInformation;


//Initialises the scheduling information.
void os_initSchedulingInformation();

//! Used to reset the SchedulingInfo for one process
void os_resetProcessSchedulingInformation(ProcessID id);

//! Used to reset the SchedulingInfo for a strategy
void os_resetSchedulingInformation(SchedulingStrategy strategy);

//! Even strategy
ProcessID os_Scheduler_Even(Process const processes[], ProcessID current);

//! Random strategy
ProcessID os_Scheduler_Random(Process const processes[], ProcessID current);

//! RoundRobin strategy
ProcessID os_Scheduler_RoundRobin(Process const processes[], ProcessID current);

//! InactiveAging strategy
ProcessID os_Scheduler_InactiveAging(Process const processes[], ProcessID current);

//! RunToCompletion strategy
ProcessID os_Scheduler_RunToCompletion(Process const processes[], ProcessID current);

// MultiLevelFeedbackQueue strategy. 
ProcessID os_Scheduler_MLFQ(Process const processes[], ProcessID current);
//Function that removes the given ProcessID from the ProcessQueues.
void MLFQ_removePID (ProcessID pid);
//Returns the corresponding ProcessQueue.
ProcessQueue *MLFQ_getQueue (uint8_t queueID);
//Returns the default number of timeslices for a specific ProcessQueue/priority class.
uint8_t MLFQ_getDefaultTimeslice (uint8_t queueID);
//Maps a process-priority to a priority class.
uint8_t MLFQ_MapToQueue (Priority prio);
//Initializes the given ProcessQueue with a predefined size.
void pqueue_init (ProcessQueue *queue);
//Resets the given ProcessQueue.
void pqueue_reset (ProcessQueue *queue);
//Checks whether there is next a ProcessID.
bool pqueue_hasNext (const ProcessQueue *queue);
//Returns the first ProcessID of the given ProcessQueue.
ProcessID pqueue_getFirst (const ProcessQueue *queue);
//Drops the first ProcessID of the given ProcessQueue.
void pqueue_dropFirst (ProcessQueue *queue);
//Appends a ProcessID to the given ProcessQueue.
void pqueue_append (ProcessQueue *queue, ProcessID pid);
//Removes a ProcessID from the given ProcessQueue.
void pqueue_removePID (ProcessQueue *queue, ProcessID pid);
//rearanges
void MLFQ_rearangeBlocked(Process const processes[], ProcessID pid_blocked);
// change blocked state
void MLFQ_unblock(Process processes[], ProcessID pid_blocked);
//searches for blocked Process and returns its PID
ProcessID searchBlockedProc(Process const processes[]);
#endif
