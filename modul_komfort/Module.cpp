#include "Module.h"

Device device;
DataRead UDPdata;

OneWire oneWire(PIN_DS18B20);
DallasTemperature sensors(&oneWire);					// initialized 1-WIRE for buffers

DHT dhtSalon(PIN_DHT_SALON, DHTTYPE);
DHT dhtPralnia(PIN_DHT_PRALNIA, DHTTYPE);
DHT dhtLazDol(PIN_DHT_LAZ_DOL, DHTTYPE);
DHT dhtRodzice(PIN_DHT_RODZICE, DHTTYPE);
DHT dhtNatalia(PIN_DHT_NATALIA, DHTTYPE);
DHT dhtKarloina(PIN_DHT_KAROLINA, DHTTYPE);
DHT dhtLazGora(PIN_DHT_LAZ_GORA, DHTTYPE);

//Functions
void firstScan();
void readSensors();
void DALLAS18b20Read ();
void DHT22Read ();
void readUDPdata();
void getMasterDeviceOrder();

void setUDPdata();
void statusUpdate();

//DIAGNOSTIC HELP FUNCTIONS
void diagDALLAS18b20ReadDeviceAdresses();
void TMPWritteValuesToEEprom();

//Delays
unsigned long dallasSensorReadMillis = 0;

//TMP
DeviceAddress tmpAddresses[8];
int count = 0;

void module_init() {
	//EEprom Scan
	firstScan();

	//Set sensors Dallas DS18B20
	sensors.begin();
	sensors.setResolution(10);
	sensors.setWaitForConversion(false);

	//Set sensors DHT11
	device.dhtSensor[ID_SALON] = dhtSalon;
	device.dhtSensor[ID_PRALNIA] = dhtPralnia;
	device.dhtSensor[ID_LAZ_DOL] = dhtLazDol;
	device.dhtSensor[ID_RODZICE] = dhtRodzice;
	device.dhtSensor[ID_NATALIA] = dhtNatalia;
	device.dhtSensor[ID_KAROLINA] = dhtKarloina;
	device.dhtSensor[ID_LAZ_GORA] = dhtLazGora;

	device.dhtSensor[ID_SALON].begin();
	device.dhtSensor[ID_PRALNIA].begin();
	device.dhtSensor[ID_LAZ_DOL].begin();
	device.dhtSensor[ID_RODZICE].begin();
	device.dhtSensor[ID_NATALIA].begin();
	device.dhtSensor[ID_KAROLINA].begin();
	device.dhtSensor[ID_LAZ_GORA].begin();

}

void module() {
	readSensors();
	readUDPdata();
	setUDPdata();
	statusUpdate();
}

void firstScan() {
	// Bajty EEPROM //
	// 0 - 6 		: 	strefa[..] wymagana temperatura
	// 50 - 57		:   zone0 18b20 device address
	// 58 - 55		:   zone1 18b20 device address
	// 66 - 73		:   zone2 18b20 device address
	// 74 - 81		:   zone3 18b20 device address
	// 82 - 89		:   zone4 18b20 device address
	// 90 - 97		:   zone5 18b20 device address
	// 98 - 105		:   zone6 18b20 device address
	int size = 1024;
	byte EEpromData[size];
	EEpromScan(EEpromData, size);
	Serial.println("\nFirst scan");

	// Get required temperatures
	for (int i=0; i<ZONE_QUANTITY; i++)
		device.zone[i].reqTemp = (float)EEpromData[i]/2;

	// Get 18b20 device addresses
	int offset = 50;						// first byte of DS18b20 device addresses
	for (int i=0; i<ZONE_QUANTITY; i++) {
		Serial.printf("\nSensor[%d] ",i);
		for (int j=0; j<8; j++) {
			device.zone[i].deviceAddress[j] = EEpromData[offset+8*i+j];
			Serial.printf("[%d]",device.zone[i].deviceAddress[j]);
		}
	}

	//TEMP
	//TMPWritteValuesToEEprom();
}

void readSensors() {
	if ((sleep(&dallasSensorReadMillis, DELAY_SENSORS_READ)) && (millis()>5000)) return;

	DALLAS18b20Read();
	DHT22Read();
}

void DALLAS18b20Read () {
	for (int zone=0; zone<ZONE_QUANTITY; zone++) {
		DeviceAddress deviceAddress;
		for (int i=0; i<8; i++)
			deviceAddress[i] = device.zone[zone].deviceAddress[i];
		float tempC = 0;
		float a=0;
		float b=0;
		if (zone== 0) {a=0.99;		b=(6.2);}			// Zone 0 - Salon, kuchnia, klatka, przedpokój
		if (zone== 1) {a=0.995;		b=(0+2.2);}			// Zone 1 - Pralnia, warsztat
		if (zone== 2) {a=1.07;		b=(-4.5+2.7);}		// Zone 2 - £azienka dó³,gabinet
		if (zone== 3) {a=0.98;		b=(2.1+4.4);}		// Zone 3 - rodzice
		if (zone== 4) {a=0.985; 	b=(0+3.8);}			// Zone 4 - Natalia
		if (zone== 5) {a=0.975; 	b=(1+0.8);}			// Zone 5 - Karolina
		if (zone== 6) {a=0.995; 	b=(-1.2+4.7);}		// Zone 6 - Laz Gora
		tempC = sensors.getTempC(deviceAddress);
		if ((tempC<5) || (tempC>100)) {
			device.zone[zone].dallasErrorCount++;
			//zapisz liczbe maksymalna zlych odczytow z rzedu
			if (device.zone[zone].dallasMaxErrorCount < device.zone[zone].dallasErrorCount) device.zone[zone].dallasMaxErrorCount = device.zone[zone].dallasErrorCount;
			//po x probach nie udanych wysli fa³szyw¹ temperaturê
			if (device.zone[zone].dallasErrorCount > 50) device.zone[zone].isTemp = 10.00;
		}
		else {
			//przelicz temperature wedlug krzywej
			tempC = a * tempC + b;						// Temperature compensation
			//zerowanie liczby bledow
			device.zone[zone].dallasErrorCount = 0;
			device.zone[zone].isTemp = tempC;
		}
	}
	sensors.requestTemperatures();						// Request temperature
}

void getHumidity(int zone) {
	int humidity = (int)device.dhtSensor[zone].readHumidity();
	if (isnan(humidity)
			|| (humidity<15)
			|| (humidity>100))
		device.zone[zone].dhtErrorCount++;
	else {
		if ((humidity>=80)
				&& (device.zone[zone].humidity<80)) {
			String log = "Strefa[";
			log+=zone;
			log +="] alert[";
			log +=humidity;
			log +="] poprzedni odczyt=";
			log +=device.zone[zone].humidity;
			addLog(log);
		}
		device.zone[zone].dhtErrorCount = 0;
		device.zone[zone].humidity = humidity;
	}
	if (device.zone[zone].dhtMaxErrorCount<device.zone[zone].dhtErrorCount)
		device.zone[zone].dhtMaxErrorCount=device.zone[zone].dhtErrorCount;

	//second temperature to compare
	device.zone[zone].dhtTemp = device.dhtSensor[zone].readTemperature();
}

void DHT22Read() {
	getHumidity(ID_SALON);
	getHumidity(ID_LAZ_DOL);
	getHumidity(ID_PRALNIA);
	getHumidity(ID_LAZ_GORA);
	getHumidity(ID_KAROLINA);
	getHumidity(ID_NATALIA);
	getHumidity(ID_RODZICE);
	//TODO list to extend
}

void readUDPdata() {
	UDPdata = getDataRead();
	if (!UDPdata.newData) return;
	if ((UDPdata.deviceType == ID_MOD_MAIN)
			&& (UDPdata.deviceNo == getModuleType())
			&& (UDPdata.frameNo == getModuleNo()))
		getMasterDeviceOrder();
	resetNewData();
}

void getMasterDeviceOrder() {
	if (UDPdata.data[0] == 2) {
		float reqTemp = (float)UDPdata.data[1]/2.0;
		if (device.zone[0].reqTemp == reqTemp)
			return;
		else device.zone[0].reqTemp = reqTemp;
		EEpromWrite(0, UDPdata.data[1]);
		String log = "Zmiana temperatury - Salon=";
		log +=device.zone[0].reqTemp;
		addLog(log);
	}
	if (UDPdata.data[0] == 6) {
		float reqTemp = (float)UDPdata.data[1]/2.0;
		if (device.zone[1].reqTemp == reqTemp)
			return;
		else device.zone[1].reqTemp = reqTemp;
		EEpromWrite(1, UDPdata.data[1]);
		String log = "Zmiana temperatury - Pralnia=";
		log +=device.zone[1].reqTemp;
		addLog(log);
	}
	if (UDPdata.data[0] == 10) {
		float reqTemp = (float)UDPdata.data[1]/2.0;
		if (device.zone[2].reqTemp == reqTemp)
			return;
		else device.zone[2].reqTemp = reqTemp;
		EEpromWrite(2, UDPdata.data[1]);
		String log = "Zmiana temperatury - £aŸ.Dol=";
		log +=device.zone[2].reqTemp;
		addLog(log);
	}
	if (UDPdata.data[0] == 14) {
		float reqTemp = (float)UDPdata.data[1]/2.0;
		if (device.zone[3].reqTemp == reqTemp)
			return;
		else device.zone[3].reqTemp = reqTemp;
		EEpromWrite(3, UDPdata.data[1]);
		String log = "Zmiana temperatury - rodzice=";
		log +=device.zone[3].reqTemp;
		addLog(log);
	}
	if (UDPdata.data[0] == 18) {
		float reqTemp = (float)UDPdata.data[1]/2.0;
		if (device.zone[4].reqTemp == reqTemp)
			return;
		else device.zone[4].reqTemp = reqTemp;
		EEpromWrite(4, UDPdata.data[1]);
		String log = "Zmiana temperatury - Natalia=";
		log +=device.zone[4].reqTemp;
		addLog(log);
	}
	if (UDPdata.data[0] == 22) {
		float reqTemp = (float)UDPdata.data[1]/2.0;
		if (device.zone[5].reqTemp == reqTemp)
			return;
		else device.zone[5].reqTemp = reqTemp;
		EEpromWrite(5, UDPdata.data[1]);
		String log = "Zmiana temperatury - Karolina=";
		log +=device.zone[5].reqTemp;
		addLog(log);
	}
	if (UDPdata.data[0] == 26) {
		float reqTemp = (float)UDPdata.data[1]/2.0;
		if (device.zone[6].reqTemp == reqTemp)
			return;
		else device.zone[6].reqTemp = reqTemp;
		EEpromWrite(6, UDPdata.data[1]);
		String log = "Zmiana temperatury - £aŸ. Góra=";
		log +=device.zone[6].reqTemp;
		addLog(log);
	}
}

void setUDPdata() {
	int size = 28;
	byte dataWrite[size];

	byte temperature1_0 = 0;
	byte temperature0_1 = 0;
	for (int i=0; i<7; i++) {
		float temperature = device.zone[i].isTemp;

		temperature1_0 = (int)temperature;
		int temp=int(temperature*10);
		int temp1 = temperature1_0*10;
		int temp2 = temp-temp1;
		temperature0_1 = temp2;

		dataWrite[(i*4)] = temperature1_0;
		dataWrite[(i*4)+1] = temperature0_1;
		dataWrite[(i*4)+2] = device.zone[i].reqTemp*2;
		dataWrite[(i*4)+3] = device.zone[i].humidity;
	}
	setUDPdata(0, dataWrite,size);
}

void statusUpdate() {
	String status;
	status = "MAIN\n";
	for (int i=0; i<ZONE_QUANTITY; i++) {
		status +="\nZone[";
		status +=i;
		status +="]\tH=";
		status +=(int)device.zone[i].humidity;
		status +="\tT=";
		status +=(double)device.zone[i].isTemp;
		status +="\treqT=";
		status +=(double)device.zone[i].reqTemp;
		status +="\tdhtT=";
		status +=(double)device.zone[i].dhtTemp;
		status +="\tdallasErrMAX[";
		status +=device.zone[i].dallasMaxErrorCount;
		status +="]";
		status +="\tdhtErrMAX[";
		status +=device.zone[i].dhtMaxErrorCount;
		status +="]\t";
		status +="Addr";
		for (int j=0; j<8; j++) {
			status +="[";
			status +=device.zone[i].deviceAddress[j];
			status +="]";
		}
		status +="\n";
	}
	setStatus(status);
}

//DIAGNOSTIC HELP FUNCTIONS
void diagDALLAS18b20ReadDeviceAdresses() {
	for (int i=0; i<ZONE_QUANTITY; i++)
		sensors.getAddress(tmpAddresses[i],i);
	count = 0;
}


void TMPWritteValuesToEEprom() {
	DeviceAddress tempAddresses[7] = {
			{40,7,0,7,151,237,1,237},		// SALON
			{40,2,0,7,104,63,1,29},			// PRALNIA
			{40,255,2,62,96,23,5,142},		// LAZIENKA DOL
			{40,2,0,7,201,119,1,173},		// RODZICE
			{40,2,0,7,38,42,1,203},			// NATALIA
			{40,2,0,7,192,72,1,22},			// KATOLINA
			{40,7,0,7,141,124,1,202}		// LAZIENKA GORA
	};

	int offset = 50;
	//ADDRESS IS SAVING
	for (int zoneNo=0; zoneNo<ZONE_QUANTITY; zoneNo++) {
		Serial.printf("\nZone[%d]\t", zoneNo);
		for (int i=0; i<8; i++) {
			EEpromWrite((offset+(8*zoneNo+i)),tempAddresses[zoneNo][i]);
			Serial.printf("[%d][%d]\t",offset+(8*zoneNo+i),tempAddresses[zoneNo][i]);
			delay(100);
		}
	delay(100);
	}
}
