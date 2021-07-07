#include "Module.h"
Device device;
HandMode handMode;
Zone zones[7];
AirPollution airPollution;
DataRead UDPdata;

//Functions
void firstScan();
void readSensors();
void readUDPdata();

void normalMode();
void humidityAlert();
void defrost();
void activeCoolingMode();
void activeHeatingMode();
void bypass();
void fan();

void manualMode();

void outputs();
void setUDPdata();
void statusUpdate();

//Variables
Adafruit_BME280 bme1(CS_BME280_CZERPNIA, SPI_MOSI, SPI_MISO, SPI_SCK);
Adafruit_BME280 bme2(CS_BME280_WYRZUTNIA, SPI_MOSI, SPI_MISO, SPI_SCK);
Adafruit_BME280 bme3(CS_BME280_NAWIEW, SPI_MOSI, SPI_MISO, SPI_SCK);
Adafruit_BME280 bme4(CS_BME280_WYWIEW, SPI_MOSI, SPI_MISO, SPI_SCK);

//Fan
int fanRev1 = 0;
int fanRev2 = 0;

//Bypass
//int lastDutyCycle = -1;
//bool servoAttached = false;

//Delays
unsigned long readSensorsMillis = 0;
unsigned long lastRevsRead = 0;

void module_init() {
	//Set CS pins
	device.sensorsBME280[0].interface= bme1;
	device.sensorsBME280[1].interface= bme2;
	device.sensorsBME280[2].interface= bme3;
	device.sensorsBME280[3].interface= bme4;
	//initialization
	for (int i=0; i<4; i++)
		device.sensorsBME280[i].interface.begin();

	//EEprom Scan
	firstScan();

	//Bypass initialization
	ledcSetup(SERVO_CHANNEL, SERVO_FREQUENCY, SERVO_RESOUTION);

	//Fan initialization
	//Czerpnia
	ledcSetup(PWM_FAN_CZ_CHANNEL, PWM_FREQUENCY, PWM_RESOUTION);
	ledcAttachPin(PIN_FAN_CZ_PWM, PWM_FAN_CZ_CHANNEL);
	pinMode(PIN_FAN_CZ_REVS, INPUT_PULLUP);
	digitalWrite(PIN_FAN_CZ_REVS, HIGH);
	//Wyrzutnia
	ledcSetup(PWM_FAN_CZ_CHANNEL, PWM_FREQUENCY, PWM_RESOUTION);
	ledcAttachPin(PIN_FAN_CZ_PWM, PWM_FAN_CZ_CHANNEL);
	pinMode(PIN_FAN_CZ_REVS, INPUT_PULLUP);
	digitalWrite(PIN_FAN_CZ_REVS, HIGH);
}

void module() {
	//Input data
	readSensors();
	readUDPdata();

	//Module events
	normalMode();
	humidityAlert();
	defrost();
	activeCoolingMode();
	activeHeatingMode();
	bypass();
	fan();

	//Hand mode
	manualMode();

	//Output settings
	outputs();
	setUDPdata();
	statusUpdate();
}

void firstScan() {
	// Bytes coding EEPROM //
	// 0			: 	Ogolne[0] autodiag etc
	// 1			:	Ogolne[1] active cooling/heating etc
	//Get data from EEprom
	int size = 1024;
	byte EEpromData[size];
	EEpromScan(EEpromData, size);
	Serial.println("\nFirst scan");

	//byte 0
	device.reqAutoDiagnosis = UDPbitStatus(EEpromData[0],0);

	//byte 1
	device.activeCooling = UDPbitStatus(EEpromData[1],6);
	device.activeHeating = UDPbitStatus(EEpromData[1],5);
	device.reqLazDol = UDPbitStatus(EEpromData[1],4);
	device.reqLazGora = UDPbitStatus(EEpromData[1],3);
	device.reqKuchnia = UDPbitStatus(EEpromData[1],2);

	//byte 26
	device.normalMode.delayTime = EEpromData[26];

	//byte 27
	device.humidityAlertMode.trigger = EEpromData[27];
	//byte 28
	device.humidityAlertMode.delayTime = EEpromData[28];

	//byte 29
	device.defrostMode.trigger = EEpromData[29];
	//byte 30
	device.defrostMode.delayTime = EEpromData[30];

	//byte 31-54
	for (int i=0; i<=24; i++) {
		device.activeTempRegByHours[i].salon = UDPbitStatus(EEpromData[i+31],7);
		device.activeTempRegByHours[i].pralnia = UDPbitStatus(EEpromData[i+31],6);
		device.activeTempRegByHours[i].lazDol = UDPbitStatus(EEpromData[i+31],5);
		device.activeTempRegByHours[i].rodzice = UDPbitStatus(EEpromData[i+31],4);
		device.activeTempRegByHours[i].Natalia = UDPbitStatus(EEpromData[i+31],3);
		device.activeTempRegByHours[i].Karolina = UDPbitStatus(EEpromData[i+31],2);
		device.activeTempRegByHours[i].lazGora= UDPbitStatus(EEpromData[i+31],1);
	}

	// byte 55
	device.minTemp = EEpromData[55];

	//byte 56-79
	for (int i=0; i<=24; i++) {
		device.normalOnByHours[i].salon = UDPbitStatus(EEpromData[i+56],7);
		device.normalOnByHours[i].pralnia = UDPbitStatus(EEpromData[i+56],6);
		device.normalOnByHours[i].lazDol = UDPbitStatus(EEpromData[i+56],5);
		device.normalOnByHours[i].rodzice = UDPbitStatus(EEpromData[i+56],4);
		device.normalOnByHours[i].Natalia = UDPbitStatus(EEpromData[i+56],3);
		device.normalOnByHours[i].Karolina = UDPbitStatus(EEpromData[i+56],2);
		device.normalOnByHours[i].lazGora= UDPbitStatus(EEpromData[i+65],1);
	}
}

void readBME280() {
	for (int i=0; i<4; i++) {
		device.sensorsBME280[i].temperature = device.sensorsBME280[i].interface.readTemperature();
		device.sensorsBME280[i].pressureHighPrec = device.sensorsBME280[i].interface.readPressure();
		device.sensorsBME280[i].pressure = (int)(device.sensorsBME280[i].pressureHighPrec/100);
		device.sensorsBME280[i].humidity = (int)device.sensorsBME280[i].interface.readHumidity();
		if (device.sensorsBME280[i].temperature>70
				|| device.sensorsBME280[i].pressure>1050
				|| device.sensorsBME280[i].pressure<800
				|| device.sensorsBME280[i].humidity>100
				|| device.sensorsBME280[i].humidity<15)
			device.sensorsBME280[i].faultyReadings++;
	}
}

void readDS18b20() {
	//TODO
}

void readSensors() {
	if (sleep(&readSensorsMillis, 5)) return;
	readBME280();
	readDS18b20();
}

void getMasterDeviceOrder() {
	//bytes 0
	if (UDPdata.data[0] == 0) {
		device.reqAutoDiagnosis = UDPbitStatus(UDPdata.data[1],0);
		EEpromWrite(0, UDPdata.data[1]);
	}

	//bytes 1
	if (UDPdata.data[0] == 1) {
		device.activeCooling = UDPbitStatus(UDPdata.data[1],6);
		device.activeHeating = UDPbitStatus(UDPdata.data[1],5);
		device.reqLazDol = UDPbitStatus(UDPdata.data[1],4);
		device.reqLazGora = UDPbitStatus(UDPdata.data[1],3);
		device.reqKuchnia = UDPbitStatus(UDPdata.data[1],2);
		EEpromWrite(1, UDPdata.data[1]);
	}

	//byte 26
	if (UDPdata.data[0] == 26) {
		device.normalMode.delayTime = UDPdata.data[26];
		EEpromWrite(26, UDPdata.data[1]);
	}

	//byte 27
	if (UDPdata.data[0] == 27) {
		device.humidityAlertMode.triggerInt = UDPdata.data[27];
		EEpromWrite(27, UDPdata.data[1]);
	}

	//byte 28
	if (UDPdata.data[0] == 28) {
		device.humidityAlertMode.delayTime = UDPdata.data[28];
		EEpromWrite(28, UDPdata.data[1]);
	}

	//byte 29
	if (UDPdata.data[0] == 29) {
		device.defrostMode.triggerInt = UDPdata.data[29];
		EEpromWrite(29, UDPdata.data[1]);
	}

	//byte 30
	if (UDPdata.data[0] == 30) {
		device.defrostMode.delayTime = UDPdata.data[30];
		EEpromWrite(30, UDPdata.data[1]);
	}

	//byte 31-54
	if ((UDPdata.data[0] >= 31) &&
			(UDPdata.data[0] <= 54)	) {
		int hour = UDPdata.data[0]-31;
		device.activeTempRegByHours[hour].salon   = UDPbitStatus(UDPdata.data[0],7);
		device.activeTempRegByHours[hour].pralnia = UDPbitStatus(UDPdata.data[0],6);
		device.activeTempRegByHours[hour].lazDol = UDPbitStatus(UDPdata.data[0],5);
		device.activeTempRegByHours[hour].rodzice = UDPbitStatus(UDPdata.data[0],4);
		device.activeTempRegByHours[hour].Natalia = UDPbitStatus(UDPdata.data[0],3);
		device.activeTempRegByHours[hour].Karolina = UDPbitStatus(UDPdata.data[0],2);
		device.activeTempRegByHours[hour].lazGora= UDPbitStatus(UDPdata.data[0],1);
		EEpromWrite(UDPdata.data[0], UDPdata.data[1]);
	}

	//byte 55
	if (UDPdata.data[0] == 55) {
		device.minTemp = UDPdata.data[55];
		EEpromWrite(55, UDPdata.data[1]);
	}

	//byte 56-79
	if ((UDPdata.data[0] >= 56) &&
			(UDPdata.data[0] <= 79)	) {
		int hour = UDPdata.data[0]-56;
		device.normalOnByHours[hour].salon = UDPbitStatus(UDPdata.data[0],7);
		device.normalOnByHours[hour].pralnia = UDPbitStatus(UDPdata.data[0],6);
		device.normalOnByHours[hour].lazDol = UDPbitStatus(UDPdata.data[0],5);
		device.normalOnByHours[hour].rodzice = UDPbitStatus(UDPdata.data[0],4);
		device.normalOnByHours[hour].Natalia = UDPbitStatus(UDPdata.data[0],3);
		device.normalOnByHours[hour].Karolina = UDPbitStatus(UDPdata.data[0],2);
		device.normalOnByHours[hour].lazGora= UDPbitStatus(UDPdata.data[0],1);
		EEpromWrite(UDPdata.data[0], UDPdata.data[1]);
	}

	//Hand Mode
	//byte 100
	if (UDPdata.data[0] == 100) {
		(UDPdata.data[1]>0)?handMode.enabled = true: handMode.enabled = false;
	}
	//byte 101
	if (UDPdata.data[0] == 101) {
		(UDPdata.data[1]>100)? handMode.fanSpeed = 100:(UDPdata.data[1]<0)? handMode.fanSpeed = 0 : handMode.fanSpeed = UDPdata.data[1];
	}
	//byte 102
	if (UDPdata.data[0] == 102) {
		(UDPdata.data[1]>0)?handMode.byPassOpen = true: handMode.byPassOpen = false;
	}
	setUDPdata();
	forceStandardUDP();
}

void getComfortParams(){
	zones[ID_ZONE_SALON].isTemp = UDPdata.data[0]+(UDPdata.data[1]/10.0);
	zones[ID_ZONE_SALON].reqTemp = UDPdata.data[2]/2;
	zones[ID_ZONE_SALON].humidity = UDPdata.data[3];

	zones[ID_ZONE_LAZDOL].isTemp = UDPdata.data[4]+(UDPdata.data[5]/10.0);
	zones[ID_ZONE_LAZDOL].reqTemp = UDPdata.data[6]/2;
	zones[ID_ZONE_LAZDOL].humidity = UDPdata.data[7];

	zones[ID_ZONE_PRALNIA].isTemp = UDPdata.data[8]+(UDPdata.data[9]/10.0);
	zones[ID_ZONE_PRALNIA].reqTemp = UDPdata.data[10]/2;
	zones[ID_ZONE_PRALNIA].humidity = UDPdata.data[11];

	zones[ID_ZONE_RODZICE].isTemp = UDPdata.data[12]+(UDPdata.data[13]/10.0);
	zones[ID_ZONE_RODZICE].reqTemp = UDPdata.data[14]/2;
	zones[ID_ZONE_RODZICE].humidity = UDPdata.data[15];

	zones[ID_ZONE_NATALIA].isTemp = UDPdata.data[16]+(UDPdata.data[17]/10.0);
	zones[ID_ZONE_NATALIA].reqTemp = UDPdata.data[18]/2;
	zones[ID_ZONE_NATALIA].humidity = UDPdata.data[19];

	zones[ID_ZONE_KAROLINA].isTemp = UDPdata.data[20]+(UDPdata.data[21]/10.0);
	zones[ID_ZONE_KAROLINA].reqTemp = UDPdata.data[22]/2;
	zones[ID_ZONE_KAROLINA].humidity = UDPdata.data[23];

	zones[ID_ZONE_LAZGORA].isTemp = UDPdata.data[24]+(UDPdata.data[25]/10.0);
	zones[ID_ZONE_LAZGORA].reqTemp = UDPdata.data[26]/2;
	zones[ID_ZONE_LAZGORA].humidity = UDPdata.data[27];
}

void getAirParams(){
	airPollution.pm25 = (UDPdata.data[5]<<8) + (UDPdata.data[6]);
	airPollution.pm10 = (UDPdata.data[7]<<8) + (UDPdata.data[8]);
}


void readUDPdata() {
	UDPdata = getDataRead();
	if (!UDPdata.newData) return;
	if ((UDPdata.deviceType == ID_MOD_MAIN)
			&& (UDPdata.deviceNo == getModuleType())
			&& (UDPdata.frameNo == getModuleNo()))
		getMasterDeviceOrder();
	if ((UDPdata.deviceType == ID_MOD_COMFORT)
			&& (UDPdata.deviceNo == 0)
			&& (UDPdata.frameNo == 0))
		getComfortParams();
	if ((UDPdata.deviceType == ID_MOD_WEATHER)
			&& (UDPdata.deviceNo == 0)
			&& (UDPdata.frameNo == 0))
		getAirParams();

	resetNewData();
}

void normalMode() {
	device.normalMode.trigger = false;

	if (device.normalOnByHours[getDateTime().hour].salon) {
		device.normalMode.trigger = true;
	}
	if (device.normalOnByHours[getDateTime().hour].pralnia) {
		device.normalMode.trigger = true;
	}
	if (device.normalOnByHours[getDateTime().hour].lazDol) {
		device.normalMode.trigger = true;
	}
	if (device.normalOnByHours[getDateTime().hour].rodzice) {
		device.normalMode.trigger = true;
	}
	if (device.normalOnByHours[getDateTime().hour].Natalia) {
		device.normalMode.trigger = true;
	}
	if (device.normalOnByHours[getDateTime().hour].Karolina) {
		device.normalMode.trigger = true;
	}
	if (device.normalOnByHours[getDateTime().hour].lazGora) {
		device.normalMode.trigger = true;
	}

	if ((device.normalMode.trigger) && (getDateTime().minute == 0))
		device.normalMode.endMillis = millis()+(device.normalMode.delayTime*1000*60);

	device.normalMode.timeLeft = (int)((device.normalMode.endMillis - millis())/1000);
	if (device.normalMode.timeLeft<0)
		device.normalMode.timeLeft = 0;

	if (device.normalMode.timeLeft > 0)
		device.normalOn = true;

	//don't turn on in normal mode ventilation if air is dusty
	if ((airPollution.pm10>150)
			|| airPollution.pm25>150)
		device.normalOn = false;

}

void humidityAlert() {
	device.humidityAlertMode.trigger = false;

	if (zones[ID_ZONE_SALON].humidity>=device.humidityAlertMode.triggerInt)
		device.humidityAlertMode.trigger = true;
	if (zones[ID_ZONE_PRALNIA].humidity>=device.humidityAlertMode.triggerInt)
		device.humidityAlertMode.trigger = true;
	if (zones[ID_ZONE_LAZDOL].humidity>=device.humidityAlertMode.triggerInt)
		device.humidityAlertMode.trigger = true;
	if (zones[ID_ZONE_RODZICE].humidity>=device.humidityAlertMode.triggerInt)
		device.humidityAlertMode.trigger = true;
	if (zones[ID_ZONE_NATALIA].humidity>=device.humidityAlertMode.triggerInt)
		device.humidityAlertMode.trigger = true;
	if (zones[ID_ZONE_KAROLINA].humidity>=device.humidityAlertMode.triggerInt)
		device.humidityAlertMode.trigger = true;
	if (zones[ID_ZONE_LAZGORA].humidity>=device.humidityAlertMode.triggerInt)
		device.humidityAlertMode.trigger = true;

	if (device.humidityAlertMode.trigger)
		device.humidityAlertMode.endMillis = millis() + (1000*60*device.humidityAlertMode.delayTime);

	device.humidityAlertMode.timeLeft = (int)((device.humidityAlertMode.endMillis - millis())/1000);
	if (device.humidityAlertMode.timeLeft<0)
		device.humidityAlertMode.timeLeft = 0;

	if (device.humidityAlertMode.timeLeft > 0)
		device.humidityAlert = true;

	int turboOnLimit = device.humidityAlertMode.triggerInt + 10;
	device.humidityAlertMode.turbo = false;
	if (zones[ID_ZONE_SALON].humidity>=turboOnLimit)
		device.humidityAlertMode.turbo = true;
	if (zones[ID_ZONE_PRALNIA].humidity>=turboOnLimit)
		device.humidityAlertMode.turbo = true;
	if (zones[ID_ZONE_LAZDOL].humidity>=turboOnLimit)
		device.humidityAlertMode.turbo = true;
	if (zones[ID_ZONE_RODZICE].humidity>=turboOnLimit)
		device.humidityAlertMode.turbo = true;
	if (zones[ID_ZONE_NATALIA].humidity>=turboOnLimit)
		device.humidityAlertMode.turbo = true;
	if (zones[ID_ZONE_KAROLINA].humidity>=turboOnLimit)
		device.humidityAlertMode.turbo = true;
	if (zones[ID_ZONE_LAZGORA].humidity>=turboOnLimit)
		device.humidityAlertMode.turbo = true;
}

void defrost() {
	unsigned long currentMillis = millis();

	device.defrostMode.trigger = false;
	if ((device.sensorsBME280[ID_CZERPNIA].pressureHighPrec-device.sensorsBME280[ID_NAWIEW].pressureHighPrec) >= device.defrostMode.triggerInt)
		device.defrostMode.trigger = true;
	if (device.defrostMode.trigger)
		device.defrostMode.endMillis = millis() + (device.defrostMode.delayTime * 1000 * 60);

	device.defrostMode.timeLeft = (int)((device.defrostMode.endMillis - millis())/1000);
	if (device.defrostMode.timeLeft<0)
		device.defrostMode.timeLeft = 0;

	if (device.defrostMode.timeLeft > 0)
		device.defrostActive = true;
}

void activeCoolingMode() {
	if (!device.activeCooling)
		return;

	//histeresis
	float hist = 1.0f;
	//SALON
	if (device.activeTempRegByHours[getDateTime().hour].salon) {
		if (zones[ID_ZONE_SALON].isTemp>=(zones[ID_ZONE_SALON].reqTemp+hist))
			device.zoneReqReg.salon = true;
	}
	if (zones[ID_ZONE_SALON].isTemp<=(zones[ID_ZONE_SALON].reqTemp))
		device.zoneReqReg.salon = false;

	//PRALNIA
	if (device.activeTempRegByHours[getDateTime().hour].pralnia) {
		if (zones[ID_ZONE_PRALNIA].isTemp>=(zones[ID_ZONE_PRALNIA].reqTemp+hist))
			device.zoneReqReg.pralnia = true;
	}
	if (zones[ID_ZONE_PRALNIA].isTemp<=(zones[ID_ZONE_PRALNIA].reqTemp))
		device.zoneReqReg.pralnia = false;

	//LAZ.DOL
	if (device.activeTempRegByHours[getDateTime().hour].lazDol) {
		if (zones[ID_ZONE_LAZDOL].isTemp>=(zones[ID_ZONE_LAZDOL].reqTemp+hist))
			device.zoneReqReg.lazDol = true;
	}
	if (zones[ID_ZONE_LAZDOL].isTemp<=(zones[ID_ZONE_LAZDOL].reqTemp))
		device.zoneReqReg.lazDol = false;

	//RODZICE
	if (device.activeTempRegByHours[getDateTime().hour].rodzice) {
		if (zones[ID_ZONE_RODZICE].isTemp>=(zones[ID_ZONE_RODZICE].reqTemp+hist))
			device.zoneReqReg.rodzice = true;
	}
	if (zones[ID_ZONE_RODZICE].isTemp<=(zones[ID_ZONE_RODZICE].reqTemp))
		device.zoneReqReg.rodzice = false;

	//NATALIA
	if (device.activeTempRegByHours[getDateTime().hour].Natalia) {
		if (zones[ID_ZONE_NATALIA].isTemp>=(zones[ID_ZONE_NATALIA].reqTemp+hist))
			device.zoneReqReg.Natalia = true;
	}
	if (zones[ID_ZONE_NATALIA].isTemp<=(zones[ID_ZONE_NATALIA].reqTemp))
		device.zoneReqReg.Natalia = false;

	//KAROLINA
	if (device.activeTempRegByHours[getDateTime().hour].Karolina) {
		if (zones[ID_ZONE_KAROLINA].isTemp>=(zones[ID_ZONE_KAROLINA].reqTemp+hist))
			device.zoneReqReg.Karolina = true;
	}
	if (zones[ID_ZONE_KAROLINA].isTemp<=(zones[ID_ZONE_KAROLINA].reqTemp))
		device.zoneReqReg.Karolina = false;

	//LAZ.GORA
	if (device.activeTempRegByHours[getDateTime().hour].lazGora) {
		if (zones[ID_ZONE_LAZGORA].isTemp>=(zones[ID_ZONE_LAZGORA].reqTemp+hist))
			device.zoneReqReg.lazGora = true;
	}
	if (zones[ID_ZONE_LAZGORA].isTemp<=(zones[ID_ZONE_LAZGORA].reqTemp))
		device.zoneReqReg.lazGora = false;
}

void activeHeatingMode() {
	if (!device.activeHeating)
		return;

	//histeresis
	float hist = device.minTemp;
	//SALON
	if (device.activeTempRegByHours[getDateTime().hour].salon) {
		if (zones[ID_ZONE_SALON].isTemp<=(zones[ID_ZONE_SALON].reqTemp-hist))
			device.zoneReqReg.salon = true;
	}
	if (zones[ID_ZONE_SALON].isTemp>=(zones[ID_ZONE_SALON].reqTemp))
		device.zoneReqReg.salon = false;

	//PRALNIA
	if (device.activeTempRegByHours[getDateTime().hour].pralnia) {
		if (zones[ID_ZONE_PRALNIA].isTemp<=(zones[ID_ZONE_PRALNIA].reqTemp-hist))
			device.zoneReqReg.pralnia = true;
	}
	if (zones[ID_ZONE_PRALNIA].isTemp>=(zones[ID_ZONE_PRALNIA].reqTemp))
		device.zoneReqReg.pralnia = false;

	//LAZ.DOL
	if (device.activeTempRegByHours[getDateTime().hour].lazDol) {
		if (zones[ID_ZONE_LAZDOL].isTemp<=(zones[ID_ZONE_LAZDOL].reqTemp-hist))
			device.zoneReqReg.lazDol = true;
	}
	if (zones[ID_ZONE_LAZDOL].isTemp>=(zones[ID_ZONE_LAZDOL].reqTemp))
		device.zoneReqReg.lazDol = false;

	//RODZICE
	if (device.activeTempRegByHours[getDateTime().hour].rodzice) {
		if (zones[ID_ZONE_RODZICE].isTemp<=(zones[ID_ZONE_RODZICE].reqTemp-hist))
			device.zoneReqReg.rodzice = true;
	}
	if (zones[ID_ZONE_RODZICE].isTemp>=(zones[ID_ZONE_RODZICE].reqTemp))
		device.zoneReqReg.rodzice = false;

	//NATALIA
	if (device.activeTempRegByHours[getDateTime().hour].Natalia) {
		if (zones[ID_ZONE_NATALIA].isTemp<=(zones[ID_ZONE_NATALIA].reqTemp-hist))
			device.zoneReqReg.Natalia = true;
	}
	if (zones[ID_ZONE_NATALIA].isTemp>=(zones[ID_ZONE_NATALIA].reqTemp))
		device.zoneReqReg.Natalia = false;

	//KAROLINA
	if (device.activeTempRegByHours[getDateTime().hour].Karolina) {
		if (zones[ID_ZONE_KAROLINA].isTemp<=(zones[ID_ZONE_KAROLINA].reqTemp-hist))
			device.zoneReqReg.Karolina = true;
	}
	if (zones[ID_ZONE_KAROLINA].isTemp>=(zones[ID_ZONE_KAROLINA].reqTemp))
		device.zoneReqReg.Karolina = false;

	//LAZ.GORA
	if (device.activeTempRegByHours[getDateTime().hour].lazGora) {
		if (zones[ID_ZONE_LAZGORA].isTemp<=(zones[ID_ZONE_LAZGORA].reqTemp-hist))
			device.zoneReqReg.lazGora = true;
	}
	if (zones[ID_ZONE_LAZGORA].isTemp>=(zones[ID_ZONE_LAZGORA].reqTemp))
		device.zoneReqReg.lazGora = false;
}

void bypass() {
	if (device.defrostMode.timeLeft > 0) {
		device.bypassOpen = true;
		return;
	}
	if (device.activeCooling) {
		if (device.sensorsBME280[ID_CZERPNIA].temperature<device.sensorsBME280[ID_WYWIEW].temperature)
			device.bypassOpen = true;
		if (device.sensorsBME280[ID_CZERPNIA].temperature>=(device.sensorsBME280[ID_WYWIEW].temperature+0.5f))
			device.bypassOpen = false;
		return;
	}

	device.bypassOpen = false;
}

void fan() {
	//revolution counting
	if ((digitalRead(PIN_FAN_CZ_REVS) == LOW) && (device.fan[FAN_CZERPNIA].release)) {
		device.fan[FAN_CZERPNIA].release = false;
		fanRev1++;
	}
	if (digitalRead(PIN_FAN_CZ_REVS) == HIGH)
		device.fan[FAN_CZERPNIA].release = true;

	if ((digitalRead(PIN_FAN_WY_REVS) == LOW) && (device.fan[FAN_WYWIEW].release)) {
		device.fan[FAN_WYWIEW].release = false;
		fanRev2++;
	}
	if (digitalRead(PIN_FAN_WY_REVS) == HIGH)
		device.fan[FAN_WYWIEW].release = true;

	unsigned long currentMillis = millis();
	if ((currentMillis-lastRevsRead)>=10000) {
		lastRevsRead = currentMillis;
		device.fan[FAN_CZERPNIA].rev = (int)(fanRev1*6);
		device.fan[FAN_WYWIEW].rev = (int)(fanRev2*6);
		fanRev1 = 0;
		fanRev2 = 0;
		//Fan MAX revs 3716
		//Fan MIN revs 0
	}

	device.fan[FAN_CZERPNIA].speed = 0;
	device.fan[FAN_WYWIEW].speed = 0;
	if (device.normalMode.delayTime>0) {
		device.fan[FAN_CZERPNIA].speed = 50;
		device.fan[FAN_WYWIEW].speed = 50;
	}
	if (device.humidityAlertMode.timeLeft>0) {
		device.fan[FAN_CZERPNIA].speed = 75;
		device.fan[FAN_WYWIEW].speed = 75;
	}
	if (device.defrostMode.timeLeft>0) {
		device.fan[FAN_CZERPNIA].speed = 80;
		device.fan[FAN_WYWIEW].speed = 80;
	}
	if (device.humidityAlertMode.turbo) {
		device.fan[FAN_CZERPNIA].speed = 100;
		device.fan[FAN_WYWIEW].speed = 100;
	}
}

void manualMode() {
	if (!handMode.enabled)
		return;
	device.fan[FAN_CZERPNIA].speed = handMode.fanSpeed;
	device.fan[FAN_WYWIEW].speed = handMode.fanSpeed;
	device.bypassOpen = handMode.byPassOpen;
}

void outputs() {
	//Bypass
	if (device.bypassOpen) device.byppass.dutyCycle =10;
	else device.byppass.dutyCycle =17;

	if (device.byppass.lastDutyCycle != device.byppass.dutyCycle) {
		device.byppass.lastDutyCycle = device.byppass.dutyCycle;
		ledcAttachPin(SERVO_PIN, SERVO_CHANNEL);
		device.byppass.attached = true;
		ledcWrite(SERVO_CHANNEL, device.byppass.dutyCycle);
		//hold servo for 5s
		device.byppass.endMillis = millis()+5000;
	}

	if ((millis()>device.byppass.endMillis) && (device.byppass.attached)) {
		ledcDetachPin(SERVO_PIN);
		device.byppass.attached = false;
	}

	//Fans
	// parsing 0-100% into 255-0
	int dutyCycle = 255-(int)((device.fan[FAN_CZERPNIA].speed/100.00)*255);
	if (dutyCycle<0) dutyCycle = 0;
	if (dutyCycle>255) dutyCycle = 255;
	ledcWrite(PWM_FAN_CZ_CHANNEL, dutyCycle);

	dutyCycle = 255-(int)((device.fan[FAN_WYWIEW].speed/100.00)*255);
	if (dutyCycle<0) dutyCycle = 0;
	if (dutyCycle>255) dutyCycle = 255;
	ledcWrite(PWM_FAN_WY_CHANNEL, dutyCycle);
}

void setUDPdata() {
	int size = 37;
	byte dataWrite[size];
	// First three bytes are reserved for device recognized purposes.
//	dataWrite[0] = (((device.fanSpeed>0)?1:0)<< 7) | (device.normalON << 6) | (device.humidityAlert.req<< 5) | (device.bypassOpen << 4) | (device.defrost.req << 3);
//	dataWrite[1] = device.hour[0];
//	dataWrite[2] = device.hour[1];
//	dataWrite[3] = device.hour[2];
//	dataWrite[4] = device.hour[3];
//	dataWrite[5] = device.hour[4];
//	dataWrite[6] = device.hour[5];
//	dataWrite[7] = device.hour[6];
//	dataWrite[8] = device.hour[7];
//	dataWrite[9] = device.hour[8];
//	dataWrite[10] = device.hour[9];
//	dataWrite[11] = device.hour[10];
//	dataWrite[12] = device.hour[11];
//	//BMEs
//	dataWrite[13] = get10Temp(device.sensorsBME280[ID_CZERPNIA].temperature);
//	dataWrite[14] = get01Temp(device.sensorsBME280[ID_CZERPNIA].temperature);
//	dataWrite[15] = device.sensorsBME280[ID_CZERPNIA].humidity;
//	dataWrite[16] = (int)(device.sensorsBME280[ID_CZERPNIA].pressure/10);
//
//	dataWrite[17] = get10Temp(device.sensorsBME280[ID_WYRZUTNIA].temperature);
//	dataWrite[18] = get01Temp(device.sensorsBME280[ID_WYRZUTNIA].temperature);
//	dataWrite[19] = device.sensorsBME280[ID_WYRZUTNIA].humidity;
//	dataWrite[20] = (int)(device.sensorsBME280[ID_WYRZUTNIA].pressure/10);
//
//	dataWrite[21] = get10Temp(device.sensorsBME280[ID_NAWIEW].temperature);
//	dataWrite[22] = get01Temp(device.sensorsBME280[ID_NAWIEW].temperature);
//	dataWrite[23] = device.sensorsBME280[ID_NAWIEW].humidity;
//	dataWrite[24] = (int)(device.sensorsBME280[ID_NAWIEW].pressure/10);
//
//	dataWrite[25] = get10Temp(device.sensorsBME280[ID_WYWIEW].temperature);
//	dataWrite[26] = get01Temp(device.sensorsBME280[ID_WYWIEW].temperature);
//	dataWrite[27] = device.sensorsBME280[ID_WYWIEW].humidity;
//	dataWrite[28] = (int)(device.sensorsBME280[ID_WYWIEW].pressure/10);
//
//	dataWrite[29] = device.fanSpeed;
//	dataWrite[30] = (int)(device.fan1revs/100);
//	dataWrite[31] = (int)(device.fan2revs/100);
//
//	dataWrite[32] = device.defrost.timeLeft;
//	dataWrite[33] = device.defrost.trigger;
//
//	dataWrite[34] = device.humidityAlert.timeLeft;
//	dataWrite[35] = device.humidityAlert.trigger;
//
//	dataWrite[36] = device.efficency.is;
//
	setUDPdata(0, dataWrite,size);
}

void statusUpdate() {
	String status;
//	status = "PARAMETRY PRACY\n";
//	status +="Wentylatory: "; status += device.fanSpeed; status +="[%]\tObr1: "; status += device.fan1revs; status +="[min-1]\tObr2: "; status += device.fan2revs; status +="[min-1]\n";
//	status +="EFF:"; status += device.efficency.is; status +="[%] MIN:"; status += device.efficency.min; status +="[%] MAX:"; status += device.efficency.max; status +="[%]\n:";
//	status +="NormalON: "; status += device.normalON ? "TAK":"NIE"; status +="\tbypass force: "; status += device.bypassForce? "TAK":"NIE"; status +="\tbypass otwarty: "; status += device.bypassOpen ? "TAK":"NIE"; status +="\tservo wysterowane: "; status += servoAttached ? "TAK":"NIE"; status +="\n";
//	status +="Odmrazanie: "; status += device.defrost.req ? "TAK":"NIE"; status +="\ttime left: "; status += device.defrost.timeLeft; status +="[min]\ttrigger EFF: "; status += device.defrost.trigger; status +="[%]\n";
//	status +="Humidity Alert: "; status += device.humidityAlert.req ? "TAK":"NIE"; status +="\ttime left: "; status += device.humidityAlert.timeLeft; status +="[min]\ttrigger: "; status += device.humidityAlert.trigger; status +="[%]\n";
//	status +="Czerpnia:\t T="; status +=device.sensorsBME280[0].temperature; status +="[stC]\tH="; status +=(int)device.sensorsBME280[0].humidity;
//	status +="[%]\tP="; status +=(int)device.sensorsBME280[0].pressureHighPrec;status +="[hPa*100] Faulty="; status +=(int)device.sensorsBME280[0].faultyReadings ;status +="\n";
//	status +="Wyrzutnia:\t T="; status +=device.sensorsBME280[1].temperature; status +="[stC]\tH="; status +=(int)device.sensorsBME280[1].humidity;
//	status +="[%]\tP="; status +=(int)device.sensorsBME280[1].pressureHighPrec;status +="[hPa*100] Faulty="; status +=(int)device.sensorsBME280[1].faultyReadings ;status +="\n";
//	status +="Nawiew:\t\t T="; status +=device.sensorsBME280[2].temperature; status +="[stC]\tH="; status +=(int)device.sensorsBME280[2].humidity;
//	status +="[%]\tP="; status +=(int)device.sensorsBME280[2].pressureHighPrec;status +="[hPa*100] Faulty="; status +=(int)device.sensorsBME280[2].faultyReadings ;status +="\n";
//	status +="Wywiew:\t\t T="; status +=device.sensorsBME280[3].temperature; status +="[stC]\tH="; status +=(int)device.sensorsBME280[3].humidity;
//	status +="[%]\tP="; status +=(int)device.sensorsBME280[3].pressureHighPrec;status +="[hPa*100] Faulty="; status +=(int)device.sensorsBME280[3].faultyReadings ;status +="\n";
//
//	status +="\n\nHand mode";
//	status +="\nEnabled="; status +=handMode.enabled;status +="\tfanSpeed="; status +=handMode.fanSpeed;status +="[%]\t bypassOpen="; status +=handMode.byPassOpen;
//	for (int i=0; i<7; i++) {
//		status +="\nZone["; status +=i; status += "]:\t\t T="; status +=zones[i].isTemp; status +="[stC]\treqT="; status+=zones[i].reqTemp; status +="[stC]\tH="; status +=zones[i].humidity;
//	}
//	status +="\nAir\tPM2.5="; status+=airPollution.pm25; status +="[ug/m3]\t PM10="; status +=airPollution.pm10; status+="[ug/m3]\n";
	setStatus(status);
}




