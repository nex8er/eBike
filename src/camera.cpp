
#include <Arduino.h>
#include "camera.h"
#include "define.h"
#include "blynk_defines.h"
#include "blynk_manager.h"
//#include "modem.h"
#include "utils.h"
#include "tasksched.h"
#include "esp_camera.h"
#include "camera_index.h"
#include "esp_timer.h"
//#include "time.h"

WiFiClient uploadClient;
Task tImageButtonProc(100, TASK_ONCE, &imageButtonProc, &runner, false);

String imageUpload_result;
bool imgUploadTimeout;						// флаг того что изобр. было передано, но ответ от сервера не был получен
bool flagCamOK = false;

bool SetupCamera() {		// первичная настройка камеры

	pinMode(PIN_CAM_SEL, OUTPUT);
	pinMode(PIN_CAM_PWR, OUTPUT);
	digitalWrite(PIN_CAM_SEL, HIGH);
	digitalWrite(PIN_CAM_PWR, LOW);

	camera_config_t config;									// жесткая настройка (пины, буфер)
	config.ledc_channel = LEDC_CHANNEL_0;
	config.ledc_timer = LEDC_TIMER_0;
	config.pin_d0 = 5;
	config.pin_d1 = 18;
	config.pin_d2 = 19;
	config.pin_d3 = 21;
	config.pin_d4 = 36;
	config.pin_d5 = 39;
	config.pin_d6 = 34;
	config.pin_d7 = 35;
	config.pin_xclk = 0;
	config.pin_pclk = 22;
	config.pin_vsync = 25;
	config.pin_href = 23;
	config.pin_sscb_sda = 26;
	config.pin_sscb_scl = 27;
	config.pin_pwdn = 32;
	config.pin_reset = -1;
	config.xclk_freq_hz = 20000000;
	config.pixel_format = PIXFORMAT_JPEG;
	//init with high specs to pre-allocate larger buffers
	config.frame_size = FRAMESIZE_UXGA;
	config.jpeg_quality = 1;
	config.fb_count = 1;
	
	delay(100);
	
	esp_err_t cam_err = esp_camera_init(&config);           // инициализация камеры
	if (cam_err != ESP_OK) {
		Logf(E, "Camera init failed with error 0x%x", cam_err);
		return false;
	}
	else {
		Log("Camera init complite");
		sensor_t * cam_set = esp_camera_sensor_get();            // настройка камеры
		cam_set->set_framesize(cam_set, FRAMESIZE_VGA);          // размер кадра
		cam_set->set_pixformat(cam_set, PIXFORMAT_JPEG);         // формат 
		cam_set->set_quality(cam_set, 14);                       // степень сжатия jpeg 1-62 
		//reinit_dma();
		//drop down frame size for higher initial frame rate

		//cam_set->set_vflip(s, 1);

		return true;
	}
	{ //возможные настроийки камеры
		//sensor_t * s = esp_camera_sensor_get();
		// s->set_brightness(s, 0);     // -2 to 2
		// s->set_contrast(s, 0);       // -2 to 2
		// s->set_saturation(s, 0);     // -2 to 2
		// s->set_special_effect(s, 0); // 0 to 6 (0 - No Effect, 1 - Negative, 2 - Grayscale, 3 - Red Tint, 4 - Green Tint, 5 - Blue Tint, 6 - Sepia)
		// s->set_whitebal(s, 1);       // 0 = disable , 1 = enable
		// s->set_awb_gain(s, 1);       // 0 = disable , 1 = enable
		// s->set_wb_mode(s, 0);        // 0 to 4 - if awb_gain enabled (0 - Auto, 1 - Sunny, 2 - Cloudy, 3 - Office, 4 - Home)
		// s->set_exposure_ctrl(s, 1);  // 0 = disable , 1 = enable
		// s->set_aec2(s, 0);           // 0 = disable , 1 = enable
		// s->set_ae_level(s, 0);       // -2 to 2
		// s->set_aec_value(s, 300);    // 0 to 1200
		// s->set_gain_ctrl(s, 1);      // 0 = disable , 1 = enable
		// s->set_agc_gain(s, 0);       // 0 to 30
		// s->set_gainceiling(s, (gainceiling_t)0);  // 0 to 6
		// s->set_bpc(s, 0);            // 0 = disable , 1 = enable
		// s->set_wpc(s, 1);            // 0 = disable , 1 = enable
		// s->set_raw_gma(s, 1);        // 0 = disable , 1 = enable
		// s->set_lenc(s, 1);           // 0 = disable , 1 = enable
		// s->set_hmirror(s, 0);        // 0 = disable , 1 = enable
		//s->set_vflip(s, 0);          // 0 = disable , 1 = enable
		// s->set_dcw(s, 1);            // 0 = disable , 1 = enable
		// s->set_colorbar(s, 0);       // 0 = disable , 1 = enable
	}
}

bool TakePhoto() {
	camera_fb_t *frameBuffer = NULL;
	frameBuffer = esp_camera_fb_get();
	if (!frameBuffer) {
		Log("Camera capture failed", E);
		return false;
	}
	return true;

}
// bool switchCamera(bool selectCamera) {
// 	if (flagCamOK) {  //?
// 		esp_err_t cam_err = esp_camera_deinit();      //Deinitialize cam
// 		if (cam_err != ESP_OK) {
// 			Log("Camera deinitialize failed with error 0x%x" + cam_err);
// 			Blynk.setProperty(BLYNKPORT_CamCaptureBtn, "offLabel", "❗️");
// 			flagCamOK = false;
// 			return false;
// 		}
// 		else {
// 			Log("Camera deinit");
// 		}
// 	}
// 	digitalWrite(PIN_CAM_PWR, HIGH);
	
// 	if (selectCamera == LEFT_CAM) {
// 		digitalWrite(PIN_CAM_SEL, LOW);
// 	}
// 	if (selectCamera == RIGHT_CAM) {
// 		digitalWrite(PIN_CAM_SEL, HIGH);
// 	}
// 	delay(500);

// 	Log("cam preset done");

// 	if (setupCamera()) {
// 		flagCamOK = true;
// 		Blynk.setProperty(BLYNKPORT_CamCaptureBtn, "offLabel", "📷");
// 		Blynk.syncVirtual(BLYNKPORT_CamQuality);  //?
// 		return true;
// 	}
// 	else {
// 		Blynk.setProperty(BLYNKPORT_CamCaptureBtn, "offLabel", "❗");
// 		return false;
// 	}
// }
String imageUpload() {						// отправка фотографии на сервер   
	char status[64] = {0};
	char response[32] = {0};
	char buf[1024];
	char fileName[20];
	char imageRemoteAddress[64] = {0};
	//struct tm timeinfo;
	imgUploadTimeout = false;
	long tOut;

	Log("Start upload image");
	camera_fb_t *frameBuffer = NULL;                                // структура для буфера кадра
	frameBuffer = esp_camera_fb_get();                              // магия. заполняем буфер с камеры
	delay(500);
	frameBuffer = esp_camera_fb_get();								// делаем два раза чтобы был правильный баланс белого
	if (!frameBuffer) {
		Log("Camera capture failed", E);
		return "Camera fail";
	}
	Log("Camera: fb_ok");
	size_t frameLen = frameBuffer->len;

	terminal.printf("Start sending JPG: %uB\n", (uint32_t)(frameBuffer->len));
	terminal.flush();
	
	if (!uploadClient.connect(IMAGE_UPLOAD_SERVER, IMAGE_UPLOAD_PORT)) {
		Logf(E, "Unable to connect to %s", String(IMAGE_UPLOAD_SERVER));
		return "Connect fail";   
	}

	// HTTP запрос. содержет в себе имя файла загружаемого на сервер
	char request_content[] = "----------------------------133747188241686651551404\r\n"
							"Content-Disposition: form-data; name=\"imageFile\"; filename=\"%s.jpg\"\r\n"
							"Content-Type: image/jpeg\r\n\r\n";
	char request_end[] = "\r\n----------------------------133747188241686651551404--\r\n\r\n";


	if (year() == 1970) {
		Log("Failed to obtain time");
		snprintf(fileName, sizeof(fileName), "%s", String(millis()).c_str());
	}
	else {
		snprintf(fileName, sizeof(fileName), "%02u-%02u-%02u_%02u-%02u-%02u", hour(), minute() ,second(),day(), month(), year());
	}

	snprintf(buf, sizeof(buf), request_content, fileName);
	snprintf(imageRemoteAddress, sizeof(imageRemoteAddress), "http://%s/%s/%s.jpg",IMAGE_UPLOAD_SERVER, UPLOAD_CATALOG, fileName);

	
	uint32_t content_len =  frameLen + strlen(buf) + strlen(request_end);   // суммарный размер всего запроса вместе с картинкой
	// хэдер HTTP запроса
	String request = "POST /index.php HTTP/1.1\r\n";
	request += "cache-control: no-cache\r\n";
	request += "Content-Type: multipart/form-data; boundary=--------------------------133747188241686651551404";
	request += "\r\n";
	request += "User-Agent: PostmanRuntime/6.4.1\r\n";
	request += "Accept: */*\r\n";
	request += "Host: " + String(IMAGE_UPLOAD_SERVER);
	request += "\r\n";
	request += "accept-encoding: gzip, deflate\r\n";
	request += "Connection: keep-alive\r\n";
	request += "content-length: " + String(content_len) + "\r\n";
	request += "\r\n";

	uploadClient.print(request+buf);

	Debug.printf("Start sending JPG: %uB\n", (uint32_t)(frameBuffer->len));
	
	// ĐžŃĐżŃĐ°Đ˛ĐşĐ° JPG Đ˝Đ° ŃĐľŃĐ˛ĐľŃ
	uint8_t *image = frameBuffer->buf;
	size_t size = frameBuffer->len;
	size_t offset = 0;
	size_t ret = 0;
	while (1) {          
		ret = uploadClient.write(image + offset, size);
		offset += ret;
		size -= ret;
		if (frameBuffer->len == offset)
		{
			break;
		}
	}

	Serial.printf("upload end - %u\n", uploadClient.print(request_end));

	delay(20);
	tOut = millis() + UPLOAD_TIMEOUT;
	
	while(uploadClient.connected() && tOut > millis())        // ĐśĐ´ĐľĐź ĐžŃĐ˛ĐľŃĐ° ŃĐľŃĐ˛ĐľŃĐ°. ĐżĐž ŃĐ°ĐšĐźĐ°ŃŃŃ 20 ŃĐľĐş.
	{
		if (uploadClient.available()) 
		{
			uploadClient.readBytesUntil('\r', response, sizeof(status));
			if (strcmp(response, "HTTP/1.1 200 OK") != 0)  // ĐżŃĐžĐ˛ĐľŃŃĐľĐź Đ˝Đ° ĐžŃĐ˛ĐľŃ ŃĐľŃĐ˛ĐľŃĐ°.
			{
				Debug.print("Unexpected response: ");
				Debug.println(response);              // Đ˝ĐľŃĐ°ŃĐżĐžĐˇĐ˝Đ°Đ˝Đ˝ŃĐš ĐžŃĐ˛ĐľŃ
				Debug.stop();
				return ("Unexpected response");
			}
			Debug.print("Server responce: ");
			Debug.println(response);
			uploadClient.stop();
			return String(imageRemoteAddress);                                 // Đ˛ŃĐľ ŃĐžŃĐžŃĐž. ĐžŃĐ˛ĐľŃ HTTP/200
				
		}
	}
	Debug.println("Send image timeout");
	uploadClient.stop();
	return("timeout");        
}



void imageButtonProc() {					// для отправки ссылки в галлерею, и установки значка кнопки
	if (!Blynk.connected()) {
		//tImageButtonProc.restartDelayed();
		return;
	}
	if (imageUpload_result == "timeout" || imageUpload_result == "Unexpected response"
		||  imageUpload_result ==  "Connect fail" ||  imageUpload_result ==  "Camera fail"){
		Blynk.setProperty(BLYNKPORT_CamCaptureBtn, "offLabel", "⚠️");
		Blynk.setProperty(BLYNKPORT_CamImageWidget, "label", "Upload failed. " + imageUpload_result);
	}
	else {
		delay(100);
		Blynk.setProperty(V0, "url", 1, imageUpload_result);
		Blynk.setProperty(V0, "label", imageUpload_result);
		Blynk.virtualWrite(V0,1);
		Blynk.setProperty(BLYNKPORT_CamCaptureBtn, "offLabel", "📷");
	}	
}
BLYNK_WRITE(BLYNKPORT_CamCaptureBtn) {		// захват и передача изобр.
	if(!param.asInt())
		return;

	Blynk.setProperty(BLYNKPORT_CamCaptureBtn, "offLabel", "📸");
	delay(50);
	imageUpload_result = imageUpload();
	//BlynkReconnect();
	tImageButtonProc.restartDelayed();	
}
BLYNK_WRITE(BLYNKPORT_CamQuality) {			// настройка качества изобр.
	char label[40];
	String res;
	uint8_t quality;
	int typSize;
	char buf[5];
	sensor_t * cam_set = esp_camera_sensor_get();            // настройка камеры
	
	switch (param.asInt()) {
	case 1:
		cam_set->set_framesize(cam_set, FRAMESIZE_QVGA);          // размер кадра
		cam_set->set_quality(cam_set, 24);                       // степень сжатия jpeg 1-62 
		res = "QVGA"; quality = 24; typSize = 3;
		break;
	case 2:
		cam_set->set_framesize(cam_set, FRAMESIZE_QVGA);          // размер кадра
		cam_set->set_quality(cam_set, 16);                       // степень сжатия jpeg 1-62 
		res = "QVGA"; quality = 16; typSize = 5;
		break;
	case 3:
		cam_set->set_framesize(cam_set, FRAMESIZE_VGA);          // размер кадра
		cam_set->set_quality(cam_set, 18);                       // степень сжатия jpeg 1-62 
		res = "VGA"; quality = 18; typSize = 13;
		break;
	case 4:
		cam_set->set_framesize(cam_set, FRAMESIZE_VGA);          // размер кадра
		cam_set->set_quality(cam_set, 12);                       // степень сжатия jpeg 1-62 
		res = "VGA"; quality = 14; typSize = 17;
		break;
	case 5:
		cam_set->set_framesize(cam_set, FRAMESIZE_XGA);          // размер кадра
		cam_set->set_quality(cam_set, 20);                       // степень сжатия jpeg 1-62 
		res = "XGA"; quality = 20; typSize = 27;
		break;
	case 6:
		cam_set->set_framesize(cam_set, FRAMESIZE_XGA);          // размер кадра
		cam_set->set_quality(cam_set, 14);                       // степень сжатия jpeg 1-62
		res = "XGA"; quality = 16; typSize = 33; 
		break;
	case 7:
		cam_set->set_framesize(cam_set, FRAMESIZE_XGA);          // размер кадра
		cam_set->set_quality(cam_set, 12);                       // степень сжатия jpeg 1-62 
		res = "XGA"; quality = 12; typSize = 36;
		break;
	case 8:
		cam_set->set_framesize(cam_set, FRAMESIZE_SXGA);          // размер кадра
		cam_set->set_quality(cam_set, 16);                       // степень сжатия jpeg 1-62 
		res = "SXGA"; quality = 16; typSize = 47;
		break;
	case 9:
		cam_set->set_framesize(cam_set, FRAMESIZE_UXGA);          // размер кадра
		cam_set->set_quality(cam_set, 16);                       // степень сжатия jpeg 1-62 
		res = "UXGA"; quality = 16; typSize = 67;
		break;
		
	default:		
		res = "????"; quality = 0; typSize = 0;
		break;
	} 
	res.toCharArray(buf, sizeof(buf));
	snprintf(label,sizeof(label), "Photo Quality: %s, %u, ~size: %uk", buf, quality, typSize);
	Blynk.setProperty(BLYNKPORT_CamQuality, "label", label);

	return;
}
// BLYNK_WRITE(BLYNKPORT_CamSwitchL) {
// 	if (!param.asInt())
// 		return;
	
// 	if (switchCamera(LEFT_CAM)) {
// 		Blynk.setProperty(BLYNKPORT_CamSwitchL, "offBackColor", "BLYNK_WHITE");
// 		Blynk.setProperty(BLYNKPORT_CamSwitchR, "offBackColor", "BLYNK_BLUE");
// 	}
// }
// BLYNK_WRITE(BLYNKPORT_CamSwitchR) {
// 	if (!param.asInt())
// 		return;
	
// 	if (switchCamera(RIGHT_CAM)) {
// 		Blynk.setProperty(BLYNKPORT_CamSwitchR, "offBackColor", "BLYNK_WHITE");
// 		Blynk.setProperty(BLYNKPORT_CamSwitchL, "offBackColor", "BLYNK_BLUE");
// 	}
// }
