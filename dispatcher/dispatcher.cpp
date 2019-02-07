#include "../includes_usr/constants.h"
#include "../includes_usr/dispatcher.h"
#include "../includes_usr/file_io.h"
#include "../includes_usr/joblist.h"
#include "../includes_usr/logger_single_thread.h"
#include <queue>

PCB runningPCB;
std::queue<PCB> ready_Q;
std::queue<PCB> blocked_Q;

void dispatcher::init() {
	//Clear queues
	while (!ready_Q.empty()) {
		ready_Q.pop();
	}
	while (!blocked_Q.empty()) {
		blocked_Q.pop();
	}
	//Reset PCB to defaults
	runningPCB.process_number = UNINITIALIZED;
	runningPCB.start_time = UNINITIALIZED;
	runningPCB.cpu_time = UNINITIALIZED;
	runningPCB.io_time = UNINITIALIZED;
}

PCB dispatcher::getCurrentJob() {
	return runningPCB;
}

void dispatcher::addJob(PCB &myPCB) {
	ready_Q.push(myPCB);
}

int dispatcher::processInterrupt(int interrupt) {
	if (interrupt == SWITCH_PROCESS) {
		if (ready_Q.empty()) {
			if (blocked_Q.empty()) {
				return NO_JOBS;
			}
			return BLOCKED_JOBS;
		}
		PCB tmpPCB = ready_Q.front(); //Temp value of front ready
		ready_Q.pop(); //remove front
		//Check if running is valid
		if (runningPCB.process_number != UNINITIALIZED) {
			ready_Q.push(runningPCB);
		}
		//Temp is now current
		runningPCB.cpu_time = tmpPCB.cpu_time;
		runningPCB.process_number = tmpPCB.process_number;
		runningPCB.io_time = tmpPCB.io_time;
		runningPCB.start_time = tmpPCB.start_time;
		return PCB_SWITCHED_PROCESSES;
	}
	if (interrupt == IO_COMPLETE) {
		if (blocked_Q.empty()) {
			return PCB_BLOCKED_QUEUE_EMPTY;
		}
		while (!blocked_Q.empty()) {
			ready_Q.push(blocked_Q.front());
			blocked_Q.pop();
		}
		return PCB_MOVED_FROM_BLOCKED_TO_READY;
	}

	return PCB_UNIMPLEMENTED;
}

int dispatcher::doTick() {
	int returnval = NO_JOBS; //Default
	//0 -> Not running OR -1 -> Uninitialized
	if (runningPCB.cpu_time <= 0) {
		if (ready_Q.empty()) {
			if (blocked_Q.empty() && runningPCB.process_number == UNINITIALIZED) {
				return NO_JOBS;
			}
			runningPCB.process_number = UNINITIALIZED;
			return BLOCKED_JOBS;
		}
		runningPCB = ready_Q.front();
		ready_Q.pop();
		return PCB_MOVED_FROM_READY_TO_RUNNING;
	}
	//cpu time > 0 -> running
	runningPCB.cpu_time--; //Work done
	if (runningPCB.cpu_time > 0) {
		return PCB_CPUTIME_DECREMENTED;
	}
	//Does NOT make an IO call
	if (runningPCB.io_time == 0) {
		returnval = PCB_FINISHED;
	} else {
		blocked_Q.push(runningPCB);
		returnval = PCB_ADDED_TO_BLOCKED_QUEUE;
	}
	runningPCB.process_number = UNINITIALIZED; //Marked as invalid
	return returnval;
}
