#ifndef ALARM_H
#define ALARM_H

#include <Arduino.h>

void MotionDetect(void);
void MildSiren(void);

enum alarm_mode {
	NOISY,
	MILD,
	SILENT,
	OFF
};

class AlarmManager {
public:
	
	void Execute(void);
	alarm_mode NextMode(void);
	void Off(void);
	bool detectionFlag = false;
private:
	alarm_mode mode = SILENT;
	
};

extern AlarmManager alarmMngr;

#endif