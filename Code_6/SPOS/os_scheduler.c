/*! \file
 *  \brief Scheduling module for the OS.
 *
 * Contains everything needed to realise the scheduling between multiple processes.
 * Also contains functions to start the execution of programs.
 *
 *  \author   Lehrstuhl Informatik 11 - RWTH Aachen
 *  \date     2013
 *  \version  2.0
 */

#include "os_scheduler.h"
#include "util.h"
#include "os_input.h"
#include "os_scheduling_strategies.h"
#include "os_taskman.h"
#include "os_core.h"
#include "lcd.h"
#include "os_memheap_drivers.h"
#include "os_memory.h"
#include <avr/interrupt.h>
#include <stdbool.h>

//----------------------------------------------------------------------------
// Private Types
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Globals
//----------------------------------------------------------------------------
// is there any Processes with Blocked status in the list ?
bool blockedProcs = false;
//! Array of states for every possible process
Process os_processes[MAX_NUMBER_OF_PROCESSES];

//! Index of process that is currently executed (default: idle)
ProcessID currentProc = 0;
//----------------------------------------------------------------------------
// Private variables
//----------------------------------------------------------------------------

//! Currently active scheduling strategy
SchedulingStrategy currentStrategy = OS_SS_EVEN;

//! Count of currently nested critical sections
uint8_t criticalSectionCount = 0;

//----------------------------------------------------------------------------
// Private function declarations
//----------------------------------------------------------------------------

//! ISR for timer compare match (scheduler)
ISR(TIMER2_COMPA_vect) __attribute__((naked));

//----------------------------------------------------------------------------
// Function headers
//----------------------------------------------------------------------------
void os_dispatcher(void);
//----------------------------------------------------------------------------
// Function definitions
//----------------------------------------------------------------------------

/*!
 *  Timer interrupt that implements our scheduler. Execution of the running
 *  process is suspended and the context saved to the stack. Then the periphery
 *  is scanned for any input events. If everything is in order, the next process
 *  for execution is derived with an exchangeable strategy. Finally the
 *  scheduler restores the next process for execution and releases control over
 *  the processor to that process.
 */
ISR(TIMER2_COMPA_vect) {
	saveContext();
    os_processes[currentProc].sp.as_int = SP;
	SP = BOTTOM_OF_ISR_STACK;
	if(os_getInput() == 0b00001001)
	{
		//Taskmanager aufrufen
		os_waitForNoInput();
		os_taskManMain();
	}
	os_processes[currentProc].checksum = os_getStackChecksum(currentProc);
	if(os_processes[currentProc].state==OS_PS_RUNNING) //terminierte Prozesse nicht wieder aktivieren!
		os_processes[currentProc].state = OS_PS_READY;
	
	currentProc = chooseNextProcess(os_processes, currentProc);
	if(os_processes[currentProc].checksum != os_getStackChecksum(currentProc))
	{
		os_error("Fehler! Stack ist inkonsistent");
	}
	os_processes[currentProc].state = OS_PS_RUNNING;
	SP = os_processes[currentProc].sp.as_int;
	restoreContext();
	
}

/*!
 *  This is the idle program. The idle process owns all the memory
 *  and processor time no other process wants to have.
 */
void idle(void) {
	while (1)
	{
		lcd_writeProgString(PSTR("."));
		delayMs(DEFAULT_OUTPUT_DELAY);
	}
}

/*!
 *  This function is used to execute a program that has been introduced with
 *  os_registerProgram.
 *  A stack will be provided if the process limit has not yet been reached.
 *  This function is multitasking safe. That means that programs can repost
 *  themselves, simulating TinyOS 2 scheduling (just kick off interrupts ;) ).
 *
 *  \param program  The function of the program to start.
 *  \param priority A priority ranging 0..255 for the new process:
 *                   - 0 means least favourable
 *                   - 255 means most favourable
 *                  Note that the priority may be ignored by certain scheduling
 *                  strategies.
 *  \return The index of the new process or INVALID_PROCESS as specified in
 *          defines.h on failure
 */
ProcessID os_exec(Program *program, Priority priority) {
	uint8_t pid;
	os_enterCriticalSection();
	//frein Platz finden 
	for(pid = 0; pid < MAX_NUMBER_OF_PROCESSES; pid++)
	{
		if(os_processes[pid].state == OS_PS_UNUSED) break;
	}
	//keinen freien Platz gefunden or NULL pointer gegeben
	if(pid == MAX_NUMBER_OF_PROCESSES || program == NULL)
	{
		os_leaveCriticalSection();
		return INVALID_PROCESS;
	}
	
	os_processes[pid].state = OS_PS_READY;
	os_processes[pid].program = program;
	os_processes[pid].priority = priority;
	os_removeUsedMemInfo(pid);
	os_processes[pid].sp.as_int = PROCESS_STACK_BOTTOM(pid);
	//Low_Byte der Adresse speichern und sp auf die naechste low-adresse
	*(os_processes[pid].sp.as_ptr --) = ((uint16_t)&os_dispatcher & 0x00FF);
	//High_Byte 
	*(os_processes[pid].sp.as_ptr --) = ((uint16_t)&os_dispatcher >> 8);
	// 32 ProcessorReg + SREG mit NULL initialisiren
	for(int i=1; i<=33 ; i++)
	*(os_processes[pid].sp.as_ptr --) = 0;
	
	
	os_resetProcessSchedulingInformation(pid);
	/////
	os_processes[pid].checksum = os_getStackChecksum(pid);
	os_leaveCriticalSection();
	return pid;
}

/*!
 *  If all processes have been registered for execution, the OS calls this
 *  function to start the idle program and the concurrent execution of the
 *  applications.
 */
void os_startScheduler(void) {
	currentProc = 0; 
	os_processes[currentProc].state = OS_PS_RUNNING;
	SP = os_processes[currentProc].sp.as_int;
	restoreContext();
	
}

/*!
 *  In order for the Scheduler to work properly, it must have the chance to
 *  initialize its internal data-structures and register.
 */
void os_initScheduler(void) {
	for(int i=0; i<MAX_NUMBER_OF_PROCESSES; i++)
		os_processes[i].state = OS_PS_UNUSED;
	// Idle prozess explizit starten
	os_exec(idle, DEFAULT_PRIORITY);
	
	// die Liste von zum automatischen start markierten Programmen durchlaufen
	struct program_linked_list_node *aktuell = autostart_head;  	
	while(aktuell != NULL)
	{
		os_exec(aktuell->program, DEFAULT_PRIORITY);
		aktuell = aktuell->next ;
	}	
}

/*!
 *  A simple getter for the slot of a specific process.
 *
 *  \param pid The processID of the process to be handled
 *  \return A pointer to the memory of the process at position pid in the os_processes array.
 */
Process* os_getProcessSlot(ProcessID pid) {
    return os_processes + pid;
}

/*!
 *  A simple getter to retrieve the currently active process.
 *
 *  \return The process id of the currently active process.
 */
ProcessID os_getCurrentProc(void) {
	return currentProc;
}

/*!
 *  Sets the current scheduling strategy.
 *
 *  \param strategy The strategy that will be used after the function finishes.
 */
void os_setSchedulingStrategy(SchedulingStrategy strategy) {
	currentStrategy = strategy;
	//relevant fuer RR und Inactiveaging
	os_resetSchedulingInformation(strategy);
}

/*!
 *  This is a getter for retrieving the current scheduling strategy.
 *
 *  \return The current scheduling strategy.
 */
SchedulingStrategy os_getSchedulingStrategy(void) {
   return currentStrategy;
}

/*!
 *  Enters a critical code section by disabling the scheduler if needed.
 *  This function stores the nesting depth of critical sections of the current
 *  process (e.g. if a function with a critical section is called from another
 *  critical section) to ensure correct behaviour when leaving the section.
 *  This function supports up to 255 nested critical sections.
 */
void os_enterCriticalSection(void) {
   if(criticalSectionCount < 255) // overflow vermeiden
   {
   uint8_t gieb = SREG & (1 << 7);
   SREG &= ~(1 << 7);
   criticalSectionCount ++ ;
   TIMSK2 &= ~(1 << 1); //0b11111101  OCIE2A
   SREG |= gieb;
   }
   else
   os_error("Zu viele kritische Bereiche");
}

/*!
 *  Leaves a critical code section by enabling the scheduler if needed.
 *  This function utilizes the nesting depth of critical sections
 *  stored by os_enterCriticalSection to check if the scheduler
 *  has to be reactivated.
 */
void os_leaveCriticalSection(void) {

	if(criticalSectionCount > 0)
	{
	uint8_t gieb = SREG & (1 << 7);
	SREG &= ~(1 << 7);
	criticalSectionCount -- ;
	if(criticalSectionCount == 0)
		TIMSK2 |= (1 << 1); //0b11111101  OCIE2A
	SREG |= gieb;	
	}
	else
		os_error("Critical condition error.");
	
}

/*!
 *  Calculates the checksum of the stack for a certain process.
 *
 *  \param pid The ID of the process for which the stack's checksum has to be calculated.
 *  \return The checksum of the pid'th stack.
 */
StackChecksum os_getStackChecksum(ProcessID pid) {
	StackChecksum pruefsumme = 0;
	uint8_t *procStack ; 
	//von erster belegter Stelle von oben bis bottom
	for(procStack=os_processes[pid].sp.as_ptr + 1; (uint16_t)procStack <= PROCESS_STACK_BOTTOM(pid) ; procStack ++)
	{
		pruefsumme = pruefsumme ^ *procStack; 
	}
	return pruefsumme;
}
/*	
	Diese Hilfsfunktion gibt uns die Prozess-ID, die durch unsere derzeitige Scheduling-Strategie
	 bestimmt wird. Die Funktion dient dazu, die ID des naechsten Prozesses zu ermitteln, der ausgefuehrt werden soll.
*/
ProcessID chooseNextProcess(Process processes[], ProcessID current)
{
	// check if theres any blocked proc
	if(blockedProcs && currentStrategy== OS_SS_MULTI_LEVEL_FEEDBACK_QUEUE)
	{
		MLFQ_rearangeBlocked(processes,searchBlockedProc(processes));
	}
	
	ProcessID nextID = 0;
	switch (currentStrategy){
		case OS_SS_EVEN :
			 nextID = os_Scheduler_Even(processes, current);
			 break;
	    case OS_SS_RANDOM :
			 nextID = os_Scheduler_Random(processes, current);
			 break;
		case OS_SS_INACTIVE_AGING :
			 nextID = os_Scheduler_InactiveAging(processes, current);
			 break;
		case OS_SS_ROUND_ROBIN :
			 nextID = os_Scheduler_RoundRobin(processes, current);
			 break;
		case  OS_SS_RUN_TO_COMPLETION:
			 nextID = os_Scheduler_RunToCompletion(processes, current);
			 break; 
		case  OS_SS_MULTI_LEVEL_FEEDBACK_QUEUE:
			 nextID = os_Scheduler_MLFQ(processes, current);
			 break;	 
		}
	// Now set the blocked Proc to ready for the next time
	processes[searchBlockedProc(processes)].state = OS_PS_READY;
		
	if(nextID == 0 && blockedProcs) //there was no ready proc beside idle
		{
			//search again (the blcoked proc will be selected because now its "ready"
			blockedProcs = false;
			return chooseNextProcess(processes, current);
		}
	blockedProcs = false;	
	return nextID;	
}
//Kills a process by cleaning up the corresponding slot in os_processes.
//It also calls the garbage collection in order to free any memory that,
//has been allocated by the killed process.
//Returns True, if the killing process was successful
bool os_kill (ProcessID pid) 
{
	os_enterCriticalSection();
	if (pid > 0 && pid < MAX_NUMBER_OF_PROCESSES){
		if(os_processes[pid].state != OS_PS_UNUSED)
		{
			os_processes[pid].state = OS_PS_UNUSED;
			// Termination via foreign process
			if(pid != os_getCurrentProc()){
			   os_enterCriticalSection();
			   ProcessID temp = os_getCurrentProc();
			   currentProc = pid;
			   for (uint8_t i = 0; i < os_getHeapListLength(); i++)
					os_freeProcessMemory(os_lookupHeap(i), pid);
			   currentProc = temp;
			   os_leaveCriticalSection();
			}
			// Termination of the own process
			else{
				for (uint8_t i = 0; i < os_getHeapListLength(); i++)
					 os_freeProcessMemory(os_lookupHeap(i), pid);
				// leaving all Critical Sections(so the scheduler can select another process)
				criticalSectionCount = 1;
				os_leaveCriticalSection();
				//while(1);
				os_yield();
				}
			os_leaveCriticalSection();
			return true;
		}
	}					
	os_leaveCriticalSection();
	return false;
}	
//Encapsulates any running process in order make it possible for processes to terminate
//This wrapper enables the possibility to perfom a few necessary steps after the actual process function has finished.
void os_dispatcher(void)
{
	ProcessID actProc = os_getCurrentProc();
	Program *progptr = os_processes[actProc].program;
	progptr();
	//Programm abgearbeitet und terminiert
	os_kill(actProc);	
	//while(1);
	//#this could be a problem
	os_yield();
}
/*
Function that is used by processes to give away their remaining processing time 
to another process. Their ProcessState is then being set to OS_PS_BLOCKED 
so the Scheduler ISR knows not to repick the same process. 
*/
void os_yield()
{
	os_enterCriticalSection();
	if(os_processes[os_getCurrentProc()].state != OS_PS_UNUSED) {
		os_processes[os_getCurrentProc()].state = OS_PS_BLOCKED;
		blockedProcs = true;
	}
	uint8_t gieb = SREG >> 7;
	uint8_t kb_anzahl = criticalSectionCount;
	criticalSectionCount = 1;
	os_leaveCriticalSection();
	TIMER2_COMPA_vect();
	os_enterCriticalSection();
	SREG &= 0x7F;
	SREG |= gieb << 7;
	criticalSectionCount = kb_anzahl;
	os_leaveCriticalSection();
}
//resets all the information in MemAllocInfo for a specific process
void os_removeUsedMemInfo(ProcessID pid)
{
	os_processes[pid].usedMemInfo.allocFrameStart = 0;
	os_processes[pid].usedMemInfo.allocFrameEnd = 0;
	os_processes[pid].usedMemInfo.usedIntHeap = false;
	os_processes[pid].usedMemInfo.usedExtHeap = false;
}
/*----------------------------------
	just a few simple setter and Getter
	for the attributes of MemAllocInfo
  ----------------------------------
*/
MemAddr os_getallocFrameStart(ProcessID pid)
{
	return os_processes[pid].usedMemInfo.allocFrameStart;
}
void os_setallocFrameStart(ProcessID pid,MemAddr addr)
{
	os_processes[pid].usedMemInfo.allocFrameStart = addr;
}
MemAddr os_getallocFrameEnd(ProcessID pid)
{
	return os_processes[pid].usedMemInfo.allocFrameEnd;
}
void os_setallocFrameEnd(ProcessID pid,MemAddr addr)
{
	os_processes[pid].usedMemInfo.allocFrameEnd = addr;
}
bool os_getusedIntHeap(ProcessID pid)
{
	return os_processes[pid].usedMemInfo.usedIntHeap;
}
void os_setusedIntHeap(ProcessID pid,bool b)
{
	os_processes[pid].usedMemInfo.usedIntHeap = b;
}
bool os_getusedExtHeap(ProcessID pid)
{
	return os_processes[pid].usedMemInfo.usedExtHeap;
}
void os_setusedExtHeap(ProcessID pid,bool b)
{
	os_processes[pid].usedMemInfo.usedExtHeap = b;
}		