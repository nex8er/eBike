#ifndef CAMERA_H
#define CAMERA_H

#include <Arduino.h>
#include <esp_camera.h>
//#include "define.h"
//#include "modem.h"
// #include "utils.h"
#include "tasksched.h"
#include "private.h"

#define UPLOAD_CATALOG  "p"
#define BOUNDARY "----------------------------133747188241686651551404"

#define MTU 					1440
#define IMAGE_UPLOAD_PORT		80
#define UPLOAD_TIMEOUT			10000

// #define L 0
// #define R 1
#define LEFT_CAM 0
#define RIGHT_CAM 1

extern String imageUpload_result;
extern bool imgUploadTimeout;						// флаг того что изобр. было передано, но ответ от сервера не был получен
extern bool flagCamOK;


bool SetupCamera(void);
bool TakePhoto(void);
// bool switchCamera(bool selectCamera);
// String imageUpload(void);
void imageButtonProc(void);

#endif