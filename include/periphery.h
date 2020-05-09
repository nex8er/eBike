#ifndef PERIPHERY_H
#define PERIPHERY_H

#include <Wire.h>
#include <OneWire.h>
#include <MPU6050.h>
#include <NeoPixelBus.h>
#include <Adafruit_ADS1015.h>


//-------------------------------------------------------------------------------- 
//                                  DEFINES                                     // 
//-------------------------------------------------------------------------------- 

#define PCF8574_ADDR 0x38
#define DS_TEMP_ADDR {0x28, 0x9D, 0x43, 0x79, 0x97, 0x5, 0x3, 0x36}
#define OW_TAIL_ADDR {0x20, 0x54, 0x41, 0x49, 0x4C, 0x4C, 0x49, 0x00}


#define PIN_MODEM_PWR 4
#define ONE_WIRE_BUS 13

#define PIN_AC_OFF 0
#define PIN_LVCHG_OFF 1
#define PIN_PP5V_EN 2
#define PIN_ALARM 3

#define ACIN_ON PortExp.write(PIN_AC_OFF, false)
#define ACIN_OFF PortExp.write(PIN_AC_OFF, true)
#define LVCHG_ON PortExp.write(PIN_LVCHG_OFF, false)
#define LVCHG_OFF PortExp.write(PIN_LVCHG_OFF, true)
#define PP5V_ON PortExp.write(PIN_PP5V_EN, false)
#define PP5V_OFF PortExp.write(PIN_PP5V_EN, true)
#define ALARM_ON PortExp.write(PIN_ALARM, true)
#define ALARM_OFF PortExp.write(PIN_ALARM, false)

#define L     0x00
#define R     0x01
#define Both  0x10

//------------------------------------------------------------------------------//

extern OneWire owi;
extern MPU6050 mpu;


extern const unsigned char taillight_addr[8];

void SetAccelerometer(void);
void SetADC(void);
void PCF8574_control(uint8_t channel, bool param);

void siren(bool param);

void SetPixels(void);
void PixelsSet(uint8_t l, uint8_t r, uint8_t g, uint8_t b);
void PixelsClear(uint8_t l = Both);

int8_t GetBatTemp(void);

void SetAllPeriphery (void);

class PCF8574 {
	public:
		PCF8574(const uint8_t _addr);
		void write(const uint8_t port, const bool state);

		uint8_t DirectRead(void) const;
		void DirectWrite(const uint8_t& _buffer);
	private:
		uint8_t addr;
		uint8_t buffer;
};

extern PCF8574 PortExp;

class ADC {
	public:
		ADC();

		void Init(void);
		float GetHVLevel(void);
		float GetLVLevel(void);
		float GetACLevel(void);
		float GetHVCurrent(void);
		void UpdateAll(const uint8_t sample = 0);
		void GetCurrentOffset(void);

	private:
		float GetVoltage(uint8_t channel, float ratio, uint8_t sample = 8);
		float HVBatLevel, LVBatLevel, ACLevel, HVCurrent;
		float currentOffset;
		Adafruit_ADS1115 ads;
};

extern ADC adc;

#endif