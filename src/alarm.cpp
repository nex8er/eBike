#include "alarm.h"
#include "periphery.h"
#include "blynk_manager.h"
#include "tasksched.h"

AlarmManager alarmMngr;

void MotionDetect() {
	mpu.readRawAccel();
	Activites act = mpu.readActivites();

	if (act.isActivity) {
		motionLed.on();
		Blynk.notify("Movement noticed!");                               // Push-уведомление
		T_MotionDetect.setInterval(10000);
		alarmMngr.Execute();

		alarmMngr.detectionFlag = false;
	}
	else {
		if (!alarmMngr.detectionFlag) {
			motionLed.off();
			T_MotionDetect.setInterval(1000);
			ALARM_OFF;
			T_MildSiren.disable();
			alarmMngr.detectionFlag = true;
		}
	}
}


void AlarmManager::Execute () {
	switch (mode)
		{
		case NOISY:
			ALARM_ON;
			break;
		case SILENT:
			break;
		case MILD:
			T_MildSiren.enable();
			T_MildSiren.restart();
			break;
		case OFF:
			ALARM_OFF;
			//T_MotionDetect.disable();
			break;
		default:
			break;
		}
}

void AlarmManager::Off() {
	T_MotionDetect.disable();
	T_MildSiren.disable();
	ALARM_OFF;
	motionLed.off();
}

alarm_mode AlarmManager::NextMode() {
	switch (mode) {
		case NOISY:
			mode = MILD;
			break;
		case MILD:
			mode = SILENT;
			break;
		case SILENT:
			mode = NOISY;
			break;
		default:
			mode = SILENT;
			break;
	} 
	return mode;
}
void MildSiren() {
	uint8_t w = 3;
	while (w) {
		ALARM_ON;
		PixelsSet(Both, 220,0,0);
		delay(130);
		ALARM_OFF;
		PixelsClear(Both);
		delay(200);
		w--;
	}
}

void lockSoundNotification (uint8_t n) {
	while(n) {
		ALARM_ON;
		delay(100);
		ALARM_OFF;
		delay(200);
		n--;
	}
}

BLYNK_WRITE(BLYNKPORT_Lock) {
	uint8_t trySet = 0;
	mpu.readRawAccel();
	Activites act = mpu.readActivites();

	if(!param.asInt()) {
		while(act.isActivity) {
			delay(200);
			if (trySet > 10) {
				lockSoundNotification(3);
				Blynk.virtualWrite(BLYNKPORT_Lock, true);
				return;
			}
			else {
				mpu.readRawAccel();
				act = mpu.readActivites();
				trySet++;
			}

		}
		T_MotionDetect.enable();
		T_MotionDetect.setInterval(500);
		T_MotionDetect.restart();
		lockSoundNotification(1);
	}
	else {
		alarmMngr.Off();
		lockSoundNotification(2);
	}
}


BLYNK_WRITE(BLYNKPORT_Silent) {
	if(param.asInt()) {
		const alarm_mode cMode = alarmMngr.NextMode();
		ALARM_OFF;
		
		switch (cMode)
		{
		case NOISY:
			Blynk.setProperty(BLYNKPORT_Silent, "offLabel", "🔔 Noisy");
			break;
		case SILENT:
			Blynk.setProperty(BLYNKPORT_Silent, "offLabel", "🔕 Silent");
			break;
		case MILD:
			Blynk.setProperty(BLYNKPORT_Silent, "offLabel", "🔊 Mild");
			break;
		default:
			break;
		}
		lockSoundNotification(1);
	}
} 