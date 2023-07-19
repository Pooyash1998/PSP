/*! \file

Scheduling strategies used by the Interrupt Service RoutineA from Timer 2 (in scheduler.c)
to determine which process may continue its execution next.

The file contains five strategies:
-even
-random
-round-robin
-inactive-aging
-run-to-completion
*/

#include "os_scheduling_strategies.h"
#include "defines.h"
#include "os_core.h"
#include <stdlib.h>


//GlobalVariable 
SchedulingInformation schedulingInfo;

// calculates the number of processes with READY state.
uint8_t findNumberOfReadyProcs(Process const processes[]) {
	uint8_t i = 0;
	ProcessID pid;
	for (pid = 0; pid < MAX_NUMBER_OF_PROCESSES; pid++)
	{
		if(processes[pid].state==OS_PS_READY) 
			i++;
	}

	return i;
}
/*tells if there is any selectable process with BLOCKED state other than IDLE
  it returns the pid of the blocked process is there was any 
  otherwise returns 0 which stands for idle */
ProcessID searchBlockedProc(Process const processes[])
{
	ProcessID pid = 0;
	uint8_t i ;
	for (i = 1; i < MAX_NUMBER_OF_PROCESSES; i++)
	{
		if(processes[i].state==OS_PS_BLOCKED)
			pid = i ;
	}
	return pid;
}

//Initialises the scheduling information.
void os_initSchedulingInformation()
{
	for (int i=0 ; i<4 ; i++)
		pqueue_init(&schedulingInfo.PQueues[i]);
	
}

/*!
 *  Reset the scheduling information for a specific strategy
 *  This is only relevant for RoundRobin and InactiveAging
 *  and is done when the strategy is changed through os_setSchedulingStrategy
 *
 *  \param strategy  The strategy to reset information for
 */
void os_resetSchedulingInformation(SchedulingStrategy strategy) {
    
    Process *currentslot = os_getProcessSlot(os_getCurrentProc());
	if(strategy == OS_SS_ROUND_ROBIN)
	{
		schedulingInfo.timeSlice = currentslot->priority ;
	}
	else if(strategy == OS_SS_INACTIVE_AGING)
	{
		for (uint8_t i=0; i < MAX_NUMBER_OF_PROCESSES; i++)
		schedulingInfo.age[i] = 0;
	}
	else if(strategy == OS_SS_MULTI_LEVEL_FEEDBACK_QUEUE)
	{
		for (uint8_t i = 0; i < 4; i++) 
			pqueue_reset(&schedulingInfo.PQueues[i]);
		for (uint8_t i = 1; i < MAX_NUMBER_OF_PROCESSES; i++){
			Process *proc = os_getProcessSlot(i); 
			if(proc->state != OS_PS_UNUSED){ //Doublecheck this if #problemquelle
			schedulingInfo.timeSlice_mlfq[i] = MLFQ_getDefaultTimeslice(MLFQ_MapToQueue(proc->priority));
			pqueue_append(MLFQ_getQueue(MLFQ_MapToQueue(proc->priority)), i);
			}
		}
	}
}

/*!
 *  Reset the scheduling information for a specific process slot
 *  This is necessary when a new process is started to clear out any
 *  leftover data from a process that previously occupied that slot
 *
 *  \param id  The process slot to erase state for
 */
void os_resetProcessSchedulingInformation(ProcessID id) {
	if(os_getSchedulingStrategy() == OS_SS_INACTIVE_AGING){
		schedulingInfo.age[id] = 0;
		}
	else if(os_getSchedulingStrategy() == OS_SS_MULTI_LEVEL_FEEDBACK_QUEUE)
	{
		if(id != 0){
			Process *proc = os_getProcessSlot(id);
			MLFQ_removePID(id);
			schedulingInfo.timeSlice_mlfq[id] = MLFQ_getDefaultTimeslice(MLFQ_MapToQueue(proc->priority));
			pqueue_append(MLFQ_getQueue(MLFQ_MapToQueue(proc->priority)), id);
		}
	}
}

/*!
 *  This function implements the even strategy. Every process gets the same
 *  amount of processing time and is rescheduled after each scheduler call
 *  if there are other processes running other than the idle process.
 *  The idle process is executed if no other process is ready for execution
 *
 *  \param processes An array holding the processes to choose the next process from.
 *  \param current The id of the current process.
 *  \return The next process to be executed determined on the basis of the even strategy.
 */
ProcessID os_Scheduler_Even(Process const processes[], ProcessID current) {
	if(findNumberOfReadyProcs(processes) > 1) {   //is there any other waiting procs other than IDLE?
		ProcessID index;
		for (index = current + 1; index<MAX_NUMBER_OF_PROCESSES; index++ )
		{
			if (processes[index].state == OS_PS_READY)
				return index;
		}
		//falls current der einzige wartende Prozess ist, bekommt er wieder Zeit.
		for (index = 1; index <= current; index++ ) 
		{
			if (processes[index].state == OS_PS_READY)
				return index;
		}
	}
	return searchBlockedProc(processes);
}

/*!
 *  This function implements the random strategy. The next process is chosen based on
 *  the result of a pseudo random number generator.
 *
 *  \param processes An array holding the processes to choose the next process from.
 *  \param current The id of the current process.
 *  \return The next process to be executed determined on the basis of the random strategy.
 */
ProcessID os_Scheduler_Random(Process const processes[], ProcessID current) {
	
	ProcessID index;
	uint16_t RandMod;
	RandMod = rand() % (findNumberOfReadyProcs(processes) - 1); //exclue idle with -1
	for( index = 1 ; index < MAX_NUMBER_OF_PROCESSES; index++ )
	{
		if(processes[index].state == OS_PS_READY) {
			if (RandMod == 0) 
				return index;
			RandMod--;
		}
	}
	return searchBlockedProc(processes);
}

/*!
 *  This function implements the round-robin strategy. In this strategy, process priorities
 *  are considered when choosing the next process. A process stays active as long its time slice
 *  does not reach zero. This time slice is initialized with the priority of each specific process
 *  and decremented each time this function is called. If the time slice reaches zero, the even
 *  strategy is used to determine the next process to run.
 *
 *  \param processes An array holding the processes to choose the next process from.
 *  \param current The id of the current process.
 *  \return The next process to be executed determined on the basis of the round robin strategy.
 */
ProcessID os_Scheduler_RoundRobin(Process const processes[], ProcessID current) {
	
	if(findNumberOfReadyProcs(processes) > 1){
		//proc soll priority-mal durchgefuehrt werden
	   if (processes[current].state == OS_PS_READY && schedulingInfo.timeSlice > 1){
		   schedulingInfo.timeSlice --;
		   return current;
	   } 
	   else {
		   current = os_Scheduler_Even(processes, current);
		   // Timeslot fuer den neuen Prozess muss neu gesetzt werden bevor wir weiter machen
		   schedulingInfo.timeSlice = processes[current].priority;
		   return current;
		  }
	  }
	return searchBlockedProc(processes);	  
}

/*!
 *  This function realizes the inactive-aging strategy. In this strategy a process specific integer ("the age") is used to determine
 *  which process will be chosen. At first, the age of every waiting process is increased by its priority. After that the oldest
 *  process is chosen. If the oldest process is not distinct, the one with the highest priority is chosen. If this is not distinct
 *  as well, the one with the lower ProcessID is chosen. Before actually returning the ProcessID, the age of the process who
 *  is to be returned is reset to its priority.
 *
 *  \param processes An array holding the processes to choose the next process from.
 *  \param current The id of the current process.
 *  \return The next process to be executed, determined based on the inactive-aging strategy.
 */
ProcessID os_Scheduler_InactiveAging(Process const processes[], ProcessID current) {

	if (findNumberOfReadyProcs(processes) > 1)
	{
		ProcessID pidOldest = 0;
		ProcessID pidNextProc = 0;
		for(ProcessID pid = 1; pid < MAX_NUMBER_OF_PROCESSES; pid++){
			if (processes[pid].state == OS_PS_READY){
				schedulingInfo.age[pid] += processes[pid].priority;
				if (schedulingInfo.age[pid] > schedulingInfo.age[pidOldest]){
					 pidOldest = pid;
					 pidNextProc = pid;
					 }
				else if (schedulingInfo.age[pidOldest] == schedulingInfo.age[pid] 
						&& processes[pid].priority > processes[pidOldest].priority )
						{	pidOldest = pid;
							pidNextProc = pid;
						}
				else if (schedulingInfo.age[pidOldest] == schedulingInfo.age[pid]
						&& processes[pid].priority == processes[pidOldest].priority
						&& pid < pidOldest)
						{
							pidNextProc = pid;
						}
			}
		}
		schedulingInfo.age[pidNextProc] = 0;
		return pidNextProc;
	}
	
	return searchBlockedProc(processes);
}

/*!
 *  This function realizes the run-to-completion strategy.
 *  As long as the process that has run before is still ready, it is returned again.
 *  If  it is not ready, the even strategy is used to determine the process to be returned
 *
 *  \param processes An array holding the processes to choose the next process from.
 *  \param current The id of the current process.
 *  \return The next process to be executed, determined based on the run-to-completion strategy.
 */
ProcessID os_Scheduler_RunToCompletion(Process const processes[], ProcessID current) {
	 if (processes[current].state == OS_PS_READY ) 
		return current;
	else 
		return os_Scheduler_Even(processes, current);
		
    return searchBlockedProc(processes);
}
// MultiLevelFeedbackQueue strategy.
ProcessID os_Scheduler_MLFQ(Process const processes[], ProcessID current)
{
	ProcessID Nextpid=0;
	uint8_t qid,i,j;
	bool flag = 0;
	ProcessQueue *q;

	if (findNumberOfReadyProcs(processes) > 1)
	{	
		for (qid = 0; qid < 4; qid++)
		{
			q = MLFQ_getQueue(qid);
			if(!pqueue_hasNext(q)) continue;
			
			j = q->tail;
			for(i = 0; i < q->count ; i++)
			{
				uint8_t k = (j + i) % q->size;
				if( q->data[k]!=0 && (processes[q->data[k]].state == OS_PS_READY))
					{
						flag = true; 
						break;
					}
			}
			if(flag)
			{		
				Nextpid = q->data[ (j + i) % q->size ];
				break;
			}
		}
		//Set Timeslice (for ready)
		if(Nextpid != 0)
		{
			schedulingInfo.timeSlice_mlfq[Nextpid] --;
			if(schedulingInfo.timeSlice_mlfq[Nextpid] == 0){
				pqueue_removePID(MLFQ_getQueue(qid), Nextpid);
				if(qid < 3) qid ++;
				pqueue_append(MLFQ_getQueue(qid), Nextpid);
				schedulingInfo.timeSlice_mlfq[Nextpid]= MLFQ_getDefaultTimeslice(qid);
			}
		}
	}
	
	return Nextpid;
}
//Function that removes the given ProcessID from the ProcessQueues.
void MLFQ_removePID (ProcessID pid)
{
	for(uint8_t i=0; i < 4; i++)
		pqueue_removePID(MLFQ_getQueue(i),pid);
}
//Returns the corresponding ProcessQueue.
ProcessQueue *MLFQ_getQueue (uint8_t queueID)
{
	return &schedulingInfo.PQueues[queueID];
}
//Returns the default number of timeslices for a specific ProcessQueue/priority class.
uint8_t MLFQ_getDefaultTimeslice (uint8_t queueID)
{
	switch(queueID)
	{
		case 0 : return 1; break;
		case 1 : return 2; break;
		case 2 : return 4; break;
		case 3 : return 8; break;
		default: os_error("wrong QueueId");
	}
	return 0;
}
//Maps a process-priority to a priority class.
uint8_t MLFQ_MapToQueue (Priority prio)
{
	uint8_t klasse = (prio & 0b11000000) >> 6;
	switch(klasse) 
	{
		case 3 : return 0; break;
		case 2 : return 1; break;
		case 1 : return 2; break;
		case 0 : return 3; break;
		default: os_error("Priority not defined");
	}
	return 0;
}
//Initializes the given ProcessQueue with a predefined size.
void pqueue_init (ProcessQueue *queue)
{
	queue->count = 0;
	queue->size = MAX_NUMBER_OF_PROCESSES;
	queue->head = 0;
	queue->tail = 0;
	for(uint8_t i = 0; i < MAX_NUMBER_OF_PROCESSES; i++)
		queue->data[i] = 0;
}
//Resets the given ProcessQueue.
void pqueue_reset (ProcessQueue *queue)
{
	queue->count = 0;
	queue->head = 0;
	queue->tail = 0; 
	queue->size = MAX_NUMBER_OF_PROCESSES;
	for(uint8_t i = 0; i < MAX_NUMBER_OF_PROCESSES; i++)
		queue->data[i] = 0;
}
//Checks whether there is next a ProcessID.
bool pqueue_hasNext (const ProcessQueue *queue)
{
	return queue->count > 0 ;
}
//Returns the first ProcessID of the given ProcessQueue.
ProcessID pqueue_getFirst (const ProcessQueue *queue)
{
	return queue->data[queue->tail];
}
//Drops the first ProcessID of the given ProcessQueue.
void pqueue_dropFirst (ProcessQueue *queue)
{
	if(queue->count == 0) os_error("Can't dequeque an empty Queue!");
	else
	{
		queue->data[queue->tail] = 0;
		queue->tail ++;
		if(queue->tail == queue->size)
		{
			queue->tail = 0;
		}
		queue->count --;	
	}
}
//Appends a ProcessID to the given ProcessQueue.
void pqueue_append (ProcessQueue *queue, ProcessID pid)
{
	if(queue->count == queue->size) os_error("Queue Owerflow!");
	else 
	{
		queue->data[queue->head] = pid;
		queue->head ++;
		if(queue->head == queue->size)
			queue->head = 0;
		queue->count ++; 
	}
}
//Removes a ProcessID from the given ProcessQueue.
void pqueue_removePID (ProcessQueue *queue, ProcessID pid)
{
	if(!pqueue_hasNext(queue)) return;
	
	uint8_t i;
	int8_t position = -1;
	//find the pid in queue
	for(i = 0; i < queue->count ; i++)
	{
		if(queue->data[ (i+queue->tail) % queue->size ] == pid){
			position = (i+queue->tail) % queue->size;
			break;
		}
	}
	//remove the pid slot in queue & move elements after the target element by shiftCount positions
	if(position>=0)
	{
		uint8_t shiftCount = queue->count - i - 1;
		for (i = 0; i <= shiftCount; i++) {
			queue->data[(position + i) % queue->size] = queue->data[(position + i + 1) % queue->size];
		}
		queue->count --;
		queue->head = (queue->head - 1 + queue->size) % queue->size;
	}
}
// reagarnges a blocked process
void MLFQ_rearangeBlocked(Process const processes[], ProcessID pid_blocked)
{	
	ProcessQueue *pq;
	ProcessID pid;
	uint8_t remaining_ts = 0;
	for (uint8_t i = 0; i < 4; i++) {
		pq = MLFQ_getQueue(i);
		if(pqueue_hasNext(pq))
		{
			pid = pqueue_getFirst(pq);
			if(processes[pid].state == OS_PS_BLOCKED)
			{
				remaining_ts = schedulingInfo.timeSlice_mlfq[pid];
				pqueue_removePID(pq,pid);
				if(remaining_ts == 0)
				{
					if(i<3) { pqueue_append(MLFQ_getQueue(i+1),pid); 
							  schedulingInfo.timeSlice_mlfq[pid] = MLFQ_getDefaultTimeslice(i+1);}
					else    { pqueue_append(pq,pid);
							  schedulingInfo.timeSlice_mlfq[pid] = MLFQ_getDefaultTimeslice(i); }
				}
				else
				{ 
					pqueue_append(pq,pid);
					schedulingInfo.timeSlice_mlfq[pid] = remaining_ts;
				}
			}
		}
	}
}