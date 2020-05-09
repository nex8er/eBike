#define TINY_GSM_MODEM_SIM808
#define GSM_Serial Serial

#include <Arduino.h>
#include <modem.h>
#include "utils.h"
#include "periphery.h"


const char apn[] = "internet.danycom.ru";


TinyGsm modem(Serial);
TinyGsmClient uploadClientGSM(modem, 2);		// обозначаем клиента HTTP для загрузки изображения. через модем, номер подключения(MUX)

void SetModem() {


	Log("Initializing modem...");
	
	PixelsSet(L,100,0,0);
	bool modemState = modem.restart();
	PixelsSet(L, 100,30,0);
	if (modemState) {
		Log("Modem: ");
		Log(modem.getModemInfo().c_str());
		Log("Waiting for network...");
		PixelsSet(L, 220,50,0);
		if (!modem.waitForNetwork(20000L)) {
			Log("Unable connect to network");
			modem.poweroff();
			delay(1000);
			PixelsSet(L, 190,0,150);
			Log("Modem power off");
			delay(10000);
			
			Log("Restart!");
			ESP.restart();
			return;
		}

		//modem.autoAnswer(true);  // Автоответ
		PixelsSet(L, 190,100,0);

		Log("Connecting to ");
		Log(apn);
		if (!modem.gprsConnect(apn, "", "")) {
			Log("Unable connect to APN");
			modem.poweroff();
			delay(45000);
			Log("Restart!");
			ESP.restart();
			return;
		}
		PixelsSet(L, 190,220,0);
	}
	else {
		Log("Unable init modem");
		PixelsSet(L, 190,0,150);
		
		Log("Restart!");
		ESP.restart();
		return;
	}
}

void ModemReconnect() {
	modem.gprsDisconnect();
	modem.poweroff();
	PixelsSet(L, 100, 10, 100);
	delay(20000);
	PixelsClear();
	modem.restart();
	delay(5000);
	PixelsSet(L,100,0,0);
	PixelsSet(L, 100,30,0);
		Log("Modem: ");
		Log(modem.getModemInfo().c_str());
		Log("Waiting for network...");
		PixelsSet(L, 220,50,0);
		if (!modem.waitForNetwork(20000L)) {
			Log("Unable connect to network");
			modem.poweroff();
			delay(1000);
			PixelsSet(L, 190,0,150);
			Log("Modem power off");
			delay(10000);
			
			Log("Restart!");
			ESP.restart();
			return;

			PixelsSet(L, 190,100,0);

		Log("Connecting to ");
	if (!modem.gprsConnect(apn, "", "")) {
			Log("Unable connect to APN");
			modem.poweroff();
			delay(45000);
			Log("Restart!");
			ESP.restart();
			return;
		}
		PixelsSet(L, 190,220,0);
	}
	else {
		Log("Unable init modem");
		PixelsSet(L, 190,0,150);
		
		Log("Restart!");
		ESP.restart();
		return;
	}
}