#include "Module.h"

Device device;
Zone zones[7];
AirPollution airPollution;
DataRead UDPdata;
Servo servo;

//Functions
void firstScan();
void readSensors();
void readUDPdata();
void getMasterDeviceOrder();
void getComfortParams();
void getAirParams();

void normalMode();
void humidityAlert();
void defrost();
void bypass();
void fan();
void efficency();

void outputs();
void setUDPdata();
void statusUpdate();

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
unsigned long efficencyDelayMillis = 0;

//TMP
int lastDutyCycle = 0;

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

	device.efficency.is=0;

	//Bypass initialization
	ledcSetup(SERVO_CHANNEL, SERVO_FREQUENCY, SERVO_RESOUTION);
	ledcAttachPin(SERVO_PIN, SERVO_CHANNEL);

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
	humidityAlert();
	defrost();
	bypass();
	fan();
	efficency();

	//Output settings
	outputs();
	setUDPdata();
	statusUpdate();
}

void firstScan() {
	// Bytes coding EEPROM //
	// 0 - 11 		: 	device.hour[0] .. device.hour[11]
	// 12			:	device.defrost.trigger
	//Get data from EEprom
	int size = 1024;
	byte EEpromData[size];
	EEpromScan(EEpromData, size);
	Serial.println("\nFirst scan");
	for (int i=0; i<12; i++) {
		byte data = EEpromData[i];
		device.hour[i] = (byte) data;
	}
	device.defrost.trigger = EEpromData[12];

	device.humidityAlert.trigger = EEpromData[13];
}

void readSensors() {
	if (sleep(&readSensorMillis, 5)) return;
	for (int i=0; i<4; i++) {
		device.sensorsBME280[i].temperature = device.sensorsBME280[i].interface.readTemperature();
		device.sensorsBME280[i].pressure = (int)(device.sensorsBME280[i].interface.readPressure()/100);
		device.sensorsBME280[i].humidity = (int)device.sensorsBME280[i].interface.readHumidity();
		if (device.sensorsBME280[i].temperature>70
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
	if ((UDPdata.deviceType == ID_MOD_WEATHER)
			&& (UDPdata.deviceNo == 0)
			&& (UDPdata.frameNo == 0))
		getAirParams();

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
	if (UDPdata.data[0] == 33) {
		device.defrost.trigger = UDPdata.data[1];
		EEpromWrite(12, UDPdata.data[1]);
	}

	//byte 35
	if (UDPdata.data[0] == 35) {
		device.humidityAlert.trigger = UDPdata.data[1];
		EEpromWrite(13, UDPdata.data[1]);
	}	setUDPdata();
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

	//don't turn on in normal mode ventilation if air is dusty
	if ((airPollution.pm10>150)
			|| airPollution.pm25>150)
		normalOn = false;

	device.normalON = normalOn;
}

void humidityAlert() {
	unsigned long currentMillis = millis();
	device.humidityAlert.req = ((zones[ID_ZONE_SALON].humidity>=device.humidityAlert.trigger)
					|| (zones[ID_ZONE_PRALNIA].humidity>=device.humidityAlert.trigger)
					|| (zones[ID_ZONE_LAZDOL].humidity>=device.humidityAlert.trigger)
					|| (zones[ID_ZONE_RODZICE].humidity>=device.humidityAlert.trigger)
					|| (zones[ID_ZONE_NATALIA].humidity>=device.humidityAlert.trigger)
					|| (zones[ID_ZONE_KAROLINA].humidity>=device.humidityAlert.trigger)
					|| (zones[ID_ZONE_LAZGORA].humidity>=device.humidityAlert.trigger));

	int turboTrigger = device.humidityAlert.trigger + 10;
	device.humidityAlert.turbo = ((zones[ID_ZONE_SALON].humidity>=turboTrigger)
					|| (zones[ID_ZONE_PRALNIA].humidity>=turboTrigger)
					|| (zones[ID_ZONE_LAZDOL].humidity>=turboTrigger)
					|| (zones[ID_ZONE_RODZICE].humidity>=turboTrigger)
					|| (zones[ID_ZONE_NATALIA].humidity>=turboTrigger)
					|| (zones[ID_ZONE_KAROLINA].humidity>=turboTrigger)
					|| (zones[ID_ZONE_LAZGORA].humidity>=turboTrigger));

	if (device.humidityAlert.req)
		device.humidityAlert.endMillis = currentMillis+(HUMIDITY_ALERT_PROCESS_TIME*60000);
	long timeLeftMillis = device.humidityAlert.endMillis - currentMillis;
	if (timeLeftMillis<0) timeLeftMillis = 0;
	device.humidityAlert.timeLeft = int((timeLeftMillis)/60000);
	if ((device.humidityAlert.timeLeft<0) || (device.humidityAlert.endMillis==0)) device.humidityAlert.timeLeft = 0;

}

void defrost() {
	unsigned long currentMillis = millis();
	device.defrost.req = ((device.efficency.is>0) && (device.efficency.is <=device.defrost.trigger));
	if (device.defrost.req) {
		String info = "Wymuszenie rozmra¿ania EFF=";
		info +=device.efficency.is;
		info +="%";
		addLog(info);
	}
	if (device.defrost.req)	device.defrost.endMillis = currentMillis + (11*60000);

	long timeLeftMillis = device.defrost.endMillis - currentMillis;
	if (timeLeftMillis<0) timeLeftMillis = 0;

	device.defrost.timeLeft = int((timeLeftMillis)/60000);
	if ((device.defrost.timeLeft<=0) || (device.defrost.endMillis==0)) device.defrost.timeLeft = 0;

	//force defrost off
	if (device.defrost.trigger == 0) {
		device.defrost.timeLeft = 0;
		device.defrost.endMillis = currentMillis;
	}
}

void bypass() {
	device.bypassOpen = device.defrost.timeLeft>0;
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
		device.fanSpeed = 50;
	if (device.humidityAlert.timeLeft>0)
		device.fanSpeed = 75;
	if (device.defrost.timeLeft>0)
		device.fanSpeed = 80;
	if (device.humidityAlert.turbo)
		device.fanSpeed = 100;
}

void efficency() {
	//efficency reset
	if (device.defrost.req == 1) {
		device.efficency.max = 0;
		device.efficency.min = 100;
	}

	if ((device.fanSpeed==0)
			|| millis()<60000
			|| device.bypassOpen) {
		device.efficency.is = 0;
		efficencyDelayMillis = millis();
		return;
	} else
		if (sleep(&efficencyDelayMillis, 60)) return;
	// https://www.engineeringtoolbox.com/heat-recovery-efficiency-d_201.html
	// ut = (t2 - t1) / (t3 - t1)  (nawiew-czerpnia)/(wywiew-czerpnia
	float nawiew = device.sensorsBME280[ID_NAWIEW].temperature;
	float czerpnia = device.sensorsBME280[ID_CZERPNIA].temperature;
	float wywiew = device.sensorsBME280[ID_WYWIEW].temperature;

	float licznik = (nawiew-czerpnia);
	float mianownik = (wywiew-czerpnia);
	if (mianownik==0)
		mianownik = 0.01f;

	device.efficency.is = (int)(licznik*100/mianownik);
	if (device.efficency.is>device.efficency.max)
		device.efficency.max = device.efficency.is;
	if ((device.efficency.is<device.efficency.min)
			|| device.efficency.min ==0)
		device.efficency.min = device.efficency.is;
}

void outputs() {
	//Bypass
	int dutyCycle = 0;
	if (device.bypassOpen) dutyCycle = 10;
	else dutyCycle = 17;
	ledcWrite(SERVO_CHANNEL, dutyCycle);

	//Fans
	// parsing 0-100% into 255-0
	dutyCycle = 255-(int)((device.fanSpeed/100.00)*255);
	if (dutyCycle<0) dutyCycle = 0;
	if (dutyCycle>255) dutyCycle = 255;
	ledcWrite(PWM_CHANNEL, dutyCycle);
}

void setUDPdata() {
	int size = 37;
	byte dataWrite[size];
	// First three bytes are reserved for device recognized purposes.
	dataWrite[0] = (((device.fanSpeed>0)?1:0)<< 7) | (device.normalON << 6) | (device.humidityAlert.req<< 5) | (device.bypassOpen << 4) | (device.defrost.req << 3);
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
	dataWrite[33] = device.defrost.trigger;

	dataWrite[34] = device.humidityAlert.timeLeft;
	dataWrite[35] = device.humidityAlert.trigger;

	dataWrite[36] = device.efficency.is;

	setUDPdata(0, dataWrite,size);
}

void statusUpdate() {
	String status;
	status = "PARAMETRY PRACY\n";
	status +="Wentylatory: "; status += device.fanSpeed; status +="[%]\tObr1: "; status += device.fan1revs; status +="[min-1]\tObr2: "; status += device.fan2revs; status +="[min-1]\n";
	status +="EFF:"; status += device.efficency.is; status +="[%] MIN:"; status += device.efficency.min; status +="[%] MAX:"; status += device.efficency.max; status +="[%]\n:";
	status +="NormalON: "; status += device.normalON ? "TAK":"NIE"; status +="\tbypass otwarty: "; status += device.bypassOpen ? "TAK":"NIE"; status +="\n";
	status +="Odmrazanie: "; status += device.defrost.req ? "TAK":"NIE"; status +="\ttime left: "; status += device.defrost.timeLeft; status +="[min]\ttrigger EFF: "; status += device.defrost.trigger; status +="[%]\n";
	status +="Humidity Alert: "; status += device.humidityAlert.req ? "TAK":"NIE"; status +="\ttime left: "; status += device.humidityAlert.timeLeft; status +="[min]\ttrigger: "; status += device.humidityAlert.trigger; status +="[%]\n";
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
	status +="Air\tPM2.5="; status+=airPollution.pm25; status +="[ug/m3]\t PM10="; status +=airPollution.pm10; status+="[ug/m3]\n";
	setStatus(status);
}




