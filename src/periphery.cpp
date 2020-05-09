#include "periphery.h"
#include "utils.h"
#include "define.h"
//#include "blynk_manager.h"



PCF8574 PortExp(PCF8574_ADDR);
OneWire owi(ONE_WIRE_BUS);
MPU6050 mpu;
unsigned const char taillight_addr[8] = OW_TAIL_ADDR; //адрес устройства, 8 байт по протоколу
NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> pixels(2, 12);
ADC adc;


void SetAccelerometer() {
	if(mpu.begin(MPU6050_SCALE_500DPS, MPU6050_RANGE_4G)) {	
		mpu.setAccelPowerOnDelay(MPU6050_DELAY_3MS);
		mpu.setIntFreeFallEnabled(false);  
		mpu.setIntZeroMotionEnabled(false);
		mpu.setIntMotionEnabled(false);
		mpu.setDHPFMode(MPU6050_DHPF_5HZ);
		mpu.setMotionDetectionThreshold(2);
		mpu.setMotionDetectionDuration(5);
		mpu.setZeroMotionDetectionThreshold(4);
		mpu.setZeroMotionDetectionDuration(2);
	}
	else {
		Log("MPU6050 init failed");
	}
}

//-------------------------------------------------------------------------------- 
//                                  PCF8574                                     // 
//-------------------------------------------------------------------------------- 
PCF8574::PCF8574(uint8_t _addr) {
	PCF8574::addr = _addr;
}
void PCF8574::write(const uint8_t port, const bool state) {
	if(state) {
		buffer &=~(1<<port);
	}
	else {
		buffer |=(1<<port);
	}
		Wire.beginTransmission(addr);
		Wire.write(0x70);
		Wire.write(buffer);
		Wire.endTransmission();
}
uint8_t PCF8574::DirectRead() const {
	return buffer;
}
void PCF8574::DirectWrite(const uint8_t& _buffer) {
	buffer = _buffer;
	Wire.beginTransmission(addr);
	Wire.write(0x70);
	Wire.write(buffer);
	Wire.endTransmission();
}
//-------------------------------------------------------------------------------- 
//                                   Temperature                                // 
//-------------------------------------------------------------------------------- 
int8_t GetBatTemp() {
	byte data[12];
	byte addr[8] = DS_TEMP_ADDR;
	int8_t temp;

	uint8_t attempt = 5;                // попыток чтения датчика
	for (; attempt > 0; attempt--) {
		
		owi.reset();
		owi.select(addr);
		owi.write(0x44, 1);
		delay(50);
		owi.reset();
		owi.select(addr);    
		owi.write(0xBE);

		for (uint8_t i = 0; i < 9; i++) {           // we need 9 bytes
			data[i] = owi.read();
		}
		if(owi.crc8(data,8) != data[8]) {
			continue;
		}
		int16_t raw = (data[1] << 8) | data[0];
		raw = raw & ~7;  // 9 bit resolution, 93.75 ms
		temp = raw / 16;
		if (temp == 85) {
			continue;
		}
		return temp;
	}
	return -127;
}
void CheckTemp() {
	const int8_t temp = GetBatTemp();
	if (temp > 50) {
		const String msg = "Battery overheating! " + String(temp) + "°C";
		Log(msg.c_str());
	}
}
//-------------------------------------------------------------------------------- 
//                                      ADC                                     // 
//-------------------------------------------------------------------------------- 
ADC::ADC() {
	init();
}
void ADC::Init() {
	ads.setGain(GAIN_ONE);        	 // 1x gain   +/- 4.096V  1 bit = 2mV      0.125mV
	ads.begin();
	HVBatLevel = -1, LVBatLevel = -1, ACLevel = -1, HVCurrent = -1;
	currentOffset = 0;

}
float ADC::GetHVLevel() {
	const float ratio = 22.64;//22.47; // коэффициент делителя
	float v = GetVoltage(2, ratio);
	if (v > 44 || v < 29) {
		return -2;
	}
	return v;
}
float ADC::GetLVLevel() {
	const float ratio = 2.0127; // коэффициент делителя
	float v = GetVoltage(3, ratio);
	if (v > 4.4 || v < 3.0) {
		return -2;
	}
	return v;
}
float ADC::GetACLevel() {
	const float ratio = 24.82; // коэффициент делителя
	float v = GetVoltage(1, ratio);
	if (v > 44 || v < 28) {
		return -2;
	}
	return v;
}
float ADC::GetHVCurrent() {
	const float ratio = 10; // коэффициент делителя
	float a = GetVoltage(0, ratio);  //1mV = 10mA (в датчике ACS712-20A). получаем значение в АМПЕРАХ
	a = a - 25 + currentOffset;  // отнимем середину чтобы получить 0. + калбровка датчика 

	if (a > 0 && a < 0.01)		// фильтруем малые значения
		a = 0;
	if (a < 0 && a > -0.01)
		a = 0;

	return a;
}
void ADC::GetCurrentOffset() {
	const uint8_t buff = PortExp.DirectRead();
	ACIN_OFF;
	LVCHG_OFF;
	//ENGINE_OFF;
	float a = GetVoltage(0, 10);   
	currentOffset = a - 25;
	PortExp.DirectWrite(buff);
}
float ADC::GetVoltage(uint8_t channel, float ratio, uint8_t sample) {
	uint8_t wdt = 0;
	int16_t values[sample];

	
	while (sample > 0) {
		wdt++;
		if (wdt > 31) {
			return -1;
		}
		delay(20);
		int16_t buff = ads.readADC_SingleEnded(channel);
		if (buff > 0) {
			sample--;
			values[sample] = buff;
		} else {
			continue;
		}
	}
	size_t valuesLen = sizeof (values) / sizeof(values[0]);
	qsort(values, valuesLen, sizeof(values[0]), [] 
		(const void *cmp1, const void *cmp2) {
			int16_t a = *((int16_t *)cmp1);
			int16_t b = *((int16_t *)cmp2);

			return a - b;
		}
	);

	
	uint8_t goodAttmps = 0;
	uint32_t sum = 0;
	uint16_t avg = values[valuesLen>>1];
	
	for (const auto& i : values) {
		if (i < avg+80 && i > avg-80) {
			sum += i;
			goodAttmps++;
		}
	}
	if (goodAttmps == 0) {
		return -1;
	}
	avg = sum / goodAttmps;
	
	float v = avg >> 3;   // делим на 8, чтобы получить значение в mV
	v = v / 1000;	    // получаем значение в целых ВОЛЬТАХ
	v = v * ratio;    // умножаем на коэффициент делителя
	return v;
}
void ADC::UpdateAll(const uint8_t sample) {
	HVBatLevel = GetHVLevel();
	LVBatLevel = GetLVLevel();
	ACLevel = GetACLevel();
	HVCurrent = GetHVCurrent();
}

//-------------------------------------------------------------------------------- 
//                                     Pixels                                   // 
//-------------------------------------------------------------------------------- 
void SetPixels() {
	pixels.Begin();
	PixelsClear(Both);
}
void PixelsSet(uint8_t l, uint8_t r, uint8_t g, uint8_t b) {
	if (l == Both) {
		pixels.SetPixelColor(L, RgbColor(r,g,b));	
		pixels.SetPixelColor(R, RgbColor(r,g,b));
	}
	else {
		pixels.SetPixelColor(l, RgbColor(r,g,b));
	}
	pixels.Show();
}

void PixelsClear (uint8_t l) {
	if (l == Both) {
		pixels.SetPixelColor(L, RgbColor(0));	
		pixels.SetPixelColor(R, RgbColor(0));
	}
	else {
		pixels.SetPixelColor(l, RgbColor(0));
	}
	pixels.Show();
}


void SetAllPeriphery() {
	Wire.begin(15,14);
	adc.Init();
	adc.UpdateAll();
	SetAccelerometer();
	SetPixels();
}


