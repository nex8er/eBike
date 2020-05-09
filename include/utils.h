#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>
#include "esp_system.h"
#include "soc/soc.h"           // Disable brownout problems
#include "soc/rtc_cntl_reg.h"  // Disable brownout problems
#include <stdlib.h>
#include "tasksched.h"
#include <TimeLib.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>

#define P Serial.printf
#define LOG_TO_SERIAL

extern const int wdtTimeout;
extern hw_timer_t *wdtTimer;

void SetWDT(void);
void ClearWDT(void);

enum log_type{
	INFO, I,
	DEBUG, D,
	WARNING, W,
	ERROR, E
};

struct CFG {
	const char* wifi_ssid;
	const char* wifi_pass;

	bool onGuard; // false
	int alarmMode; // 0
	bool GPS; // false
	int headLight; // 0
	int tailLight; // 0
};


void Log(const char *msg, log_type lt = log_type::INFO);
void Logf(log_type lt, const char *msg, ...);
const String GetDateTime(void);
bool LoadCfg(void);
bool SaveCfg(void);
bool NewCfg(void);

class ConfigMgr {
public:
	ConfigMgr();
	bool Load(void);
	bool GetNextWiFi(void);
	bool Save(void);
	bool New(void);

CFG config;
	
private:
	uint8_t currentWiFi = 0;
};

extern ConfigMgr Config;

#endif