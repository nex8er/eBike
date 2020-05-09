#include <Arduino.h>
//#include "modem.h"
#include "net.h"
#include "utils.h"
#include "blynk_manager.h"
#include "periphery.h"
#include "tasksched.h"
#include <WidgetRTC.h>
#include "camera.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>


#define BAUDRATE 115200



WidgetRTC rtc;
RemoteDebug Debug;
extern WebServer server;


//LiquidCrystal_I2C lcd(0x3F,20,4); 

void setup() {

	WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector

	Serial.begin(BAUDRATE);
	SPIFFS.begin(true);
	SetWDT();
	Log("EBike ESP-controller started!");


	runner.startNow();
	//Config.Load();



	SetWiFi();
	SetRemoteDebug();
	SetOTA();
	SetMDNS();
	InitServer();

	//SetupCamera();

	SetAllPeriphery();

	ALARM_OFF;

	ACIN_ON;
	LVCHG_ON;
	PP5V_OFF;
	// delay(300);
	// ALARM_OFF;

	BlynkManager blynkManager;
	blynkManager.Begin(&blynkManager);

	//      ???
	// if(Blynk_.Connect()) {
	//   Log("Blynk connect");
	// }

	Blynk.notify("E-Bike started!");
	// PixelsClear();
}

void loop() {
	//Blynk.run();
	Debug.handle();
	runner.execute();
	server.handleClient();
	ArduinoOTA.handle();
	ClearWDT();
}

BLYNK_CONNECTED() {
	rtc.begin();
	delay(1000);
	terminal.clear();
	PixelsClear();
}
BLYNK_DISCONNECTED() {
	PixelsSet(L, 30, 40, 10);
}
