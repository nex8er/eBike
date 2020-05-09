#ifndef BLYNK_MANAGER_H
#define BLYNK_MANAGER_H

#include <Arduino.h>
#include "blynk_defines.h"
#include "utils.h"
#include "net.h"
#include <BlynkSimpleEsp32.h>
#include "modem.h"
#include <BlynkSimpleTinyGSM.h>

#define GSM

#ifdef GSM
	static BlynkArduinoClient _blynkTransport;
	extern BlynkSIM Blynk;
#elif
	static WiFiClient _blynkWifiClient;
	static BlynkArduinoClient _blynkTransport(_blynkWifiClient);
	extern BlynkWifi Blynk;
#endif


#include <BlynkWidgets.h>

extern int connectAttempts;

// again, only declare your widgets here - will also define these in cpp!
//extern WidgetRTC rtc;
extern WidgetTerminal terminal;
extern WidgetLCD lcd;
extern WidgetLED motionLed;


class BlynkManager
{
public:

		void Begin(BlynkManager *instance);
		static bool GetIsFirstConnect();
		static bool SetIsFirstConnect(bool b);
		
private:
		static BlynkManager *instance;
};

void BlynkRun(void);

#endif