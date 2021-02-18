#include "Module.h"

Device device;
Display display;
HeatingModule heatingModule;
DataRead UDPdata;

OneWire oneWire(PIN_DS18B20);
DallasTemperature sensors(&oneWire);					// initialized 1-WIRE for buffers
DeviceAddress tmpAddresses[3];
SH1106 oled(true, OLED_RESET, OLED_DC, OLED_CS); 	// for Oled

//Functions
void firstScan();
void warningAndAlarmsDefinition();
void readSensors();
void DALLAS18b20Read ();
void readUDPdata();
void getMasterDeviceOrder();
void getHeatingParams();

//Module specific functions
void mode();
void throttle();
void warningsAlarms();
void displayEvents();
void rotarySwitch();

//Output functions
void setUDPdata();
void statusUpdate();

//Variables

//HELP FUNCTIONS
void diagDALLAS18b20ReadDeviceAdresses();
void TMPWritteValuesToEEprom();
//oled
void printCenter(int offsetX, int y, String text);

//Delays
unsigned long dallasSensorReadMillis = 0;

//TMP
//int lastDutyCycle = 0;

void module_init() {
	//EEprom Scan
	firstScan();

	//O-LED Display
	oled.init();
//	display.flipScreenVertically();

	warningAndAlarmsDefinition();

	//Set sensors Dallas DS18B20
	sensors.begin();
	sensors.setResolution(10);
	sensors.setWaitForConversion(false);
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

	//Temp
	TMPWritteValuesToEEprom();
}

void warningAndAlarmsDefinition() {
	//when water is over 90 degrees
	display.info[0].mess1 ="!!!WODA ZAGOTOWANA!!!";
	display.info[0].mess2 ="!!!GASZENIE KOMINKA!!!";
	display.info[0].type = ALARM;

	//when by Standby mode flow-temp is over 40 degrees turn alarm on, that nobody has trigger device to warm up
	display.info[1].mess1 ="TEMPERATURA KOMINKA ";
	display.info[1].mess2 ="W TRYBIE CZUWANIA";
	display.info[1].mess3 ="!!!PRZEKROCZONA!!!";
	display.info[1].type = ALARM;

	//WARNINGS
	//Notice to open chimney and air in-take to max by warming up
	display.info[2].mess1 ="Rozpalanie";
	display.info[2].mess2 ="Otworz klape";
	display.info[2].mess3 ="powietrza i komin";
	display.info[2].type = WARN;

	//Notice to shut chimney and air in-take by switching to normal mode
	display.info[3].mess1 ="Rozpalanie zakonczone";
	display.info[3].mess2 ="Zamknij klape";
	display.info[3].mess3 ="powietrza";
	display.info[3].type = WARN;

	//Notice to less timber. Set temperature unable to reach
	display.info[4].mess1 ="Ogien zbyt slaby";
	display.info[4].mess2 ="Zmienic tryb";
	display.info[4].mess3 ="na gaszenie?";
	display.info[4].type = WARN;
}

void module() {
	//Input data
	readSensors();
	readUDPdata();

	//Main functions
	mode();
	throttle();
	warningsAlarms();
	displayEvents();
	rotarySwitch();

	//Output settings
	setUDPdata();
	statusUpdate();
}

void readSensors() {
	if (sleep(&dallasSensorReadMillis, DELAY_SENSORS_READ)) return;
	DALLAS18b20Read();
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
	heatingModule.tBuffCOgora = UDPdata.data[7]/2.0;
	heatingModule.paramsLastTimeRead = millis();
}

void mode() {
	device.pump = false;
	//0-CZUWANIE, 1-ROZPALANIE, 2-GRZANIE, 3-GASZENIE
	if (device.mode==0) {
		device.throttle=0;								// Standby shut throttle. Pump is off
	}

	if (device.mode==1) {
		device.throttle=100;							// By warming up open throttle on 100%

		if (device.thermo[ID_OUTLET].isTemp>50) {						// when reach 50C degree switch to mode 2-GRZANIE
			device.mode=2;
			display.info[3].active = true;
		}
		//if after 10 minutes flow-temp is not 5 degrees higher than at the begin switch back to standby mode
		if ((device.minutesOnFire==10) && (device.thermo[ID_OUTLET].isTemp<(device.mode1StartTemperature+5))) device.mode=0;
	}

	if ((device.mode==2) || (device.mode==3)) {
		device.pump=true;
		// if fire place fire very poor switch to standby
		if ((device.thermo[ID_INLET].isTemp+3) > device.thermo[ID_OUTLET].isTemp)
			device.mode = 0;
	}

}

void throttle() {
	//TODO
}

void warningsAlarms() {
	//TODO

}

void displayEvents(){
	//TODO

}

void rotarySwitch() {
	//TODO

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
	status +="STAN\n";
	if (device.mode == 0) status += "CZUWANIE";
	if (device.mode == 1) status += "ROZPALANIE";
	if (device.mode == 2) status += "GRZANIE";
	if (device.mode == 3) status += "WYGASZANIE";
	status += "\tPompa["; status += device.pump; status+="]";
	status += "\tPrzepustnica["; status += device.throttle; status+="]";
	status += "\nAlarm["; status += device.alarm; status+="]";
	status += "\tWarning["; status += device.warning; status+="]";
	status += "\tFireAlarm["; status += device.fireAlarm; status+="]";
	status += "PARAMETRY \n";
	status +="\nTemp.ustawiona\tT="; status +=device.reqTemp; status +="[stC]";
	status +="\nTemp.WE\t\tT="; status +=device.thermo[ID_INLET].isTemp; status +="[stC]";
	status +="\nTemp.WY\t\tT="; status +=device.thermo[ID_OUTLET].isTemp; status +="[stC]";
	status +="\nTemp.KOMIN\tT="; status +=device.thermo[ID_CHIMNEY].isTemp; status +="[stC]";

	setStatus(status);
}

//HELP FUNCTIONS
void printCenter(int offsetX, int y, String text)
// to reach display center offsetX=64 or by scale 2x offsetX=32
{
	oled.drawString(offsetX-oled.getStringWidth(text)/2, y,text);
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
