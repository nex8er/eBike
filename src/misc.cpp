#include "misc.h"
#include "modem.h"
#include "blynk_manager.h"

void NetworkInfo() {
	int csqProc;
	int csq = modem.getSignalQuality();
	String ops = modem.getOperator();
	char text[3];
	csqProc = map(csq, 0, 32, 1, 99);
	snprintf(text,sizeof(text), "%u", csqProc);
	lcd.print(0, 1, "Net: ");
	lcd.print(7, 1, ops);
	lcd.print(13, 1, text);
	lcd.print(15, 1, "%");

}