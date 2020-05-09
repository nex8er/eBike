#ifndef TASKSCHED_H
#define TASKSCHED_H

#include <TaskSchedulerDeclarations.h>
#include <define.h>

extern Scheduler runner;

void TestTask(void);
void MotionDetect(void);
void MildSiren(void);

extern Task T_test;
extern Task T_MotionDetect;
extern Task T_MildSiren;

#endif