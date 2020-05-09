// #include "periphery.h"


// BLYNK_WRITE(BLYNKPORT_IndicatorMode) {
// 	switch (param.asInt())
// 	{
// 	case 0:
// 		pixels.SetPixelColor(0, black);
// 		pixels.SetPixelColor(1, black);
// 		pixels.Show();
// 		break;
// 	case 1:
// 		pixels.SetPixelColor(0, (255,255,255));
// 		pixels.SetPixelColor(1, green);
// 		pixels.Show();
// 		break;	
// 	case 2:
// 		pixels.SetPixelColor(0, blue);
// 		pixels.SetPixelColor(1, red);
// 		pixels.Show();
// 		break;	
// 	case 3:
// 		pixels.SetPixelColor(0, hslRed);
// 		pixels.SetPixelColor(1, hslGreen);
// 		pixels.Show();
// 		break;	
// 	case 4:
// 		pixels.SetPixelColor(0, hslBlack);
// 		pixels.SetPixelColor(1, hslWhite);
// 		pixels.Show();
// 		break;
// 	case 5:
// 		//batteryTemp();
// 		break;
// 	default:
// 		break;
// 	}
// }