#ifndef TIMING_H
#define TIMING_H

//#include <sstream>
#include <time.h>
#include <stdio.h>
#include <iostream>

long cpuTime();
void printCPUtime(clock_t); /* display a time value in sec. */
void printCPUtime(clock_t, std::ostream&); /* display a time value in sec. */
void dateToday(char*);
void wallTime(char*);

#endif
