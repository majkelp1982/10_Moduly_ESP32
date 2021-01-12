#include "Module.h"

Device device;
Zone zones[7];
DataRead UDPdata;
Servo servo;

//Functions
void firstScan();
void readSensors();
void readUDPdata();
void getMasterDeviceOrder();
void getComfortParams();

void normalMode();
void humidityAllert();
void defrost();
void bypass();
void fan();

void outputs();
void setUDPdata();
void statusUpdate();

//help functions
int get10Temp(float temp);
int get01Temp(float temp);

//Variables
Adafruit_BME280 bme1(CS_BME280_CZERPNIA, SPI_MOSI, SPI_MISO, SPI_SCK);
Adafruit_BME280 bme2(CS_BME280_WYRZUTNIA, SPI_MOSI, SPI_MISO, SPI_SCK);
Adafruit_BME280 bme3(CS_BME280_NAWIEW, SPI_MOSI, SPI_MISO, SPI_SCK);
Adafruit_BME280 bme4(CS_BME280_WYWIEW, SPI_MOSI, SPI_MISO, SPI_SCK);

//Fans
int fanRev1 = 0;
boolean release1  = true;
int fanRev2 = 0;
boolean release2  = true;

//Delays
unsigned long readSensorMillis = 0;
unsigned long lastRevsRead = 0;
unsigned long defrostEndMillis = 0;

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
	servo.attach(PIN_BYPASS);

	//Fan initialization
	ledcSetup(PWM_CHANNEL, PWM_FREQUENCY, PWM_RESOUTION);
	ledcAttachPin(PIN_FAN_PWM, PWM_CHANNEL);
	pinMode(PIN_FAN1_REVS, INPUT_PULLUP);
	digitalWrite(PIN_FAN1_REVS, HIGH);
	pinMode(PIN_FAN2_REVS, INPUT_PULLUP);
	digitalWrite(PIN_FAN2_REVS, HIGH);
}

void module() {
	//Input data
	readSensors();
	readUDPdata();

	//Module events
	normalMode();
	humidityAllert();
	defrost();
	fan();

	//Output settings
	outputs();
	setUDPdata();
	statusUpdate();
}

void firstScan() {
	// Bytes coding EEPROM //
	// 0 - 11 		: 	device.hour[0] .. device.hour[11]

	//Get data from EEprom
	int size = 12;
	byte EEpromData[size];
	EEpromScan(EEpromData, size);
	Serial.println("\nFirst scan");
	for (int i=0; i<size; i++) {
		byte data = EEpromData[i];
		device.hour[i] = data;
	}
}

void readSensors() {
	if (sleep(&readSensorMillis, 5)) return;
	for (int i=0; i<4; i++) {
		device.sensorsBME280[i].temperature = device.sensorsBME280[i].interface.readTemperature();
		device.sensorsBME280[i].pressure = (int)(device.sensorsBME280[i].interface.readPressure()/100);
		device.sensorsBME280[i].humidity = (int)device.sensorsBME280[i].interface.readHumidity();
		if (device.sensorsBME280[i].temperature>35
				|| device.sensorsBME280[i].temperature<10
				|| device.sensorsBME280[i].pressure>1050
				|| device.sensorsBME280[i].pressure<800
				|| device.sensorsBME280[i].humidity>100
				|| device.sensorsBME280[i].humidity<15)
			device.sensorsBME280[i].faultyReadings++;
	}
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

	resetNewData();
}

void getMasterDeviceOrder() {
	//bytes 1-12
	if ((UDPdata.data[0] >= 1)
			&& (UDPdata.data[0] <=12)) {
		int hour = UDPdata.data[0]-1;
		device.hour[hour] = UDPdata.data[1];
		EEpromWrite(hour, UDPdata.data[1]);
	}
	//byte 33
	if (UDPdata.data[0] == 33)
		device.defrost.hPaDiff = UDPdata.data[1]*10;

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

void normalMode() {
	bool normalOn = false;
	DateTime dateTime = getDateTime();
	if (dateTime.hour == 0) {
		if ((dateTime.minute>=0) && (dateTime.minute<15) && (UDPbitStatus(device.hour[0],7))) normalOn = true;
		if ((dateTime.minute>=15) && (dateTime.minute<30) && (UDPbitStatus(device.hour[0],6))) normalOn = true;
		if ((dateTime.minute>=30) && (dateTime.minute<45) && (UDPbitStatus(device.hour[0],5))) normalOn = true;
		if ((dateTime.minute>=45) && (dateTime.minute<60) && (UDPbitStatus(device.hour[0],4))) normalOn = true;
	}
	if (dateTime.hour == 1) {
		if ((dateTime.minute>=0) && (dateTime.minute<15) && (UDPbitStatus(device.hour[0],3))) normalOn = true;
		if ((dateTime.minute>=15) && (dateTime.minute<30) && (UDPbitStatus(device.hour[0],2))) normalOn = true;
		if ((dateTime.minute>=30) && (dateTime.minute<45) && (UDPbitStatus(device.hour[0],1))) normalOn = true;
		if ((dateTime.minute>=45) && (dateTime.minute<60) && (UDPbitStatus(device.hour[0],0))) normalOn = true;
	}
	if (dateTime.hour == 2) {
		if ((dateTime.minute>=0) && (dateTime.minute<15) && (UDPbitStatus(device.hour[1],7))) normalOn = true;
		if ((dateTime.minute>=15) && (dateTime.minute<30) && (UDPbitStatus(device.hour[1],6))) normalOn = true;
		if ((dateTime.minute>=30) && (dateTime.minute<45) && (UDPbitStatus(device.hour[1],5))) normalOn = true;
		if ((dateTime.minute>=45) && (dateTime.minute<60) && (UDPbitStatus(device.hour[1],4))) normalOn = true;
	}
	if (dateTime.hour == 3) {
		if ((dateTime.minute>=0) && (dateTime.minute<15) && (UDPbitStatus(device.hour[1],3))) normalOn = true;
		if ((dateTime.minute>=15) && (dateTime.minute<30) && (UDPbitStatus(device.hour[1],2))) normalOn = true;
		if ((dateTime.minute>=30) && (dateTime.minute<45) && (UDPbitStatus(device.hour[1],1))) normalOn = true;
		if ((dateTime.minute>=45) && (dateTime.minute<60) && (UDPbitStatus(device.hour[1],0))) normalOn = true;
	}
	if (dateTime.hour == 4) {
		if ((dateTime.minute>=0) && (dateTime.minute<15) && (UDPbitStatus(device.hour[2],7))) normalOn = true;
		if ((dateTime.minute>=15) && (dateTime.minute<30) && (UDPbitStatus(device.hour[2],6))) normalOn = true;
		if ((dateTime.minute>=30) && (dateTime.minute<45) && (UDPbitStatus(device.hour[2],5))) normalOn = true;
		if ((dateTime.minute>=45) && (dateTime.minute<60) && (UDPbitStatus(device.hour[2],4))) normalOn = true;
	}
	if (dateTime.hour == 5) {
		if ((dateTime.minute>=0) && (dateTime.minute<15) && (UDPbitStatus(device.hour[2],3))) normalOn = true;
		if ((dateTime.minute>=15) && (dateTime.minute<30) && (UDPbitStatus(device.hour[2],2))) normalOn = true;
		if ((dateTime.minute>=30) && (dateTime.minute<45) && (UDPbitStatus(device.hour[2],1))) normalOn = true;
		if ((dateTime.minute>=45) && (dateTime.minute<60) && (UDPbitStatus(device.hour[2],0))) normalOn = true;
	}
	if (dateTime.hour == 6) {
		if ((dateTime.minute>=0) && (dateTime.minute<15) && (UDPbitStatus(device.hour[3],7))) normalOn = true;
		if ((dateTime.minute>=15) && (dateTime.minute<30) && (UDPbitStatus(device.hour[3],6))) normalOn = true;
		if ((dateTime.minute>=30) && (dateTime.minute<45) && (UDPbitStatus(device.hour[3],5))) normalOn = true;
		if ((dateTime.minute>=45) && (dateTime.minute<60) && (UDPbitStatus(device.hour[3],4))) normalOn = true;
	}
	if (dateTime.hour == 7) {
		if ((dateTime.minute>=0) && (dateTime.minute<15) && (UDPbitStatus(device.hour[3],3))) normalOn = true;
		if ((dateTime.minute>=15) && (dateTime.minute<30) && (UDPbitStatus(device.hour[3],2))) normalOn = true;
		if ((dateTime.minute>=30) && (dateTime.minute<45) && (UDPbitStatus(device.hour[3],1))) normalOn = true;
		if ((dateTime.minute>=45) && (dateTime.minute<60) && (UDPbitStatus(device.hour[3],0))) normalOn = true;
	}
	if (dateTime.hour == 8) {
		if ((dateTime.minute>=0) && (dateTime.minute<15) && (UDPbitStatus(device.hour[4],7))) normalOn = true;
		if ((dateTime.minute>=15) && (dateTime.minute<30) && (UDPbitStatus(device.hour[4],6))) normalOn = true;
		if ((dateTime.minute>=30) && (dateTime.minute<45) && (UDPbitStatus(device.hour[4],5))) normalOn = true;
		if ((dateTime.minute>=45) && (dateTime.minute<60) && (UDPbitStatus(device.hour[4],4))) normalOn = true;
	}
	if (dateTime.hour == 9) {
		if ((dateTime.minute>=0) && (dateTime.minute<15) && (UDPbitStatus(device.hour[4],3))) normalOn = true;
		if ((dateTime.minute>=15) && (dateTime.minute<30) && (UDPbitStatus(device.hour[4],2))) normalOn = true;
		if ((dateTime.minute>=30) && (dateTime.minute<45) && (UDPbitStatus(device.hour[4],1))) normalOn = true;
		if ((dateTime.minute>=45) && (dateTime.minute<60) && (UDPbitStatus(device.hour[4],0))) normalOn = true;
	}
	if (dateTime.hour == 10) {
		if ((dateTime.minute>=0) && (dateTime.minute<15) && (UDPbitStatus(device.hour[5],7))) normalOn = true;
		if ((dateTime.minute>=15) && (dateTime.minute<30) && (UDPbitStatus(device.hour[5],6))) normalOn = true;
		if ((dateTime.minute>=30) && (dateTime.minute<45) && (UDPbitStatus(device.hour[5],5))) normalOn = true;
		if ((dateTime.minute>=45) && (dateTime.minute<60) && (UDPbitStatus(device.hour[5],4))) normalOn = true;
	}
	if (dateTime.hour == 11) {
		if ((dateTime.minute>=0) && (dateTime.minute<15) && (UDPbitStatus(device.hour[5],3))) normalOn = true;
		if ((dateTime.minute>=15) && (dateTime.minute<30) && (UDPbitStatus(device.hour[5],2))) normalOn = true;
		if ((dateTime.minute>=30) && (dateTime.minute<45) && (UDPbitStatus(device.hour[5],1))) normalOn = true;
		if ((dateTime.minute>=45) && (dateTime.minute<60) && (UDPbitStatus(device.hour[5],0))) normalOn = true;
	}
	if (dateTime.hour == 12) {
		if ((dateTime.minute>=0) && (dateTime.minute<15) && (UDPbitStatus(device.hour[6],7))) normalOn = true;
		if ((dateTime.minute>=15) && (dateTime.minute<30) && (UDPbitStatus(device.hour[6],6))) normalOn = true;
		if ((dateTime.minute>=30) && (dateTime.minute<45) && (UDPbitStatus(device.hour[6],5))) normalOn = true;
		if ((dateTime.minute>=45) && (dateTime.minute<60) && (UDPbitStatus(device.hour[6],4))) normalOn = true;
	}
	if (dateTime.hour == 13) {
		if ((dateTime.minute>=0) && (dateTime.minute<15) && (UDPbitStatus(device.hour[6],3))) normalOn = true;
		if ((dateTime.minute>=15) && (dateTime.minute<30) && (UDPbitStatus(device.hour[6],2))) normalOn = true;
		if ((dateTime.minute>=30) && (dateTime.minute<45) && (UDPbitStatus(device.hour[6],1))) normalOn = true;
		if ((dateTime.minute>=45) && (dateTime.minute<60) && (UDPbitStatus(device.hour[6],0))) normalOn = true;
	}
	if (dateTime.hour == 14) {
		if ((dateTime.minute>=0) && (dateTime.minute<15) && (UDPbitStatus(device.hour[7],7))) normalOn = true;
		if ((dateTime.minute>=15) && (dateTime.minute<30) && (UDPbitStatus(device.hour[7],6))) normalOn = true;
		if ((dateTime.minute>=30) && (dateTime.minute<45) && (UDPbitStatus(device.hour[7],5))) normalOn = true;
		if ((dateTime.minute>=45) && (dateTime.minute<60) && (UDPbitStatus(device.hour[7],4))) normalOn = true;
	}
	if (dateTime.hour == 15) {
		if ((dateTime.minute>=0) && (dateTime.minute<15) && (UDPbitStatus(device.hour[7],3))) normalOn = true;
		if ((dateTime.minute>=15) && (dateTime.minute<30) && (UDPbitStatus(device.hour[7],2))) normalOn = true;
		if ((dateTime.minute>=30) && (dateTime.minute<45) && (UDPbitStatus(device.hour[7],1))) normalOn = true;
		if ((dateTime.minute>=45) && (dateTime.minute<60) && (UDPbitStatus(device.hour[7],0))) normalOn = true;
	}
	if (dateTime.hour == 16) {
		if ((dateTime.minute>=0) && (dateTime.minute<15) && (UDPbitStatus(device.hour[8],7))) normalOn = true;
		if ((dateTime.minute>=15) && (dateTime.minute<30) && (UDPbitStatus(device.hour[8],6))) normalOn = true;
		if ((dateTime.minute>=30) && (dateTime.minute<45) && (UDPbitStatus(device.hour[8],5))) normalOn = true;
		if ((dateTime.minute>=45) && (dateTime.minute<60) && (UDPbitStatus(device.hour[8],4))) normalOn = true;
	}
	if (dateTime.hour == 17) {
		if ((dateTime.minute>=0) && (dateTime.minute<15) && (UDPbitStatus(device.hour[8],3))) normalOn = true;
		if ((dateTime.minute>=15) && (dateTime.minute<30) && (UDPbitStatus(device.hour[8],2))) normalOn = true;
		if ((dateTime.minute>=30) && (dateTime.minute<45) && (UDPbitStatus(device.hour[8],1))) normalOn = true;
		if ((dateTime.minute>=45) && (dateTime.minute<60) && (UDPbitStatus(device.hour[8],0))) normalOn = true;
	}
	if (dateTime.hour == 18) {
		if ((dateTime.minute>=0) && (dateTime.minute<15) && (UDPbitStatus(device.hour[9],7))) normalOn = true;
		if ((dateTime.minute>=15) && (dateTime.minute<30) && (UDPbitStatus(device.hour[9],6))) normalOn = true;
		if ((dateTime.minute>=30) && (dateTime.minute<45) && (UDPbitStatus(device.hour[9],5))) normalOn = true;
		if ((dateTime.minute>=45) && (dateTime.minute<60) && (UDPbitStatus(device.hour[9],4))) normalOn = true;
	}
	if (dateTime.hour == 19) {
		if ((dateTime.minute>=0) && (dateTime.minute<15) && (UDPbitStatus(device.hour[9],3))) normalOn = true;
		if ((dateTime.minute>=15) && (dateTime.minute<30) && (UDPbitStatus(device.hour[9],2))) normalOn = true;
		if ((dateTime.minute>=30) && (dateTime.minute<45) && (UDPbitStatus(device.hour[9],1))) normalOn = true;
		if ((dateTime.minute>=45) && (dateTime.minute<60) && (UDPbitStatus(device.hour[9],0))) normalOn = true;
	}
	if (dateTime.hour == 20) {
		if ((dateTime.minute>=0) && (dateTime.minute<15) && (UDPbitStatus(device.hour[10],7))) normalOn = true;
		if ((dateTime.minute>=15) && (dateTime.minute<30) && (UDPbitStatus(device.hour[10],6))) normalOn = true;
		if ((dateTime.minute>=30) && (dateTime.minute<45) && (UDPbitStatus(device.hour[10],5))) normalOn = true;
		if ((dateTime.minute>=45) && (dateTime.minute<60) && (UDPbitStatus(device.hour[10],4))) normalOn = true;
	}
	if (dateTime.hour == 21) {
		if ((dateTime.minute>=0) && (dateTime.minute<15) && (UDPbitStatus(device.hour[10],3))) normalOn = true;
		if ((dateTime.minute>=15) && (dateTime.minute<30) && (UDPbitStatus(device.hour[10],2))) normalOn = true;
		if ((dateTime.minute>=30) && (dateTime.minute<45) && (UDPbitStatus(device.hour[10],1))) normalOn = true;
		if ((dateTime.minute>=45) && (dateTime.minute<60) && (UDPbitStatus(device.hour[10],0))) normalOn = true;
	}
	if (dateTime.hour == 22) {
		if ((dateTime.minute>=0) && (dateTime.minute<15) && (UDPbitStatus(device.hour[11],7))) normalOn = true;
		if ((dateTime.minute>=15) && (dateTime.minute<30) && (UDPbitStatus(device.hour[11],6))) normalOn = true;
		if ((dateTime.minute>=30) && (dateTime.minute<45) && (UDPbitStatus(device.hour[11],5))) normalOn = true;
		if ((dateTime.minute>=45) && (dateTime.minute<60) && (UDPbitStatus(device.hour[11],4))) normalOn = true;
	}
	if (dateTime.hour == 23) {
		if ((dateTime.minute>=0) && (dateTime.minute<15) && (UDPbitStatus(device.hour[11],3))) normalOn = true;
		if ((dateTime.minute>=15) && (dateTime.minute<30) && (UDPbitStatus(device.hour[11],2))) normalOn = true;
		if ((dateTime.minute>=30) && (dateTime.minute<45) && (UDPbitStatus(device.hour[11],1))) normalOn = true;
		if ((dateTime.minute>=45) && (dateTime.minute<60) && (UDPbitStatus(device.hour[11],0))) normalOn = true;
	}
	device.normalON = normalOn;
}

void humidityAllert() {
	//TODO
	device.humidityAlert = false;
}

void defrost() {
	unsigned long currentMillis = millis();
	device.defrost.timeLeft = (int)((defrostEndMillis - currentMillis)/60000);
	if ((device.defrost.timeLeft<0) || !device.defrost.req) device.defrost.timeLeft = 0;

	// reset defrosting after process time run out
	if (device.defrost.req && device.defrost.timeLeft<=0)
		device.defrost.req = false;
	// if temperature over 0, no risk to recu frozen
	if (device.sensorsBME280[ID_CZERPNIA].temperature >0) return;
	// if req already true, no need to check again
	if (device.defrost.req) return;

	if (device.sensorsBME280[ID_CZERPNIA].pressure-device.sensorsBME280[ID_NAWIEW].pressure>device.defrost.hPaDiff) {
		device.defrost.req = true;
		defrostEndMillis = currentMillis + (10*60000);
	}
}

void bypass() {
	device.bypassOpen = device.defrost.req;
}

void fan() {
	//revolution counting
	if ((digitalRead(PIN_FAN1_REVS) == LOW) && (release1)) {
		release1 = false;
		fanRev1++;
	}
	if (digitalRead(PIN_FAN1_REVS) == HIGH)
		release1 = true;

	if ((digitalRead(PIN_FAN2_REVS) == LOW) && (release2)) {
		release2 = false;
		fanRev2++;
	}
	if (digitalRead(PIN_FAN2_REVS) == HIGH)
		release2 = true;

	unsigned long currentMillis = millis();
	if ((currentMillis-lastRevsRead)>=10000) {
		lastRevsRead = currentMillis;
		device.fan1revs = (int)(fanRev1*6);
		device.fan2revs = (int)(fanRev2*6);
		fanRev1 = 0;
		fanRev2 = 0;
		//Fan MAX revs 3716
		//Fan MIN revs 0
	}

	device.fanSpeed = 0;
	if (device.normalON)
		device.fanSpeed = 80;
	if (device.humidityAlert)
		device.fanSpeed = 100;
	if (device.defrost.req)
		device.fanSpeed = 50;
}

void outputs() {
	//Bypass
	if (device.bypassOpen) servo.write(0);
	else servo.write(90);

	//Fans
	// parsing 0-100% into 0-255
	int dutyCycle = (int)((device.fanSpeed/100)*255);
	if (dutyCycle<0) dutyCycle = 0;
	if (dutyCycle>255) dutyCycle = 255;
	ledcWrite(PWM_CHANNEL, dutyCycle);
}

void setUDPdata() {
	int size = 34;
	byte dataWrite[size];
	// First three bytes are reserved for device recognized purposes.
	dataWrite[0] = (((device.fanSpeed>0)?1:0)<< 7) | (device.normalON << 6) | (device.humidityAlert<< 5) | (device.bypassOpen << 4) | (device.defrost.req << 3);
	dataWrite[1] = device.hour[0];
	dataWrite[2] = device.hour[1];
	dataWrite[3] = device.hour[2];
	dataWrite[4] = device.hour[3];
	dataWrite[5] = device.hour[4];
	dataWrite[6] = device.hour[5];
	dataWrite[7] = device.hour[6];
	dataWrite[8] = device.hour[7];
	dataWrite[9] = device.hour[8];
	dataWrite[10] = device.hour[9];
	dataWrite[11] = device.hour[10];
	dataWrite[12] = device.hour[11];
	//BMEs
	dataWrite[13] = get10Temp(device.sensorsBME280[ID_CZERPNIA].temperature);
	dataWrite[14] = get01Temp(device.sensorsBME280[ID_CZERPNIA].temperature);
	dataWrite[15] = device.sensorsBME280[ID_CZERPNIA].humidity;
	dataWrite[16] = (int)(device.sensorsBME280[ID_CZERPNIA].pressure/10);

	dataWrite[17] = get10Temp(device.sensorsBME280[ID_WYRZUTNIA].temperature);
	dataWrite[18] = get01Temp(device.sensorsBME280[ID_WYRZUTNIA].temperature);
	dataWrite[19] = device.sensorsBME280[ID_WYRZUTNIA].humidity;
	dataWrite[20] = (int)(device.sensorsBME280[ID_WYRZUTNIA].pressure/10);

	dataWrite[21] = get10Temp(device.sensorsBME280[ID_NAWIEW].temperature);
	dataWrite[22] = get01Temp(device.sensorsBME280[ID_NAWIEW].temperature);
	dataWrite[23] = device.sensorsBME280[ID_NAWIEW].humidity;
	dataWrite[24] = (int)(device.sensorsBME280[ID_NAWIEW].pressure/10);

	dataWrite[25] = get10Temp(device.sensorsBME280[ID_WYWIEW].temperature);
	dataWrite[26] = get01Temp(device.sensorsBME280[ID_WYWIEW].temperature);
	dataWrite[27] = device.sensorsBME280[ID_WYWIEW].humidity;
	dataWrite[28] = (int)(device.sensorsBME280[ID_WYWIEW].pressure/10);

	dataWrite[29] = device.fanSpeed;
	dataWrite[30] = (int)(device.fan1revs/100);
	dataWrite[31] = (int)(device.fan2revs/100);

	dataWrite[32] = device.defrost.timeLeft;
	int hPaDiff = (int)(device.defrost.hPaDiff/10);
	dataWrite[33] = (hPaDiff>255? 255 : hPaDiff);

	setUDPdata(0, dataWrite,size);
}

void statusUpdate() {
	String status;
	status = "PARAMETRY PRACY\n";
	status +="Wentylatory: "; status += device.fanSpeed; status +="[%]\tObr1: "; status += device.fan1revs; status +="[min-1]\tObr2: "; status += device.fan2revs; status +="[min-1]\n";
	status +="NormalON: "; status += device.normalON ? "TAK":"NIE"; status +="\tHumidityALERT: "; status += device.humidityAlert ? "TAK":"NIE"; status +="\tbypass otwarty: "; status += device.bypassOpen ? "TAK":"NIE"; status +="\n";
	status +="Odmrazanie: "; status += device.defrost.req ? "TAK":"NIE"; status +="\ttime left: "; status += device.defrost.timeLeft; status +="\t[s] pressure diff: "; status += device.defrost.hPaDiff; status +="[hPa]\n";
	status +="Czerpnia:\t T="; status +=device.sensorsBME280[0].temperature; status +="[stC]\tH="; status +=(int)device.sensorsBME280[0].humidity;
	status +="[%]\tP="; status +=(int)device.sensorsBME280[0].pressure;status +="[hPa] Faulty="; status +=(int)device.sensorsBME280[0].faultyReadings ;status +="\n";
	status +="Wyrzutnia:\t T="; status +=device.sensorsBME280[1].temperature; status +="[stC]\tH="; status +=(int)device.sensorsBME280[1].humidity;
	status +="[%]\tP="; status +=(int)device.sensorsBME280[1].pressure;status +="[hPa] Faulty="; status +=(int)device.sensorsBME280[1].faultyReadings ;status +="\n";
	status +="Nawiew:\t\t T="; status +=device.sensorsBME280[2].temperature; status +="[stC]\tH="; status +=(int)device.sensorsBME280[2].humidity;
	status +="[%]\tP="; status +=(int)device.sensorsBME280[2].pressure;status +="[hPa] Faulty="; status +=(int)device.sensorsBME280[2].faultyReadings ;status +="\n";
	status +="Wywiew:\t\t T="; status +=device.sensorsBME280[3].temperature; status +="[stC]\tH="; status +=(int)device.sensorsBME280[3].humidity;
	status +="[%]\tP="; status +=(int)device.sensorsBME280[3].pressure;status +="[hPa] Faulty="; status +=(int)device.sensorsBME280[3].faultyReadings ;status +="\n";
	for (int i=0; i<7; i++) {
		status +="Zone["; status +=i; status += "]:\t\t T="; status +=zones[i].isTemp; status +="[stC]\treqT="; status+=zones[i].reqTemp; status +="[stC]\tH="; status +=zones[i].humidity; status +="\n";
	}
	setStatus(status);
}

//Help functions
int get10Temp(float temp){
	return (int)temp;
}

int get01Temp(float temp) {
	int temp10 = get10Temp(temp);
	return (int) ((temp*10)-(temp10*10));
}




