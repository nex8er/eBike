#include <net.h>
#include "utils.h"
#include "blynk_manager.h"
#include "camera.h"
#include "esp_camera.h"

#define DBGPRT Serial

CFG config;

WebServer server(80);

// const char* ssid = config.wifi_ssid;
// const char* password = config.wifi_pass;
const char* host = "ebike";
const char* ssid = STASSID;
const char* password = STAPSK;
const char* ssidAP = "ebike";
const char* passwordAP = STAPSK;
IPAddress ipAP(192, 168, 4, 1); 


void SetWiFi() {
	uint8_t WiFiAttempts = 0;
	DBGPRT.print("Try connecting to WiFi network ");
	DBGPRT.println(ssid);
	WiFi.mode(WIFI_MODE_STA);                            // подключение к WiFi
	WiFi.begin(ssid,password);
	while (WiFi.status() != WL_CONNECTED && WiFiAttempts < 20) {
		delay(500);
		DBGPRT.print(".");
		WiFiAttempts++;
	}
	if (WiFi.status() == WL_CONNECTED) 
	{
		DBGPRT.print("\nWiFi connected\nIP Address: ");
		DBGPRT.println(WiFi.localIP());
	}
	if (WiFiAttempts >= 20 || WiFi.status() != WL_CONNECTED) {
		DBGPRT.println("\nConnection failed! Creating AP...");
		WiFi.mode(WIFI_MODE_AP);
		//WiFi.softAPConfig(ipAP, NULL, NULL);
		if (WiFi.softAP(ssidAP, passwordAP)) {
			DBGPRT.print("WiFi AP created: ");
			WiFi.softAPsetHostname(host);
			DBGPRT.println(ssidAP);
			DBGPRT.print("IP Address: ");
			DBGPRT.println(WiFi.softAPIP());
		}
		else {
			DBGPRT.println("WiFi AP create failed");
		}
	}
}


void SetRemoteDebug() {
	Debug.begin("ebike");
	
	Debug.setResetCmdEnabled(true); // Enable the reset command
	Debug.showProfiler(false);
	Debug.showDebugLevel(false); // Profiler (Good to measure times, to optimize codes)
	Debug.showColors(true); // Colors
}
void SetMDNS() {
	if (!MDNS.begin("ebike")) {
		Log("Error setting up MDNS responder!");
	}
	MDNS.addService("http", "tcp", 80);
	MDNS.addService("telnet", "tcp", 23);
	MDNS.addService("ota", "tcp", 3232);
}
void SetOTA() {
	// ArduinoOTA.setPort(3232);
	ArduinoOTA.setHostname("ebike");

	ArduinoOTA.onStart([]() {
			runner.disableAll();
			String type;
			if (ArduinoOTA.getCommand() == U_FLASH)
				type = "sketch";
			else // U_SPIFFS
				type = "filesystem";

			// NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
			Serial.println("Start updating " + type);
		});
	ArduinoOTA.onEnd([]() {
			Serial.println("\nEnd");
		});
	ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
			Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
		});
	ArduinoOTA.onError([](ota_error_t error) {
			Serial.printf("Error[%u]: ", error);
			if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
			else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
			else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
			else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
			else if (error == OTA_END_ERROR) Serial.println("End Failed");
		});

	ArduinoOTA.begin();
	
}

void InitServer() {
	String content_done = "Done!<br />";
	content_done += "<a href=\"/\">Back</a>";;

	server.on("/log", [](){
		File f = SPIFFS.open("/log", "r");
		server.streamFile(f,"text/plain");
	});
	server.on("/", [](){
		size_t free = SPIFFS.totalBytes() - SPIFFS.usedBytes();
		String content = "Total space: " + String(SPIFFS.totalBytes()) + "<br />";
		content += "Used space: " + String(SPIFFS.usedBytes()) + "<br />";
		content += "Free space: " + String(free) + "<br />";
		content += "Free heap: " + String(ESP.getFreeHeap()) + "<br />";
		if (Blynk.connected()) {
			content += "Blynk connected";
		} else {
			content += "Blynk NOT connected";
		}
		content += "<br />";
		//content += "VCC: " + String(ESP.getVcc()) + "V<br />";
		content += "<a href=\"/log\">Open Log</a><br />";
		content += "<a href=\"/test\">Test Log</a><br />";
		content += "<a href=\"/logdel\">Clear Log</a><br />";
		content += "<br />";
		content += "<a href=\"/cam\">Camera</a><br />";
		content += "<br />";
		content += "<a href=\"/restart\">ESP Restart</a><br />";
		
		server.send(200, "text/html", content);
	});
	server.on("/logdel", [content_done](){
		String content;
		if (SPIFFS.remove("/log")) {
			server.send(200, "text/html", content_done);
		}
		
	});
	server.on("/restart", [content_done](){
		server.sendHeader("Location", String("/"), true);
		server.send (302, "text/plain", "");
		ESP.restart();
	});
	server.on("/test", [](){
		Logf(W, "Test record");
		server.sendHeader("Location", String("/"), true);
		server.send (302, "text/plain", "");
		
	
	});
	server.on("/cam", [](){
		String content = "<a href=\"/init_cam\">Init Cam</a><br />";
		content += "<a href=\"/photo\">Take Photo</a><br />";
		content += "<br />";
		content += "<a href=\"/\">Back</a><br />";
		server.send(200, "text/html", content);
	});
	server.on("/init_cam", [](){
		String content;
		if (SetupCamera()) 
			content += "Camera init OK";
		else
			content += "Camera FAIL!";

		content += "<br />";
		content += "<a href=\"/\">Back</a><br />";
		server.send(200, "text/html", content);
	});
	server.on("/photo", [](){
		String content;
		if(!TakePhoto()) {
			content += "<a href=\"/\">Fail</a><br />";
			server.send(200, "text/html", content);
		}
	});
	
	server.begin();
}
