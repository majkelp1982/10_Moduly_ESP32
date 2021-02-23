#include "Module.h"

SH1106 display(true, OLED_RESET, OLED_DC, OLED_CS); 	// FOR SPI
Device device;
Screen screen;
HeatingModule heatingModule;
DataRead UDPdata;

OneWire oneWire(PIN_DS18B20);
DallasTemperature sensors(&oneWire);					// initialized 1-WIRE for buffers
DeviceAddress tmpAddresses[3];

//Functions
void firstScan();
void pinDef();
void warningAndAlarmsDefinition();
void readSensors();
void DALLAS18b20Read ();
void readUDPdata();
void getMasterDeviceOrder();
void getHeatingParams();

//Module specific functions
void mode();
void throttle();
void displayEvents();
void rotarySwitch();
void alarms();
void pump();
void buzzer();

//Output functions
void setUDPdata();
void statusUpdate();
void outputs();

//Variables
PinInput inCLK;
PinInput inDT;
PinInput inSW;

//HELP FUNCTIONS
void diagDALLAS18b20ReadDeviceAdresses();
void TMPWritteValuesToEEprom();
//oled
void printCenter(int offsetX, int y, String text);

//Delays
unsigned long dallasSensorReadMillis = 0;
unsigned long throttleMillis = 0;

//TMP
int tmpSensorActive = 1;

void screenPos() {
	screen.top.curr=100;
	screen.top.goal=0;
	screen.bottom.curr=100;
	screen.bottom.goal=100;
}

void module_init() {
	//EEprom Scan
	firstScan();
	//Temp
//	TMPWritteValuesToEEprom();

	//pin definitions
	pinDef();

	//O-LED Display
	display.init();
	screenPos();
//	display.flipScreenVertically();

	warningAndAlarmsDefinition();

	//Set sensors Dallas DS18B20
	sensors.begin();
	sensors.setResolution(10);
	sensors.setWaitForConversion(false);

	//Throttle
	ledcSetup(SERVO_CHANNEL, SERVO_FREQUENCY, SERVO_RESOUTION);
	ledcAttachPin(THROTTLE_PIN, SERVO_CHANNEL);

}

void firstScan() {
	// Bajty EEPROM //
	// 0 - 07		:   18b20 device address inlet
	// 08 - 15		:   18b20 device address outlet
	// 16 - 23		:   18b20 device address chimney
	// 24			:   temperature set in C

	int size = 1024;
	byte EEpromData[size];
	EEpromScan(EEpromData, size);
	Serial.println("\nFirst scan");

	// Get 18b20 device addresses
	int offset = 0;						// first byte of DS18b20 device addresses
	for (int i=0; i<3; i++) {
		Serial.printf("\nSensor[%d] ",i);
		for (int j=0; j<8; j++) {
			device.thermo[i].deviceAddress[j] = EEpromData[offset+8*i+j];
			Serial.printf("[%d]",device.thermo[i].deviceAddress[j]);
		}
	}
	device.reqTemp = EEpromData[24];
}

void pinDef() {
	//Rotary encoder
	pinMode (pinDT,INPUT_PULLUP);
	pinMode (pinCLK,INPUT_PULLUP);
	pinMode (pinSW,INPUT_PULLUP);

	pinMode (pinPUMP,OUTPUT);	digitalWrite(pinPUMP, LOW);
	pinMode (pinSPIKER,OUTPUT);	digitalWrite(pinSPIKER, LOW);
}

void warningAndAlarmsDefinition() {
	//when water is over 90 degrees
	screen.info[0].mess1 ="!!!WODA ZAGOTOWANA!!!";
	screen.info[0].mess2 ="!!!GASZENIE KOMINKA!!!";
	screen.info[0].mess3 ="";
	screen.info[0].type = ALARM;

	//when by Standby mode flow-temp is over 40 degrees turn alarm on, that nobody has trigger device to warm up
	screen.info[1].mess1 ="TEMPERATURA KOMINKA ";
	screen.info[1].mess2 ="W TRYBIE CZUWANIA";
	screen.info[1].mess3 ="!!!PRZEKROCZONA!!!";
	screen.info[1].type = ALARM;

	//WARNINGS
	//Notice to open chimney and air in-take to max by warming up
	screen.info[2].mess1 ="Rozpalanie";
	screen.info[2].mess2 ="Otworz klape";
	screen.info[2].mess3 ="powietrza i komin";
	screen.info[2].type = WARN;

	//Notice to shut chimney and air in-take by switching to normal mode
	screen.info[3].mess1 ="Rozpalanie zakonczone";
	screen.info[3].mess2 ="Zamknij klape";
	screen.info[3].mess3 ="powietrza";
	screen.info[3].type = WARN;

	//Notice to less timber. Set temperature unable to reach
	screen.info[4].mess1 ="Ogien zbyt slaby";
	screen.info[4].mess2 ="Zmienic tryb";
	screen.info[4].mess3 ="na gaszenie?";
	screen.info[4].type = WARN;
}

void module() {
	//Input data
	readSensors();
	readUDPdata();

	//Main functions
	mode();
	throttle();
	displayEvents();
	rotarySwitch();
	alarms();
	pump();
	buzzer();

	//Output settings
	setUDPdata();
	statusUpdate();
	outputs();
}

void readSensors() {
	if (sleep(&dallasSensorReadMillis, DELAY_SENSORS_READ)) return;
	DALLAS18b20Read();

	//TMP
//	String numberStr = Serial.readString();
//	int code = atoi(numberStr.c_str());
//
//	if (code!=0) {
//		Serial.println(code);
//		if (code<=4) tmpSensorActive = code-1;
//		else
//			if (tmpSensorActive<3)
//				device.thermo[tmpSensorActive].isTemp=code;
//			else
//				device.reqTemp=code;
//	}
}

void DALLAS18b20Read () {
	for (int number=0; number<3; number++) {
		DeviceAddress deviceAddress;
		for (int i=0; i<8; i++)
			deviceAddress[i] = device.thermo[number].deviceAddress[i];
		float tempC = 0;
		float a=0;
		float b=0;
		if (number== 0) {a=0.99;		b=(6.2);}			// thermometer inlet
		if (number== 1) {a=0.995;		b=(0+2.2);}			// thermometer outlet
		if (number== 2) {a=1.07;		b=(-4.5+2.7);}		// thermometer chimney
		tempC = sensors.getTempC(deviceAddress);
		if ((tempC<5)) {
			device.thermo[number].errorCount++;
			//zapisz liczbe maksymalna zlych odczytow z rzedu
			if (device.thermo[number].maxErrorCount < device.thermo[number].errorCount) device.thermo[number].maxErrorCount = device.thermo[number].errorCount;
			//po x probach nie udanych wysli fa³szyw¹ temperaturê
			if (device.thermo[number].errorCount > 10) device.thermo[number].isTemp = 100.00;
		}
		else {
			//przelicz temperature wedlug krzywej
			tempC = a * tempC + b;						// Temperature compensation
			//zerowanie liczby bledow
			device.thermo[number].errorCount = 0;
			device.thermo[number].isTemp = tempC;
		}
	}
	sensors.requestTemperatures();						// Request temperature
}

void readUDPdata() {
	UDPdata = getDataRead();
	if (!UDPdata.newData) return;
	if ((UDPdata.deviceType == ID_MOD_MAIN)
			&& (UDPdata.deviceNo == getModuleType())
			&& (UDPdata.frameNo == getModuleNo()))
		getMasterDeviceOrder();
	if ((UDPdata.deviceType == ID_MOD_HEATING)
			&& (UDPdata.deviceNo == 0)
			&& (UDPdata.frameNo == 0))
		getHeatingParams();
	resetNewData();
}

void getMasterDeviceOrder() {
	if (UDPdata.data[0] == 2) {
		device.reqTemp = UDPdata.data[1];
		EEpromWrite(24, UDPdata.data[1]);
	}
}

void getHeatingParams() {
	heatingModule.pumpInHouse = UDPbitStatus(UDPdata.data[0],5);
	heatingModule.tBuffCOdol = (int)UDPdata.data[5]/2.0;
	heatingModule.tBuffCOgora = (int)UDPdata.data[7]/2.0;
	heatingModule.paramsLastTimeRead = millis();
}

void mode() {
	//0-CZUWANIE, 1-ROZPALANIE, 2-GRZANIE, 3-GASZENIE
	if (device.mode==0) {
		device.throttle=0;								// Standby shut throttle. Pump is off
		device.startMillis = 0;
		device.reqTempReached = false;
		display.displayOff();
	}
	if (device.mode==1) {
		display.displayOn();
		if (device.startMillis == 0) {
			device.startMillis = millis();
			device.startTemp = (int)device.thermo[ID_OUTLET].isTemp;
		}

		device.throttle=100;							// By warming up open throttle on 100%

		//switch back to mode 0 if no fire recognized
		if ((millis()-device.startMillis)>60000)
			if (device.startTemp>(device.thermo[ID_OUTLET].isTemp-5))
				device.mode = 0;

		if (device.thermo[ID_OUTLET].isTemp>=50) {						// when reach 50C degree switch to mode 2-GRZANIE
			device.mode=2;
			screen.info[3].active = true;
		}
	}

	if (device.mode==2) {
		if (device.reqTemp>=device.thermo[ID_OUTLET].isTemp)
			device.reqTempReached = true;
		if ((device.reqTempReached) && (device.reqTemp>device.thermo[ID_OUTLET].isTemp)) {
			if (device.throttle<100)
				device.lowestTemp = (int)device.thermo[ID_OUTLET].isTemp;
		}

		if (device.lowestTemp>device.thermo[ID_OUTLET].isTemp)
			screen.info[4].active = true;
		if (screen.info[4].active) {
			if (device.lowestTemp<device.thermo[ID_OUTLET].isTemp)
				screen.info[4].active = false;
		}
	}

	if (((device.thermo[ID_INLET].isTemp+3) > device.thermo[ID_OUTLET].isTemp)
			&& (device.mode==3))
		device.mode = 0;
}

void throttle() {
	if (sleep(&throttleMillis, 60)) return;
	int delta = (int)(device.reqTemp-device.thermo[ID_OUTLET].isTemp);
	device.throttle = device.throttle + (delta*10);
	if (device.throttle>100) device.throttle = 100;
	if (device.throttle<0) device.throttle = 0;
}

void alarms() {
	screen.info[0].active = false;
	screen.info[1].active = false;

	if (device.thermo[ID_OUTLET].isTemp>90)
		screen.info[0].active = true;

	if (device.mode == 0) {
		if ((device.thermo[ID_OUTLET].isTemp>50) && (device.thermo[ID_OUTLET].isTemp>heatingModule.tBuffCOdol)) {
			screen.info[1].active = true;
		}
	}
}

void pump() {
	device.pump = false;
	if (device.mode == 2)
		device.pump = true;

	if (device.mode == 3)
		device.pump = true;

	if (screen.info[0].active)
		device.pump = true;

	if (screen.info[1].active)
		device.pump = true;
}

bool blink2Hz() {
	unsigned long current = millis();
	if (((current-((current/1000)*1000))/100) < 5)
		return true;
	else return false;
}

void buzzer() {
	if (device.alarm)
		device.buzzer = blink2Hz();
	if (device.warning)
		device.buzzer = blink2Hz() && (getDateTime().second==1);
}

void showMessage(Info info) {
	device.alarm = false;
	device.warning = false;
	display.clear();
	display.setTextAlignment(TEXT_ALIGN_LEFT);
	display.setFont(ArialMT_Plain_10);
	if (blink2Hz()) {
		if (info.type == ALARM) {
			printCenter(64,0,"!!!ALARM!!!");
			device.alarm = true;
		}
		if (info.type == WARN) {
			printCenter(64, 0, "OSTRZEZENIE");
			device.warning = true;
		}
		display.drawRect(0, 15, 128, 1);
	}
	printCenter(64,18,info.mess1);
	printCenter(64,29,info.mess2);
	printCenter(64,40,info.mess3);
	if (blink2Hz())
		printCenter(64,51,"OK?");
	display.display();
}

void showSettings() {
	display.clear();
	display.setTextAlignment(TEXT_ALIGN_LEFT);
	display.setFont(ArialMT_Plain_10);
	printCenter(64,0,"TEMP.ZADANA");
	display.drawRect(0, 15, 128, 1);
	display.setFont(ArialMT_Plain_24);
	char tmp[6];
	itoa(device.reqTemp,tmp,10); strcat(tmp,"stC");
	printCenter(64,20,tmp);
	display.drawRect(0, 63, 128, 1);
	display.display();
}

void getScreenInfo() {
	if (screen.activeScreen>=7)
		screen.activeScreen = 1;
	switch (device.mode) {
	  case 0 : screen.header = "CZUWANIE"; break;
	  case 1 : screen.header = "ROZPALANIE"; break;
	  case 2 : screen.header = "GRZANIE"; break;
	  case 3 : screen.header = "GASZENIE"; break;
	}
	char tmp[6];
	switch (screen.activeScreen) {
		case 1 : {
			screen.top.message = "WEJSCIE";
			itoa(device.thermo[ID_INLET].isTemp,tmp,10); strcat(tmp," C");
			screen.bottom.message = tmp;
		} break;
		case 2 : {
			screen.top.message = "WYJSCIE";
			itoa(device.thermo[ID_OUTLET].isTemp,tmp,10); strcat(tmp," C");
			screen.bottom.message = tmp;
		} break;
		case 3 : {
			screen.top.message = "KOMIN";
			itoa(device.thermo[ID_CHIMNEY].isTemp,tmp,10); strcat(tmp," C");
			screen.bottom.message = tmp;
		} break;
		case 4 : {
			screen.top.message = "PRZEPUSTNIA";
			itoa(device.throttle,tmp,10); strcat(tmp,"%");
			screen.bottom.message = tmp;
		} break;
		case 5 : {
			screen.top.message = "CO dol";
			itoa(heatingModule.tBuffCOdol,tmp,10); strcat(tmp," C");
			screen.bottom.message = tmp;
		} break;
		case 6 : {
			screen.top.message = "CO gora";
			itoa(heatingModule.tBuffCOgora,tmp,10); strcat(tmp," C");
			screen.bottom.message = tmp;
		} break;
	}
}

void displayEvents(){
	Info info;
	for (int i=0; i<INFO_MAX; i++) {
		if (screen.info[i].active) {
			info = screen.info[i];
			break;
		}
	}
	if (info.active) {
		showMessage(info);
		return;
	}
	if (device.settingsActive) {
		showSettings();
		return;
	}
	getScreenInfo();

	int oled_deltaPosTop, oled_deltaPosBottom;

	if (screen.top.curr>0) oled_deltaPosTop = abs(screen.top.curr-screen.top.goal);					// difference between goal and current position TOP SLOW->FAST
	else oled_deltaPosTop = abs(-100+screen.top.curr-screen.top.goal);									// difference between goal and current position TOP FAST->SLOW
	if (oled_deltaPosTop<14) oled_deltaPosTop = 14;															// constant scrolling speed when almost goal position TOP

	if (screen.bottom.curr>0) oled_deltaPosBottom = abs(screen.bottom.curr-screen.bottom.goal);		// difference between goal and current position TOP SLOW->FAST
	else oled_deltaPosBottom = abs(-100+screen.bottom.curr-screen.bottom.goal);							// difference between goal and current position TOP FAST->SLOW
	if (oled_deltaPosBottom<14) oled_deltaPosBottom = 14;														// constant scrolling speed when almost goal position BOTTOM

	screen.top.curr = screen.top.curr - (int)oled_deltaPosTop*0.15;									// scroll speed calculation TOP
	if (screen.top.curr<screen.top.goal) screen.top.curr=screen.top.goal;								// if current position TOP is too far then POS=GOAL

	screen.bottom.curr = screen.bottom.curr - (int)oled_deltaPosBottom*0.15;							// scroll speed calculation BOTTOM
	if (screen.bottom.curr<screen.bottom.goal) screen.bottom.curr=screen.bottom.goal;					// if current position BOTTOM is too far then POS=GOAL

	if (screen.top.curr==0) screen.bottom.goal=0;														// animation driver
	if (screen.bottom.curr==0) screen.top.goal=-100;
	if (screen.top.curr==-100) screen.bottom.goal=-100;
	if (screen.bottom.curr==-100) {
		screen.top.goal=0;
		screen.top.curr=100;
		screen.bottom.goal=100;
		screen.bottom.curr=100;
		screen.activeScreen++;
	}

	display.clear();
	if (device.mode == 0)
		return;
	//header
	display.setTextAlignment(TEXT_ALIGN_LEFT);
	display.setFont(ArialMT_Plain_10);
	printCenter(64,0,screen.header);

	//pump
	if (device.pump) {
		if (blink2Hz()) {
			display.drawRect(0, 6, 20, 1);
			display.drawRect(108, 6, 20, 1);
		};
	};
	display.drawRect(0, 15, 128, 1);
	printCenter(64+screen.top.curr,18,screen.top.message);
	display.setFont(ArialMT_Plain_24);
	printCenter(64+screen.bottom.curr,34,screen.bottom.message);
	display.drawRect(0, 63, 128, 1);
	display.display();
}

void rotarySwitch() {
	getPinState(&inCLK, pinCLK, true);
	getPinState(&inDT, pinDT, false);
	getPinState(&inSW, pinSW, true);
	if (inSW.isState) {
		if (device.mode == 0) {
			device.mode = 1;
			screen.info[2].active = true;
			addLog("Aktywacja tryby rozpalania");
		}
		else if (screen.info[2].active) screen.info[2].active = false;
		else if (screen.info[3].active) screen.info[3].active = false;
		else if (screen.info[4].active) {
			device.mode = 3;
			screen.info[4].active = false;
		}
		else device.settingsActive = !device.settingsActive;
	}
	if ((device.settingsActive) && (inCLK.isState)) {
		if (inDT.isState)
			device.reqTemp++;
		else device.reqTemp--;
		EEpromWrite(24, device.reqTemp);
	}
	if (device.reqTemp<50) device.reqTemp = 50;
	if (device.reqTemp>90) device.reqTemp = 90;
}

void setUDPdata() {
	int size = 6;
	byte dataWrite[size];
	dataWrite[0] = (device.mode << 6) | (device.alarm << 5) | (device.warning << 4) | (device.pump << 3) | (device.fireAlarm << 2);
	dataWrite[1] = device.throttle;
	dataWrite[2] = (byte)device.reqTemp;
	dataWrite[3] = (byte)device.thermo[ID_INLET].isTemp;
	dataWrite[4] = (byte)device.thermo[ID_OUTLET].isTemp;
	dataWrite[5] = (byte)device.thermo[ID_CHIMNEY].isTemp;
	setUDPdata(0, dataWrite,size);
}

void statusUpdate() {
	String status;
	status +="STAN";
	if (device.mode == 0) status += "\tCZUWANIE";
	if (device.mode == 1) status += "\tROZPALANIE";
	if (device.mode == 2) status += "\tGRZANIE";
	if (device.mode == 3) status += "\tWYGASZANIE";
	status += "\tPompa["; status += device.pump; status+="]";
	status += "\tPrzepustnica["; status += device.throttle; status+="]";
	status += "\tStartTemp["; status += device.startTemp; status+="]";
	status += "\nAlarm["; status += device.alarm; status+="]";
	status += "\tWarning["; status += device.warning; status+="]";
	status += "\tFireAlarm["; status += device.fireAlarm; status+="]";
	status += "\tLowestTemp["; status += device.lowestTemp; status+="]";
	status += "\nPARAMETRY";
	status +="\nTemp.ustawiona\tT="; status +=device.reqTemp; status +="[stC]";
	status +="\nTemp.WE\t\tT="; status +=device.thermo[ID_INLET].isTemp; status +="[stC]";
	status +="\nTemp.WY\t\tT="; status +=device.thermo[ID_OUTLET].isTemp; status +="[stC]";
	status +="\nTemp.KOMIN\tT="; status +=device.thermo[ID_CHIMNEY].isTemp; status +="[stC]\n";
	setStatus(status);
}

void outputs() {
	digitalWrite(pinPUMP, !device.pump);
	digitalWrite(pinSPIKER, !device.buzzer);

	//Throttle
	// parsing 0-100% into 0-90 degree
	int min = 10;			// 0 degree
	int max = 17;			// 90 degree
	int dutyCycle = min+(int)(device.throttle*((max-min)/100.00));
	if (dutyCycle<0) dutyCycle = 0;
	if (dutyCycle>255) dutyCycle = 255;
	ledcWrite(SERVO_CHANNEL, dutyCycle);
}

//HELP FUNCTIONS
void printCenter(int offsetX, int y, String text)
// to reach display center offsetX=64 or by scale 2x offsetX=32
{
	display.drawString(offsetX-display.getStringWidth(text)/2, y,text);
}

void diagDALLAS18b20ReadDeviceAdresses() {
	for (int i=0; i<3; i++) {
		sensors.getAddress(tmpAddresses[i],i);
	}
}

void TMPWritteValuesToEEprom() {
	DeviceAddress tempAddresses[7] = {
			{0,0,0,0,0,0,0,0},				// temperatura na wyjsciu
			{0,0,0,0,0,0,0,0},				// temperatura na wejsciu
			{0,0,0,0,0,0,0,0}				// temperatura komina
	};

	int offset = 0;
	//ADDRESS IS SAVING
	for (int dallasNo=0; dallasNo<3; dallasNo++) {
		Serial.printf("\nds18b20[%d]\t", dallasNo);
		for (int i=0; i<8; i++) {
			EEpromWrite((offset+(8*dallasNo+i)),tempAddresses[dallasNo][i]);
			Serial.printf("[%d][%d]\t",offset+(8*dallasNo+i),tempAddresses[dallasNo][i]);
			delay(100);
		}
	delay(100);
	}
}
