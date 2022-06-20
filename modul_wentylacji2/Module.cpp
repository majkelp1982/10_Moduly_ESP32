#include "Module.h"
Device device;
HandMode handMode;
TestMode testMode;
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
void flaps();
void fan();
void circuitPump();

void manualMode();

void outputs();
void setUDPdata();
void statusUpdate();

//Variables
Adafruit_BME280 bme1(PIN_CS_BME280_CZERPNIA, PIN_SPI_MOSI, PIN_SPI_MISO, PIN_SPI_SCK);
Adafruit_BME280 bme2(PIN_CS_BME280_WYRZUTNIA, PIN_SPI_MOSI, PIN_SPI_MISO, PIN_SPI_SCK);
Adafruit_BME280 bme3(PIN_CS_BME280_NAWIEW, PIN_SPI_MOSI, PIN_SPI_MISO, PIN_SPI_SCK);
Adafruit_BME280 bme4(PIN_CS_BME280_WYWIEW, PIN_SPI_MOSI, PIN_SPI_MISO, PIN_SPI_SCK);

//Fan
int fanRev1 = 0;
int fanRev2 = 0;

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
	ledcSetup(PWM_FAN_WY_CHANNEL, PWM_FREQUENCY, PWM_RESOUTION);
	ledcAttachPin(PIN_FAN_WY_PWM, PWM_FAN_WY_CHANNEL);
	pinMode(PIN_FAN_WY_REVS, INPUT_PULLUP);
	digitalWrite(PIN_FAN_WY_REVS, HIGH);

	//relays
	pinMode(PIN_CIRCUIT_PUMP, OUTPUT); digitalWrite(PIN_CIRCUIT_PUMP, HIGH);
	pinMode(PIN_RELAY_RES, OUTPUT);  digitalWrite(PIN_RELAY_RES, HIGH);
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
	if (!device.activeCooling
			&& !device.activeHeating) {
		device.zoneReqReg.salon = false;
		device.zoneReqReg.pralnia = false;
		device.zoneReqReg.lazDol = false;
		device.zoneReqReg.rodzice = false;
		device.zoneReqReg.Natalia = false;
		device.zoneReqReg.Karolina= false;
		device.zoneReqReg.lazGora = false;
	}
	bypass();
	fan();
	circuitPump();
	flaps();

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

	//byte 30
	device.normalMode.delayTime = EEpromData[30];

	//byte 31
	device.humidityAlertMode.triggerInt = EEpromData[31];
	//byte 32
	device.humidityAlertMode.delayTime = EEpromData[32];

	//byte 33
	device.defrostMode.triggerInt = EEpromData[33];
	//byte 34
	device.defrostMode.delayTime = EEpromData[34];

	//byte 35-58
	for (int i=0; i<=24; i++) {
		device.activeTempRegByHours[i].salon = UDPbitStatus(EEpromData[i+35],7);
		device.activeTempRegByHours[i].pralnia = UDPbitStatus(EEpromData[i+35],6);
		device.activeTempRegByHours[i].lazDol = UDPbitStatus(EEpromData[i+35],5);
		device.activeTempRegByHours[i].rodzice = UDPbitStatus(EEpromData[i+35],4);
		device.activeTempRegByHours[i].Natalia = UDPbitStatus(EEpromData[i+35],3);
		device.activeTempRegByHours[i].Karolina = UDPbitStatus(EEpromData[i+35],2);
		device.activeTempRegByHours[i].lazGora= UDPbitStatus(EEpromData[i+35],1);
	}

	// byte 59
	device.minTemp = EEpromData[59];

	//byte 60-83
	for (int i=0; i<=24; i++) {
		device.normalOnByHours[i].salon = UDPbitStatus(EEpromData[i+60],7);
		device.normalOnByHours[i].pralnia = UDPbitStatus(EEpromData[i+60],6);
		device.normalOnByHours[i].lazDol = UDPbitStatus(EEpromData[i+60],5);
		device.normalOnByHours[i].rodzice = UDPbitStatus(EEpromData[i+60],4);
		device.normalOnByHours[i].Natalia = UDPbitStatus(EEpromData[i+60],3);
		device.normalOnByHours[i].Karolina = UDPbitStatus(EEpromData[i+60],2);
		device.normalOnByHours[i].lazGora= UDPbitStatus(EEpromData[i+60],1);
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

	//byte 30
	if (UDPdata.data[0] == 30) {
		device.normalMode.delayTime = UDPdata.data[1];
		EEpromWrite(30, UDPdata.data[1]);
	}

	//byte 31
	if (UDPdata.data[0] == 31) {
		device.humidityAlertMode.triggerInt = UDPdata.data[1];
		EEpromWrite(31, UDPdata.data[1]);
	}

	//byte 32
	if (UDPdata.data[0] == 32) {
		device.humidityAlertMode.delayTime = UDPdata.data[1];
		EEpromWrite(32, UDPdata.data[1]);
	}

	//byte 33
	if (UDPdata.data[0] == 33) {
		device.defrostMode.triggerInt = UDPdata.data[1];
		EEpromWrite(33, UDPdata.data[1]);
	}

	//byte 34
	if (UDPdata.data[0] == 34) {
		device.defrostMode.delayTime = UDPdata.data[1];
		EEpromWrite(34, UDPdata.data[1]);
	}

	//byte 35-58
	if ((UDPdata.data[0] >= 35) &&
			(UDPdata.data[0] <= 58)	) {
		int hour = UDPdata.data[0]-35;
		device.activeTempRegByHours[hour].salon   = UDPbitStatus(UDPdata.data[1],7);
		device.activeTempRegByHours[hour].pralnia = UDPbitStatus(UDPdata.data[1],6);
		device.activeTempRegByHours[hour].lazDol = UDPbitStatus(UDPdata.data[1],5);
		device.activeTempRegByHours[hour].rodzice = UDPbitStatus(UDPdata.data[1],4);
		device.activeTempRegByHours[hour].Natalia = UDPbitStatus(UDPdata.data[1],3);
		device.activeTempRegByHours[hour].Karolina = UDPbitStatus(UDPdata.data[1],2);
		device.activeTempRegByHours[hour].lazGora= UDPbitStatus(UDPdata.data[1],1);
		EEpromWrite(UDPdata.data[0], UDPdata.data[1]);
	}

	//byte 59
	if (UDPdata.data[0] == 59) {
		device.minTemp = UDPdata.data[1];
		EEpromWrite(59, UDPdata.data[1]);
	}

	//byte 60-83
	if ((UDPdata.data[0] >= 60) &&
			(UDPdata.data[0] <= 83)	) {
		int hour = UDPdata.data[0]-60;
		device.normalOnByHours[hour].salon = UDPbitStatus(UDPdata.data[1],7);
		device.normalOnByHours[hour].pralnia = UDPbitStatus(UDPdata.data[1],6);
		device.normalOnByHours[hour].lazDol = UDPbitStatus(UDPdata.data[1],5);
		device.normalOnByHours[hour].rodzice = UDPbitStatus(UDPdata.data[1],4);
		device.normalOnByHours[hour].Natalia = UDPbitStatus(UDPdata.data[1],3);
		device.normalOnByHours[hour].Karolina = UDPbitStatus(UDPdata.data[1],2);
		device.normalOnByHours[hour].lazGora= UDPbitStatus(UDPdata.data[1],1);
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

	//Test Mode
	setUDPdata();
	forceStandardUDP();
}

void getComfortParams(){
	zones[ID_ZONE_SALON].isTemp = UDPdata.data[0]+(UDPdata.data[1]/10.0);
	zones[ID_ZONE_SALON].reqTemp = UDPdata.data[2]/2.0;
	zones[ID_ZONE_SALON].humidity = UDPdata.data[3];

	zones[ID_ZONE_LAZDOL].isTemp = UDPdata.data[4]+(UDPdata.data[5]/10.0);
	zones[ID_ZONE_LAZDOL].reqTemp = UDPdata.data[6]/2.0;
	zones[ID_ZONE_LAZDOL].humidity = UDPdata.data[7];

	zones[ID_ZONE_PRALNIA].isTemp = UDPdata.data[8]+(UDPdata.data[9]/10.0);
	zones[ID_ZONE_PRALNIA].reqTemp = UDPdata.data[10]/2.0;
	zones[ID_ZONE_PRALNIA].humidity = UDPdata.data[11];

	zones[ID_ZONE_RODZICE].isTemp = UDPdata.data[12]+(UDPdata.data[13]/10.0);
	zones[ID_ZONE_RODZICE].reqTemp = UDPdata.data[14]/2.0;
	zones[ID_ZONE_RODZICE].humidity = UDPdata.data[15];

	zones[ID_ZONE_NATALIA].isTemp = UDPdata.data[16]+(UDPdata.data[17]/10.0);
	zones[ID_ZONE_NATALIA].reqTemp = UDPdata.data[18]/2.0;
	zones[ID_ZONE_NATALIA].humidity = UDPdata.data[19];

	zones[ID_ZONE_KAROLINA].isTemp = UDPdata.data[20]+(UDPdata.data[21]/10.0);
	zones[ID_ZONE_KAROLINA].reqTemp = UDPdata.data[22]/2.0;
	zones[ID_ZONE_KAROLINA].humidity = UDPdata.data[23];

	zones[ID_ZONE_LAZGORA].isTemp = UDPdata.data[24]+(UDPdata.data[25]/10.0);
	zones[ID_ZONE_LAZGORA].reqTemp = UDPdata.data[26]/2.0;
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
	device.normalOn = false;

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

	int time = device.normalMode.endMillis - millis();
	device.normalMode.timeLeft = (int)(time/60000);
	if (device.normalMode.timeLeft<=0)
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
	device.humidityAlert = false;

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

	if (device.humidityAlertMode.trigger) {
		addLog("Humidity trigger");
		device.humidityAlertMode.endMillis = millis() + (device.humidityAlertMode.delayTime*1000*60);
	}
	int time = device.humidityAlertMode.endMillis - millis();
	device.humidityAlertMode.timeLeft = (int)(time/60000);
	if (device.humidityAlertMode.timeLeft<=0)
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

	device.defrostMode.trigger = false;
	device.defrostActive = false;
	if ((device.sensorsBME280[ID_CZERPNIA].pressureHighPrec-device.sensorsBME280[ID_NAWIEW].pressureHighPrec) >= device.defrostMode.triggerInt)
		device.defrostMode.trigger = true;
	if (device.defrostMode.trigger)
		device.defrostMode.endMillis = millis() + (device.defrostMode.delayTime * 1000 * 60);

	int time = device.defrostMode.endMillis - millis();
	device.defrostMode.timeLeft = (int)(time/60000);
	if (device.defrostMode.timeLeft<=0)
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
	else device.zoneReqReg.salon = false;
	if (zones[ID_ZONE_SALON].isTemp<=(zones[ID_ZONE_SALON].reqTemp))
		device.zoneReqReg.salon = false;

	//PRALNIA
	if (device.activeTempRegByHours[getDateTime().hour].pralnia) {
		if (zones[ID_ZONE_PRALNIA].isTemp>=(zones[ID_ZONE_PRALNIA].reqTemp+hist))
			device.zoneReqReg.pralnia = true;
	}
	else device.zoneReqReg.pralnia = false;
	if (zones[ID_ZONE_PRALNIA].isTemp<=(zones[ID_ZONE_PRALNIA].reqTemp))
		device.zoneReqReg.pralnia = false;

	//LAZ.DOL
	if (device.activeTempRegByHours[getDateTime().hour].lazDol) {
		if (zones[ID_ZONE_LAZDOL].isTemp>=(zones[ID_ZONE_LAZDOL].reqTemp+hist))
			device.zoneReqReg.lazDol = true;
	}
	else device.zoneReqReg.lazDol = false;
	if (zones[ID_ZONE_LAZDOL].isTemp<=(zones[ID_ZONE_LAZDOL].reqTemp))
		device.zoneReqReg.lazDol = false;

	//RODZICE
	if (device.activeTempRegByHours[getDateTime().hour].rodzice) {
		if (zones[ID_ZONE_RODZICE].isTemp>=(zones[ID_ZONE_RODZICE].reqTemp+hist))
			device.zoneReqReg.rodzice = true;
	}
	else device.zoneReqReg.rodzice = false;
	if (zones[ID_ZONE_RODZICE].isTemp<=(zones[ID_ZONE_RODZICE].reqTemp))
		device.zoneReqReg.rodzice = false;

	//NATALIA
	if (device.activeTempRegByHours[getDateTime().hour].Natalia) {
		if (zones[ID_ZONE_NATALIA].isTemp>=(zones[ID_ZONE_NATALIA].reqTemp+hist))
			device.zoneReqReg.Natalia = true;
	}
	else device.zoneReqReg.Natalia = false;
	if (zones[ID_ZONE_NATALIA].isTemp<=(zones[ID_ZONE_NATALIA].reqTemp))
		device.zoneReqReg.Natalia = false;

	//KAROLINA
	if (device.activeTempRegByHours[getDateTime().hour].Karolina) {
		if (zones[ID_ZONE_KAROLINA].isTemp>=(zones[ID_ZONE_KAROLINA].reqTemp+hist))
			device.zoneReqReg.Karolina = true;
	}
	else device.zoneReqReg.Karolina = false;
	if (zones[ID_ZONE_KAROLINA].isTemp<=(zones[ID_ZONE_KAROLINA].reqTemp))
		device.zoneReqReg.Karolina = false;

	//LAZ.GORA
	if (device.activeTempRegByHours[getDateTime().hour].lazGora) {
		if (zones[ID_ZONE_LAZGORA].isTemp>=(zones[ID_ZONE_LAZGORA].reqTemp+hist))
			device.zoneReqReg.lazGora = true;
	}
	else device.zoneReqReg.lazGora = false;
	if (zones[ID_ZONE_LAZGORA].isTemp<=(zones[ID_ZONE_LAZGORA].reqTemp))
		device.zoneReqReg.lazGora = false;
}

void activeHeatingMode() {
	if (!device.activeHeating)
		return;

	//Hysteresis
	float hist = device.minTemp;
	//SALON
	if (device.activeTempRegByHours[getDateTime().hour].salon) {
		if (zones[ID_ZONE_SALON].isTemp<=(zones[ID_ZONE_SALON].reqTemp-hist))
			device.zoneReqReg.salon = true;
	}
	else device.zoneReqReg.salon = false;
	if (zones[ID_ZONE_SALON].isTemp>=(zones[ID_ZONE_SALON].reqTemp))
		device.zoneReqReg.salon = false;

	//PRALNIA
	if (device.activeTempRegByHours[getDateTime().hour].pralnia) {
		if (zones[ID_ZONE_PRALNIA].isTemp<=(zones[ID_ZONE_PRALNIA].reqTemp-hist))
			device.zoneReqReg.pralnia = true;
	}
	else device.zoneReqReg.pralnia = false;
	if (zones[ID_ZONE_PRALNIA].isTemp>=(zones[ID_ZONE_PRALNIA].reqTemp))
		device.zoneReqReg.pralnia = false;

	//LAZ.DOL
	if (device.activeTempRegByHours[getDateTime().hour].lazDol) {
		if (zones[ID_ZONE_LAZDOL].isTemp<=(zones[ID_ZONE_LAZDOL].reqTemp-hist))
			device.zoneReqReg.lazDol = true;
	}
	else device.zoneReqReg.lazDol = false;
	if (zones[ID_ZONE_LAZDOL].isTemp>=(zones[ID_ZONE_LAZDOL].reqTemp))
		device.zoneReqReg.lazDol = false;

	//RODZICE
	if (device.activeTempRegByHours[getDateTime().hour].rodzice) {
		if (zones[ID_ZONE_RODZICE].isTemp<=(zones[ID_ZONE_RODZICE].reqTemp-hist))
			device.zoneReqReg.rodzice = true;
	}
	else device.zoneReqReg.rodzice = false;
	if (zones[ID_ZONE_RODZICE].isTemp>=(zones[ID_ZONE_RODZICE].reqTemp))
		device.zoneReqReg.rodzice = false;

	//NATALIA
	if (device.activeTempRegByHours[getDateTime().hour].Natalia) {
		if (zones[ID_ZONE_NATALIA].isTemp<=(zones[ID_ZONE_NATALIA].reqTemp-hist))
			device.zoneReqReg.Natalia = true;
	}
	else device.zoneReqReg.Natalia = false;
	if (zones[ID_ZONE_NATALIA].isTemp>=(zones[ID_ZONE_NATALIA].reqTemp))
		device.zoneReqReg.Natalia = false;

	//KAROLINA
	if (device.activeTempRegByHours[getDateTime().hour].Karolina) {
		if (zones[ID_ZONE_KAROLINA].isTemp<=(zones[ID_ZONE_KAROLINA].reqTemp-hist))
			device.zoneReqReg.Karolina = true;
	}
	else device.zoneReqReg.Karolina = false;
	if (zones[ID_ZONE_KAROLINA].isTemp>=(zones[ID_ZONE_KAROLINA].reqTemp))
		device.zoneReqReg.Karolina = false;

	//LAZ.GORA
	if (device.activeTempRegByHours[getDateTime().hour].lazGora) {
		if (zones[ID_ZONE_LAZGORA].isTemp<=(zones[ID_ZONE_LAZGORA].reqTemp-hist))
			device.zoneReqReg.lazGora = true;
	}
	else device.zoneReqReg.lazGora = false;
	if (zones[ID_ZONE_LAZGORA].isTemp>=(zones[ID_ZONE_LAZGORA].reqTemp))
		device.zoneReqReg.lazGora = false;
}

void bypass() {
	if (device.defrostMode.timeLeft > 0) {
		device.bypassOpen = true;
		return;
	}
	if ((device.activeCooling) && (device.fan[FAN_CZERPNIA].speed>0)) {
		if (device.sensorsBME280[ID_CZERPNIA].temperature<device.sensorsBME280[ID_WYWIEW].temperature)
			device.bypassOpen = true;
		if (device.sensorsBME280[ID_CZERPNIA].temperature>=(device.sensorsBME280[ID_WYWIEW].temperature+0.5f))
			device.bypassOpen = false;
		return;
	}

	device.bypassOpen = false;
}

void flaps() {
	//TODO
	device.flapFresh.salon1 = false;
	device.flapFresh.salon2 = false;
	device.flapFresh.gabinet = false;
	device.flapFresh.warsztat = false;
	device.flapFresh.rodzice = false;
	device.flapFresh.natalia = false;
	device.flapFresh.karolina = false;

	device.flapUsed.kuchnia = false;
	device.flapUsed.pralnia = false;
	device.flapUsed.przedpokoj = false;
	device.flapUsed.lazDol1 = false;
	device.flapUsed.lazDol2 = false;
	device.flapUsed.garderoba = false;
	device.flapUsed.lazGora1 = false;
	device.flapUsed.lazGora2 = false;

	if (device.zoneReqReg.salon) {
		device.flapFresh.salon1 = true;
		device.flapFresh.salon2 = true;
		device.flapUsed.kuchnia = true;
	}
	if (device.zoneReqReg.pralnia) {
		device.flapFresh.warsztat = true;
		device.flapUsed.pralnia= true;
	}
	if (device.zoneReqReg.lazDol) {
		device.flapFresh.salon1 = true;
		device.flapFresh.salon2 = true;
		device.flapFresh.gabinet = true;
		device.flapUsed.lazDol1= true;
		device.flapUsed.lazDol2= true;
	}
	if (device.zoneReqReg.pralnia) {
		device.flapFresh.warsztat = true;
		device.flapUsed.pralnia= true;
	}
	if (device.zoneReqReg.rodzice) {
		device.flapFresh.rodzice = true;
		device.flapUsed.garderoba= true;

	}
	if (device.zoneReqReg.Natalia) {
		device.flapFresh.natalia = true;
		device.flapUsed.lazGora1= true;
		device.flapUsed.lazGora2= true;
	}
	if (device.zoneReqReg.Karolina) {
		device.flapFresh.karolina = true;
		device.flapUsed.lazGora1= true;
		device.flapUsed.lazGora2= true;
	}
	if (device.zoneReqReg.lazGora) {
		device.flapFresh.natalia = true;
		device.flapFresh.karolina = true;
		device.flapUsed.lazGora1= true;
		device.flapUsed.lazGora2= true;
	}
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
	if (device.circuitPump) {
		device.fan[FAN_CZERPNIA].speed = 60;
	}
	if (device.normalMode.timeLeft>0) {
		device.fan[FAN_CZERPNIA].speed = 50;
		device.fan[FAN_WYWIEW].speed = 50;
	}
	if (device.humidityAlertMode.timeLeft>0) {
//		device.fan[FAN_CZERPNIA].speed = 75;
		device.fan[FAN_WYWIEW].speed = 75;
	}
//	if (device.defrostMode.timeLeft>0) {
//		device.fan[FAN_CZERPNIA].speed = 80;
//		device.fan[FAN_WYWIEW].speed = 80;
//	}
	if (device.humidityAlertMode.turbo) {
//		device.fan[FAN_CZERPNIA].speed = 100;
		device.fan[FAN_WYWIEW].speed = 100;
	}
}

void circuitPump() {
	device.circuitPump = false;
	if (device.zoneReqReg.salon 		||
			device.zoneReqReg.pralnia 	||
			device.zoneReqReg.lazDol 	||
			device.zoneReqReg.rodzice 	||
			device.zoneReqReg.Natalia 	||
			device.zoneReqReg.Karolina 	||
			device.zoneReqReg.lazGora)
		device.circuitPump = true;

	device.reqPumpColdWater = false;
	device.reqPumpHotWater = false;
	if (device.circuitPump) {
		if (device.activeCooling)
			device.reqPumpColdWater = true;
		else if (device.activeHeating)
			device.reqPumpHotWater = true;
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
	if (device.bypassOpen) device.byppass.dutyCycle =5;
	else device.byppass.dutyCycle =25;

	if (device.byppass.lastDutyCycle != device.byppass.dutyCycle) {
		device.byppass.lastDutyCycle = device.byppass.dutyCycle;
		ledcAttachPin(PIN_SERVO, SERVO_CHANNEL);
		device.byppass.attached = true;
		ledcWrite(SERVO_CHANNEL, device.byppass.dutyCycle);
		//hold servo for 5s
		device.byppass.endMillis = millis()+5000;
	}

	if ((millis()>device.byppass.endMillis) && (device.byppass.attached)) {
		ledcDetachPin(PIN_SERVO);
		device.byppass.attached = false;
	}

	//Circuit pump
	digitalWrite(PIN_CIRCUIT_PUMP, !device.circuitPump);

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
	int size = 89;
	byte dataWrite[size];
	// First three bytes are reserved for device recognized purposes.
	dataWrite[0] = (device.humidityAlert<< 7) | (device.bypassOpen << 6) | (device.circuitPump<< 5) | (device.reqPumpColdWater << 4) | (device.reqPumpHotWater << 3) | (device.defrostActive << 2) | (device.reqAutoDiagnosis << 0);
	dataWrite[1] = (device.normalOn<< 7) | (device.activeCooling << 6) | (device.activeHeating<< 5) | (device.reqLazDol << 4) | (device.reqLazGora << 3) | (device.reqKuchnia << 2);
	dataWrite[2] = get10Temp(device.sensorsBME280[ID_CZERPNIA].temperature);
	dataWrite[3] = get01Temp(device.sensorsBME280[ID_CZERPNIA].temperature);
	dataWrite[4] = device.sensorsBME280[ID_CZERPNIA].humidity;
	dataWrite[5] = (int)(device.sensorsBME280[ID_CZERPNIA].pressure/10);
	dataWrite[6] = get10Temp(device.sensorsBME280[ID_WYRZUTNIA].temperature);
	dataWrite[7] = get01Temp(device.sensorsBME280[ID_WYRZUTNIA].temperature);
	dataWrite[8] = device.sensorsBME280[ID_WYRZUTNIA].humidity;
	dataWrite[9] = (int)(device.sensorsBME280[ID_WYRZUTNIA].pressure/10);
	dataWrite[10] = get10Temp(device.sensorsBME280[ID_NAWIEW].temperature);
	dataWrite[11] = get01Temp(device.sensorsBME280[ID_NAWIEW].temperature);
	dataWrite[12] = device.sensorsBME280[ID_NAWIEW].humidity;
	dataWrite[13] = (int)(device.sensorsBME280[ID_NAWIEW].pressure/10);
	dataWrite[14] = get10Temp(device.sensorsBME280[ID_WYWIEW].temperature);
	dataWrite[15] = get01Temp(device.sensorsBME280[ID_WYWIEW].temperature);
	dataWrite[16] = device.sensorsBME280[ID_WYWIEW].humidity;
	dataWrite[17] = (int)(device.sensorsBME280[ID_WYWIEW].pressure/10);
	dataWrite[18] = device.fan[FAN_CZERPNIA].speed;
	dataWrite[19] = (int)(device.fan[FAN_CZERPNIA].rev/100);
	dataWrite[20] = device.fan[FAN_WYWIEW].speed;
	dataWrite[21] = (int)(device.fan[FAN_WYWIEW].rev/100);
	dataWrite[22] = get10Temp(device.heatExchanger[ID_WATER_INLET]);
	dataWrite[23] = get01Temp(device.heatExchanger[ID_WATER_INLET]);
	dataWrite[24] = get10Temp(device.heatExchanger[ID_WATER_OUTLET]);
	dataWrite[25] = get01Temp(device.heatExchanger[ID_WATER_OUTLET]);
	dataWrite[26] = get10Temp(device.heatExchanger[ID_AIR_INTAKE]);
	dataWrite[27] = get01Temp(device.heatExchanger[ID_AIR_INTAKE]);
	dataWrite[28] = get10Temp(device.heatExchanger[ID_AIR_OUTLET]);
	dataWrite[29] = get01Temp(device.heatExchanger[ID_AIR_OUTLET]);
	dataWrite[30] = device.normalMode.delayTime;
	dataWrite[31] = device.humidityAlertMode.triggerInt;
	dataWrite[32] = device.humidityAlertMode.delayTime;
	dataWrite[33] = device.defrostMode.triggerInt;
	dataWrite[34] = device.defrostMode.delayTime;

	for (int i=35; i<=58; i++) {
		int hour = i-35;
		dataWrite[i] = (device.activeTempRegByHours[hour].salon<< 7) | (device.activeTempRegByHours[hour].pralnia << 6) | (device.activeTempRegByHours[hour].lazDol<< 5) |
				(device.activeTempRegByHours[hour].rodzice << 4) | (device.activeTempRegByHours[hour].Natalia << 3) | (device.activeTempRegByHours[hour].Karolina << 2) | (device.activeTempRegByHours[hour].lazGora << 1);
	}
	dataWrite[59] = device.minTemp;

	for (int i=60; i<=83; i++) {
		int hour = i-60;
		dataWrite[i] = (device.normalOnByHours[hour].salon<< 7) | (device.normalOnByHours[hour].pralnia << 6) | (device.normalOnByHours[hour].lazDol<< 5) |
				(device.normalOnByHours[hour].rodzice << 4) | (device.normalOnByHours[hour].Natalia << 3) | (device.normalOnByHours[hour].Karolina << 2) | (device.normalOnByHours[hour].lazGora << 1);
	}
	dataWrite[84] = device.normalMode.timeLeft;
	dataWrite[85] = device.humidityAlertMode.timeLeft;
	dataWrite[86] = device.defrostMode.timeLeft;
	dataWrite[87] = (device.flapFresh.salon1<< 7) | (device.flapFresh.salon2 << 6) | (device.flapFresh.gabinet<< 5) | (device.flapFresh.warsztat << 4) | (device.flapFresh.rodzice << 3) | (device.flapFresh.natalia<< 2) | (device.flapFresh.karolina << 1);
	dataWrite[88] = (device.flapUsed.kuchnia<< 7) | (device.flapUsed.lazDol1 << 6) | (device.flapUsed.lazDol2<< 5) | (device.flapUsed.pralnia << 4) | (device.flapUsed.przedpokoj << 3) | (device.flapUsed.garderoba<< 2) | (device.flapUsed.lazGora1 << 1)  | (device.flapUsed.lazGora2 << 0);
	setUDPdata(0, dataWrite,size);
}

void statusUpdate() {
	String status;
	status = "\nOG粌NE\n";
	status += addStatus("HumAlert", device.humidityAlert);
	status += addStatus("BypassOpen", device.bypassOpen);
	status += addStatus("pompa", device.circuitPump);
	status += addStatus("reqPumpColdWater", device.reqPumpColdWater);
	status += addStatus("reqPumpHotWater", device.reqPumpHotWater);
	status += addStatus("defrostActive", device.defrostActive);
	status += addStatus("reqAutoDiagnosis", device.reqAutoDiagnosis);
	status += "\nOG粌NE\n";
	status += addStatus("normalOn", device.normalOn);
	status += addStatus("activeCooling", device.activeCooling);
	status += addStatus("activeHeating", device.activeHeating);
	status += addStatus("reqLazDol", device.reqLazDol);
	status += addStatus("reqLazGora", device.reqLazGora);
	status += addStatus("reqKuchnia", device.reqKuchnia);

	status += "\nBME280";
	status += "\nCZERPNIA\t";
	status += addStatus("T", device.sensorsBME280[ID_CZERPNIA].temperature, "stC");
	status += addStatus("H", device.sensorsBME280[ID_CZERPNIA].humidity, "%");
	status += addStatus("\tP", device.sensorsBME280[ID_CZERPNIA].pressureHighPrec, "hPa*100");
	status += addStatus("Fault", device.sensorsBME280[ID_CZERPNIA].faultyReadings, "");
	status += "\nWYRZUTNIA";
	status += addStatus("T", device.sensorsBME280[ID_WYRZUTNIA].temperature, "stC");
	status += addStatus("H", device.sensorsBME280[ID_WYRZUTNIA].humidity, "%");
	status += addStatus("\tP", device.sensorsBME280[ID_WYRZUTNIA].pressureHighPrec, "hPa*100");
	status += addStatus("Fault", device.sensorsBME280[ID_WYRZUTNIA].faultyReadings, "");
	status += "\nNAWIEW\t\t";
	status += addStatus("T", device.sensorsBME280[ID_NAWIEW].temperature, "stC");
	status += addStatus("H", device.sensorsBME280[ID_NAWIEW].humidity, "%");
	status += addStatus("\tP", device.sensorsBME280[ID_NAWIEW].pressureHighPrec, "hPa*100");
	status += addStatus("Fault", device.sensorsBME280[ID_NAWIEW].faultyReadings, "");
	status += "\nWYWIEW\t\t";
	status += addStatus("T", device.sensorsBME280[ID_WYWIEW].temperature, "stC");
	status += addStatus("H", device.sensorsBME280[ID_WYWIEW].humidity, "%");
	status += addStatus("\tP", device.sensorsBME280[ID_WYWIEW].pressureHighPrec, "hPa*100");
	status += addStatus("Fault", device.sensorsBME280[ID_WYWIEW].faultyReadings, "");

	status += "\nWENTYLATORY\n";
	status += "\nCZERPNIA\t";
	status += addStatus("Speed", device.fan[FAN_CZERPNIA].speed, "%");
	status += addStatus("Rev", device.fan[FAN_CZERPNIA].rev, "min-1");
	status += "\nWYWIEW\t";
	status += addStatus("Speed", device.fan[FAN_WYWIEW].speed, "%");
	status += addStatus("Rev", device.fan[FAN_WYWIEW].rev, "min-1");

	status += "\nWYMIENNIK";
	status += addStatus("Water in", device.heatExchanger[ID_WATER_INLET], "stC");
	status += addStatus("Water out", device.heatExchanger[ID_WATER_OUTLET], "stC");
	status += addStatus("Air in", device.heatExchanger[ID_AIR_INTAKE], "stC");
	status += addStatus("Air out", device.heatExchanger[ID_AIR_OUTLET], "stC");

	status += "\nKLAPY NAWIEW";
	status += addStatus("salon1", device.flapFresh.salon1);
	status += addStatus("salon2", device.flapFresh.salon2);
	status += addStatus("gabinet", device.flapFresh.gabinet);
	status += addStatus("warsztat", device.flapFresh.warsztat);
	status += addStatus("rodzice", device.flapFresh.rodzice);
	status += addStatus("natalia", device.flapFresh.natalia);
	status += addStatus("karolina", device.flapFresh.karolina);

	status += "\nKLAPY WYWIEW";
	status += addStatus("kuchnia", device.flapUsed.kuchnia);
	status += addStatus("lazDol1", device.flapUsed.lazDol1);
	status += addStatus("lazDol2", device.flapUsed.lazDol2);
	status += addStatus("pralnia", device.flapUsed.pralnia);
	status += addStatus("przedpokoj", device.flapUsed.przedpokoj);
	status += addStatus("garderoba", device.flapUsed.garderoba);
	status += addStatus("lazGora1", device.flapUsed.lazGora1);
	status += addStatus("lazGora2", device.flapUsed.lazGora2);

	status += "\nTRYBY";
	status += "\nNormalOn\t\t";
	status += addStatus("trig", device.normalMode.trigger);
	status += addStatus("trigInt", device.normalMode.triggerInt, "");
	status += addStatus("turbo", device.normalMode.turbo);
	status += addStatus("podtrzymanie", device.normalMode.delayTime, "min");
	status += addStatus("pozosta這", device.normalMode.timeLeft, "min");

	status += "\nHumidity Alert\t";
	status += addStatus("trig", device.humidityAlertMode.trigger);
	status += addStatus("trigInt", device.humidityAlertMode.triggerInt, "%");
	status += addStatus("turbo", device.humidityAlertMode.turbo);
	status += addStatus("podtrzymanie", device.humidityAlertMode.delayTime, "min");
	status += addStatus("pozosta這", device.humidityAlertMode.timeLeft, "min");

	status += "\nDefrost\t\t";
	status += addStatus("trig", device.defrostMode.trigger);
	status += addStatus("trigInt", device.defrostMode.triggerInt, "hPa");
	status += addStatus("turbo", device.defrostMode.turbo);
	status += addStatus("podtrzymanie", device.defrostMode.delayTime, "min");
	status += addStatus("pozosta這", device.defrostMode.timeLeft, "min");

	status += "\nAktywne ch這dzenie/ogrzewanie";
	status += "\n\t\tSalon Pralnia LazDol\tRodzice Natalia Karolina LazGora";
	for (int i=0; i<24; i++) {
		status +="\nh=";
		status +=i;
		status +="\t";
		status +=device.activeTempRegByHours[i].salon;
		status +="\t\t\t";
		status +=device.activeTempRegByHours[i].pralnia;
		status +="\t\t\t";
		status +=device.activeTempRegByHours[i].lazDol;
		status +="\t\t\t";
		status +=device.activeTempRegByHours[i].rodzice;
		status +="\t\t\t";
		status +=device.activeTempRegByHours[i].Natalia;
		status +="\t\t\t";
		status +=device.activeTempRegByHours[i].Karolina;
		status +="\t\t\t";
		status +=device.activeTempRegByHours[i].lazGora;
		status +="\t\t\t";
	}

	status += addStatus("\nHistereza T", device.minTemp, "stC");

	status += "\nNormal On matryca";
	status += "\n\t\tSalon Pralnia LazDol\tRodzice Natalia Karolina LazGora";
	for (int i=0; i<24; i++) {
		status +="\nh=";
		status +=i;
		status +="\t";
		status +=device.normalOnByHours[i].salon;
		status +="\t\t\t";
		status +=device.normalOnByHours[i].pralnia;
		status +="\t\t\t";
		status +=device.normalOnByHours[i].lazDol;
		status +="\t\t\t";
		status +=device.normalOnByHours[i].rodzice;
		status +="\t\t\t";
		status +=device.normalOnByHours[i].Natalia;
		status +="\t\t\t";
		status +=device.normalOnByHours[i].Karolina;
		status +="\t\t\t";
		status +=device.normalOnByHours[i].lazGora;
		status +="\t\t\t";
	}

	status +="\n\nHand mode";
	status +="\nEnabled="; status +=handMode.enabled;status +="\tfanSpeed="; status +=handMode.fanSpeed;status +="[%]\t bypassOpen="; status +=handMode.byPassOpen;
	for (int i=0; i<7; i++) {
		status +="\nZone["; status +=i; status += "]:\t\t T="; status +=zones[i].isTemp; status +="[stC]\treqT="; status+=zones[i].reqTemp; status +="[stC]\tH="; status +=zones[i].humidity;
	}

	status +="\nAir\tPM2.5="; status+=airPollution.pm25; status +="[ug/m3]\t PM10="; status +=airPollution.pm10; status+="[ug/m3]\n";
	setStatus(status);
}




