#include <iostream>
#include "../includes_usr/constants.h"
#include "../includes_usr/dispatcher.h"
#include "../includes_usr/file_io.h"
#include "../includes_usr/joblist.h"

//assumme the worst
bool joblistHasJobs = false;

int joblist::init(const char* filename) {
	loadData(filename);
	sortData(START_TIME);
	joblistHasJobs = size() > 0;
	return SUCCESS;
}

PCB joblist::getNextJob() {
	return getNext();
}

int joblist::doTick(int currentTick) {
	joblistHasJobs = size() > 0;
	if (!joblistHasJobs) {
		return NO_JOBS;
	}
	if (currentTick == peekNextStartTime()) {
		return ADD_JOB_TO_DISPATCHER;
	}
	return WAITING_TO_ADD_JOB_TO_DISPATCHER;
}

