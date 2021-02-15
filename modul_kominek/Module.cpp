#include "Module.h"

Device device;
HeatingDevice heatingDevice;
DataRead UDPdata;

OneWire oneWire(PIN_DS18B20);
DallasTemperature sensors(&oneWire);					// initialized 1-WIRE for buffers
DeviceAddress tmpAddresses[3];

//Functions
void firstScan();
void readSensors();
void DALLAS18b20Read ();
void readUDPdata();
void getMasterDeviceOrder();
void getHeatingParams();

//Module specific functions

//Output functions
void setUDPdata();
void statusUpdate();

//Variables

//DIAGNOSTIC HELP FUNCTIONS
void diagDALLAS18b20ReadDeviceAdresses();
void TMPWritteValuesToEEprom();

//Delays
unsigned long dallasSensorReadMillis = 0;

//TMP
//int lastDutyCycle = 0;

void module_init() {
	//EEprom Scan
	firstScan();

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

	device.setTemp = EEpromData[24];

	//Temp
	TMPWritteValuesToEEprom();
}

void module() {
	//Input data
	readSensors();
	readUDPdata();

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
		device.setTemp = UDPdata.data[1];
		EEpromWrite(24, UDPdata.data[1]);
	}
}

void getHeatingParams() {
	heatingDevice.pumpInHouse = UDPbitStatus(UDPdata.data[0],5);
	heatingDevice.paramsLastTimeRead = millis();
}

void setUDPdata() {
	int size = 6;
	byte dataWrite[size];
	dataWrite[0] = (device.mode << 6) | (device.alarm << 5) | (device.warning << 4) | (device.pump << 3) | (device.fireAlarm << 2);
	dataWrite[1] = device.throtlle;
	dataWrite[2] = (byte)device.setTemp;
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
	status += "\tPrzepustnica["; status += device.throtlle; status+="]";
	status += "\nAlarm["; status += device.alarm; status+="]";
	status += "\tWarning["; status += device.warning; status+="]";
	status += "\tFireAlarm["; status += device.fireAlarm; status+="]";
	status += "PARAMETRY \n";
	status +="\nTemp.ustawiona\tT="; status +=device.setTemp; status +="[stC]";
	status +="\nTemp.WE\t\tT="; status +=device.thermo[ID_INLET].isTemp; status +="[stC]";
	status +="\nTemp.WY\t\tT="; status +=device.thermo[ID_OUTLET].isTemp; status +="[stC]";
	status +="\nTemp.KOMIN\tT="; status +=device.thermo[ID_CHIMNEY].isTemp; status +="[stC]";

	setStatus(status);
}

//DIAGNOSTIC HELP FUNCTIONS
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
