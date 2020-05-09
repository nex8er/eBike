#ifndef NET_H
#define NET_H

#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <RemoteDebug.h>
#include <ESPmDNS.h>
#include <WebServer.h>
#include <WiFiClient.h>

#include "private.h"



extern WebServer server;
extern RemoteDebug Debug;

void SetWiFi(void);
void SetRemoteDebug(void);

void SetMDNS(void);
//void InitServer(void);

void SetOTA(void);

void InitServer(void);

#endif