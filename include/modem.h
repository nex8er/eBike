#ifndef MODEM_H
#define MODEM_H

#define TINY_GSM_MODEM_SIM808
#define GSM_Serial Serial

#include <Arduino.h>
#include <TinyGsmClient.h>

extern TinyGsm modem;
extern TinyGsmClient uploadClientGSM;		// обозначаем клиента HTTP для загрузки изображения. через модем, номер подключения(MUX)

void SetModem(void);
void ModemReconnect(void);

#endif