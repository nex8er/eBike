#include "wifi.h"
#include "utils.h"

const int wdtTimeout = 100000;  //time in ms to trigger the watchdog
hw_timer_t *wdtTimer = NULL;


void IRAM_ATTR resetModule() {
	ets_printf("WDT Reset\n");
	esp_restart();
}

// настройка вочдог таймера
void SetWDT() {
	wdtTimer = timerBegin(0, 80, true);                  //timer 0, div 80
	timerAttachInterrupt(wdtTimer, &resetModule, true);  //attach callback
	timerAlarmWrite(wdtTimer, wdtTimeout * 1000, false); //set time in us
	timerAlarmEnable(wdtTimer);                          //enable interrupt
}

	// add to loop
void ClearWDT() {  
	timerWrite(wdtTimer, 0);  //reset timer (feed watchdog)
	//long loopTime = millis();
}


void Log(const char *msg, log_type lt) {
	#ifdef LOG_TO_SERIAL
		Serial.print("[LOG] ");
		Serial.write(msg);
		Serial.print("\n");
	#endif

	const char *path = "/log";
	String str = "[" + GetDateTime() +  "] ";
	switch (lt) {
		case I:
			lt = INFO;
			break;
		case D:
			lt = DEBUG;
			break;
		case W:
			lt = WARNING;
			break;
		case E:
			lt = ERROR;
			break;
		default:
			;
	}
	switch (lt) {
		case INFO:
			str += "[INFO]    ";
			break;
		case DEBUG:
			str += "[DEBUG]   ";
			break;
		case WARNING:
			str += "[WARNING] ";
			break;
		case ERROR:
			str += "[ERROR]   ";
			break;
		default:
			str += "[]        ";
	}
	size_t free = SPIFFS.totalBytes() - SPIFFS.usedBytes();

	if (free < 1024) {
		Serial.printf("LOG: no free space\n");
		SPIFFS.remove(path);
	}

	
	File f = SPIFFS.open(path, "a+");
	if (!f) {
		Serial.print("LOG: can't open log file\n");
		return;
	}

	f.print(str.c_str());
	f.print(msg);
	f.print("\n");
	f.close();

}
void Logf(log_type lt, const char *msg, ...) {
	char buf[256];
	va_list ap; va_start(ap, msg);
	size_t n = vsnprintf(buf, sizeof(buf), msg, ap);
	va_end(ap);
	if(n >= sizeof(buf))
		; // handle buffer overflow
	Log(buf, lt);
}
const String GetDateTime() {
	char buf[20];
	snprintf(buf, sizeof(buf),"%02u/%02u/%u %02u:%02u:%02u", day(), month(), year(), hour(), minute(), second());
	const String s(buf);
	return s;
}


bool ConfigMgr::Load() {
	File configFile = SPIFFS.open("/config.json", "r");
	if (!configFile) {
		Log("Unable load config file");
		return false;
	}
	
	const size_t capacity = JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(6) + 90;
	DynamicJsonDocument doc(capacity);

	deserializeJson(doc, configFile);

	config.wifi_ssid = doc["wifi"][currentWiFi]["ssid"];
	config.wifi_pass = doc["wifi"][currentWiFi]["pass"]; 

	config.onGuard = doc["onGuard"]; 
	config.alarmMode = doc["alarmMode"];
	config.GPS = doc["GPS"];
	config.headLight = doc["headLight"];
	config.tailLight = doc["tailLight"];

	return true;
}

bool ConfigMgr::Save() {
	File configFile = SPIFFS.open("/config.json", "w");
	if (!configFile) {
		Log("Unable open config file for write");
		return false;
	}

	const size_t capacity = 3*JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(6);
	DynamicJsonDocument doc(capacity);

	JsonObject wifi = doc.createNestedObject("wifi");

	JsonObject wifi_cfg = wifi.createNestedObject(String(currentWiFi));

	wifi_cfg["ssid"] = config.wifi_ssid;
	wifi_cfg["pass"] = config.wifi_pass;
	doc["onGuard"] = config.onGuard;
	doc["alarmMode"] = config.alarmMode;
	doc["GPS"] = config.GPS;
	doc["headLight"] = config.headLight;
	doc["tailLight"] = config.tailLight;

	serializeJson(doc, configFile);

	return true;
}
