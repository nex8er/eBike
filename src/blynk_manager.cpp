#include "blynk_manager.h"
#include "utils.h"
#include "net.h"
#include "tasksched.h"
#include "misc.h"
#include "periphery.h"
#include "private.h"

const char *auth = BLYNK_KEY;

Task T_BlynkRun(250, TASK_FOREVER, &BlynkRun, &runner, true);

int connectAttempts = 0;

void BlynkManager::Begin(BlynkManager *i) {
	instance = i;
	#ifdef GSM
		SetModem();
		Blynk.config(modem, auth);
	#elif
		Blynk.config(auth);
	#endif

	Blynk.connect();
}


void BlynkRun() {
	if (Blynk.connected() == Blynk.CONNECTED) {
		if (Blynk.run()) {
			connectAttempts = 0;
		}
	}
	else {
		Log("Blynk is not connect!\n");
		Blynk.disconnect();
		delay(1000);
		//Blynk.config(modem, blynk_token);
		Blynk.connect(900UL);
		connectAttempts++;
	}

	if (connectAttempts > 0) {
		Logf(I, "Blynk connect attempt - %u\n", connectAttempts);
		if (connectAttempts == 10) {
			ModemReconnect();
			delay(5000);
			Blynk.connect();
		}
		if (connectAttempts > 20) {
			Log("Could not connect to Blynk Server. Restart!");
			esp_restart();
		}
	}
}


BLYNK_WRITE(BLYNKPORT_Check) {
	if (!param.asInt()) {
		return;
	}
	Blynk.setProperty(BLYNKPORT_Check, "offLabel", "🔄");
	//NetworkInfo();
	Blynk.virtualWrite(BLYNKPORT_BatteryTemp, GetBatTemp());
	Serial.println(GetBatTemp());
	Blynk.virtualWrite(BLYNKPORT_LVBatLevel, adc.GetLVLevel());
	delay(100);
	Blynk.virtualWrite(BLYNKPORT_HVBatLevel, adc.GetHVLevel());
	delay(100);
	Blynk.virtualWrite(BLYNKPORT_HVBatCurrent, adc.GetHVCurrent());
	Blynk.setProperty(BLYNKPORT_Check, "offLabel", "Check");
	
}


// these lines go at the very bottom of BlynkManager.cpp:
// define the instance
BlynkManager *BlynkManager::instance;

// define Blynk here (copied from BlynkSimpleEsp32.h)
#ifdef GSM
	BlynkSIM Blynk(_blynkTransport);
#elif
	BlynkWifi Blynk(_blynkTransport);
#endif
// define all widgets here
WidgetLED RelayLED(V0);
WidgetTerminal terminal(V10);
WidgetLCD lcd(BLYNKPORT_LCD);
WidgetLED motionLed(BLYNKPORT_MotionLED);
