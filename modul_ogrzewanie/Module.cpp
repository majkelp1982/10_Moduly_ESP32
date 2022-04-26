#include "Module.h"

// Variables
Device device;
DataRead UDPdata;
VentData ventData;

//Buffers
bool reqCOload = false;
bool reqCOloadToCold = false;
bool reqCOloadByNight = false;

bool reqCWUload = false;
bool reqCWUloadByNight = false;

//Heat pump
bool cheapTariffHoursActive = false;
bool heatPumpIsHeating = false;
bool heatPumpDelayActiv = false;
bool heatPumpOverheat = false;

int lastCircuitOnAmount = 0;
bool valve_bypass_CloseDelayActive = false;

OneWire oneWireBuffers(ONE_WIRE_BUFFERS);
OneWire oneWireDevices(ONE_WIRE_DEVICES);
DallasTemperature sensorsBuffers(&oneWireBuffers);					// initialized 1-WIRE for buffers
DallasTemperature sensorsDevices(&oneWireDevices);					// initialized 1-WIRE for devices


//Functions
void firstScan();
void pinSetup();
void ValvesInit();
void readSensors();
void dallas18b20Read (Thermometer *thermometer);
void readUDPdata();
void getMasterDeviceOrder();
void getComfortParams();
void getVentParams();

void valves();
void buffers();
void heatingAndPumps();

void outputs();
void setUDPdata();
void statusUpdate();

//millis
unsigned long dallasReadMillis = 0;
unsigned long currentMillis;
unsigned long valve_bypass_CloseDelayMillis = 0;
unsigned long heatPumpTempCorrectionMillis = 0;
unsigned long lastCorrectReadTempZrodla = 0;
unsigned long heatPumpDelay = 0;
unsigned long valve_3way_activatedMillis = 0;
unsigned long valve_bypass_inMove = 0;
unsigned long heatPumpOverheatDelay = 0;

void module_init() {
	//EEprom Scan
	firstScan();

	//Pin setup
	pinSetup();

	//Valves
	ValvesInit();

	//1-wire sensors
	sensorsBuffers.begin();
	sensorsDevices.begin();
	sensorsBuffers.setResolution(10);
	sensorsDevices.setResolution(10);
	sensorsBuffers.setWaitForConversion(false);
	sensorsDevices.setWaitForConversion(false);
}

void pinSetup() {
	//CIRCUT RELAYS
	pinMode (ZONE0,OUTPUT);					digitalWrite(ZONE0, HIGH);
	pinMode (ZONE1,OUTPUT);					digitalWrite(ZONE1, HIGH);
	pinMode (ZONE2,OUTPUT);					digitalWrite(ZONE2, HIGH);
	pinMode (ZONE3,OUTPUT);					digitalWrite(ZONE3, HIGH);
	pinMode (ZONE4,OUTPUT);					digitalWrite(ZONE4, HIGH);
	pinMode (ZONE5,OUTPUT);					digitalWrite(ZONE5, HIGH);
	pinMode (ZONE6,OUTPUT);					digitalWrite(ZONE6, HIGH);

	//CIRCULATION PUMP
	pinMode (PUMP_IN_HOUSE,OUTPUT); 		digitalWrite(PUMP_IN_HOUSE, HIGH);
	pinMode (PUMP_UNDER_HOUSE,OUTPUT); 		digitalWrite(PUMP_UNDER_HOUSE, HIGH);

	//VALVES
	pinMode (VALVE_3WAY_CO,OUTPUT); 		digitalWrite(VALVE_3WAY_CO, HIGH);
	pinMode (VALVE_3WAY_CWU,OUTPUT); 		digitalWrite(VALVE_3WAY_CWU, HIGH);
	pinMode (VALVE_BYPASS_OPEN,OUTPUT); 	digitalWrite(VALVE_BYPASS_OPEN, HIGH);
	pinMode (VALVE_BYPASS_CLOSE,OUTPUT); 	digitalWrite(VALVE_BYPASS_CLOSE, HIGH);

	//HEATING PUMP
	pinMode (RELAY_HEAT_PUMP_ON,OUTPUT); digitalWrite(RELAY_HEAT_PUMP_ON, HIGH);

	//ANTYLEGIONELLIA
	pinMode (RELAY_ANTILEGIONELLIA,OUTPUT); digitalWrite(RELAY_ANTILEGIONELLIA, HIGH);
}

void ValvesInit() {
	//Drive valve to position full open and set real position on 20
	addLog("Ustawienie pozycji pocz¹tkowej Bypass=100%, 3-way=CWU");
	digitalWrite(VALVE_BYPASS_CLOSE,1);
	digitalWrite(VALVE_3WAY_CO,1);
	while (!digitalRead(VALVE_BYPASS_CLOSE));
	while (!digitalRead(VALVE_3WAY_CO));
	delay(1000);
	digitalWrite(VALVE_3WAY_CWU,0);
	digitalWrite(VALVE_BYPASS_OPEN,0);
	//40 second needed to full open from close position
	delay(40000);

	digitalWrite(VALVE_BYPASS_OPEN,1);
	digitalWrite(VALVE_3WAY_CWU,1);
	device.valve_3wayLastState = CWU;
	device.valve_bypass_realPos = 40;
	device.valve_bypass = 40;
	addLog("Valve Bypass OK");
}

void module() {
	//Input data
	readSensors();
	readUDPdata();

	//Module events
	valves();
	buffers();
	heatingAndPumps();

	//Output settings
	outputs();
	setUDPdata();
	statusUpdate();
}

void firstScan() {
	// Bajty EEPROM //
	// 1 			: 	tylko taryfa II
	// 2			:   ogrzewanie
	// 3			:   nastawiona temperatura CO
	// 4			:   nastawiona temperatura CWU
	// 5			:	Nastawa temperatury alarmowej
	// 10 - 17		:   Bufor CO dol 18b20 device address
	// 18 - 25		:   Bufor CO srodek 18b20 device address
	// 26 - 33		:   Bufor CO gora 18b20 device address
	// 34 - 41		:   Bufor CWU dol 18b20 device address
	// 42 - 49		:   Bufor CWU srodek 18b20 device address
	// 50 - 57		:   Bufor CWU gora 18b20 device address
	// 58 - 65		:   zasilanie 18b20 device address
	// 66 - 73		:   powrot 18b20 device address
	// 74 - 81		:   dolneZrodlo 18b20 device address
	// 82 - 89		:   Kominek 18b20 device address
	// 90 - 97		:   Rozdzielacze 18b20 device address
	// 98 - 105		:   PowrotParter 18b20 device address
	// 106 - 113	:   PowrotPietro 18b20 device address
	int size = 1024;
	byte EEpromData[size];
	EEpromScan(EEpromData, size);
	Serial.println("\nFirst scan");

	device.cheapTariffOnly = (EEpromData[1] != 0);
	device.heatingActivated  = (EEpromData[2] != 0);
	device.reqTempBuforCO = EEpromData[3]/2.0;
	device.reqTempBuforCWU = EEpromData[4]/2.0;
	device.heatPumpAlarmTemperature = EEpromData[5];

	// Get 18b20 device addresses
	int offset = 10;
	for (int i=0; i<8; i++) {
		device.tBuffCOdol.deviceAddress[i] = EEpromData[offset+i];
		Serial.printf("[%d]",device.tBuffCOdol.deviceAddress[i]);
	}

	Serial.println();
	offset += 8;
	for (int i=0; i<8; i++) {
		device.tBuffCOsrodek.deviceAddress[i] = EEpromData[offset+i];
		Serial.printf("[%d]",device.tBuffCOsrodek.deviceAddress[i]);
	}
	Serial.println();
	offset += 8;
	for (int i=0; i<8; i++) {
		device.tBuffCOgora.deviceAddress[i] = EEpromData[offset+i];
		Serial.printf("[%d]",device.tBuffCOgora.deviceAddress[i]);
	}
	Serial.println();
	offset += 8;
	for (int i=0; i<8; i++) {
		device.tBuffCWUdol.deviceAddress[i] = EEpromData[offset+i];
		Serial.printf("[%d]",device.tBuffCWUdol.deviceAddress[i]);
	}
	Serial.println();
	offset += 8;
	for (int i=0; i<8; i++) {
		device.tBuffCWUsrodek.deviceAddress[i] = EEpromData[offset+i];
		Serial.printf("[%d]",device.tBuffCWUsrodek.deviceAddress[i]);
	}
	Serial.println();
	offset += 8;
	for (int i=0; i<8; i++) {
		device.tBuffCWUgora.deviceAddress[i] = EEpromData[offset+i];
		Serial.printf("[%d]",device.tBuffCWUgora.deviceAddress[i]);
	}
	Serial.println();
	offset += 8;
	for (int i=0; i<8; i++) {
		device.tZasilanie.deviceAddress[i] = EEpromData[offset+i];
		Serial.printf("[%d]",device.tZasilanie.deviceAddress[i]);
	}
	Serial.println();
	offset += 8;
	for (int i=0; i<8; i++) {
		device.tPowrot.deviceAddress[i] = EEpromData[offset+i];
		Serial.printf("[%d]",device.tPowrot.deviceAddress[i]);
	}
	Serial.println();
	offset += 8;
	for (int i=0; i<8; i++) {
		device.tDolneZrodlo.deviceAddress[i] = EEpromData[offset+i];
		Serial.printf("[%d]",device.tDolneZrodlo.deviceAddress[i]);
	}
	Serial.println();
	offset += 8;
	for (int i=0; i<8; i++) {
		device.tKominek.deviceAddress[i] = EEpromData[offset+i];
		Serial.printf("[%d]",device.tKominek.deviceAddress[i]);
	}
	Serial.println();
	offset += 8;
	for (int i=0; i<8; i++) {
		device.tRozdzielacze.deviceAddress[i] = EEpromData[offset+i];
		Serial.printf("[%d]",device.tRozdzielacze.deviceAddress[i]);
	}
	Serial.println();
	offset += 8;
	for (int i=0; i<8; i++) {
		device.tPowrotParter.deviceAddress[i] = EEpromData[offset+i];
		Serial.printf("[%d]",device.tPowrotParter.deviceAddress[i]);
	}
	Serial.println();
	offset += 8;
	for (int i=0; i<8; i++) {
		device.tPowrotPietro.deviceAddress[i] = EEpromData[offset+i];
		Serial.printf("[%d]",device.tPowrotPietro.deviceAddress[i]);
	}
	Serial.println();
	offset += 8;


}

void readSensors() {
	if (sleep(&dallasReadMillis, DELAY_DS18B20_READ)) return;

	// Get actual temperatures and compensate
	dallas18b20Read(&device.tBuffCOdol);
	dallas18b20Read(&device.tBuffCOsrodek);
	dallas18b20Read(&device.tBuffCOgora);

	dallas18b20Read(&device.tBuffCWUdol);
	dallas18b20Read(&device.tBuffCWUsrodek);
	dallas18b20Read(&device.tBuffCWUgora);

	dallas18b20Read(&device.tZasilanie);
	dallas18b20Read(&device.tPowrot);
	dallas18b20Read(&device.tDolneZrodlo);

	dallas18b20Read(&device.tKominek);
	dallas18b20Read(&device.tRozdzielacze);
	dallas18b20Read(&device.tPowrotParter);
	dallas18b20Read(&device.tPowrotPietro);

	sensorsBuffers.requestTemperatures();				// Request temperature
	sensorsDevices.requestTemperatures();				// Request temperature

}

void dallas18b20Read (Thermometer *thermometer) {
	DeviceAddress deviceAddress;
	for (int i=0; i<8; i++) deviceAddress[i] = thermometer->deviceAddress[i];
	float tempC = 0;
	float a=0;
	float b=0;
	bool buffers = false;
	bool devices = false;

	if (memcmp(device.tBuffCOdol.deviceAddress,deviceAddress,8) == 0) 		{a=0.9; 	b=7.7;	buffers = true;}
	if (memcmp(device.tBuffCOsrodek.deviceAddress,deviceAddress,8) == 0) 	{a=0.935; 	b=4.7;	buffers = true;}
	if (memcmp(device.tBuffCOgora.deviceAddress,deviceAddress,8) == 0) 		{a=0.9; 	b=6.2;	buffers = true;}
	if (memcmp(device.tBuffCWUdol.deviceAddress,deviceAddress,8) == 0) 		{a=1.005;	b=3.5;	buffers = true;}
	if (memcmp(device.tBuffCWUsrodek.deviceAddress,deviceAddress,8) == 0) 	{a=0.97; 	b=3.5;	buffers = true;}
	if (memcmp(device.tBuffCWUgora.deviceAddress,deviceAddress,8) == 0) 	{a=0.99; 	b=3.5;	buffers = true;}

	if (memcmp(device.tZasilanie.deviceAddress,deviceAddress,8) == 0) 		{a=0.92; 	b=5;	devices = true;}
	if (memcmp(device.tPowrot.deviceAddress,deviceAddress,8) == 0) 			{a=0.955; 	b=2.7;	devices = true;}
	if (memcmp(device.tDolneZrodlo.deviceAddress,deviceAddress,8) == 0) 	{a=0.1;		b=0;	devices = true;}
	if (memcmp(device.tKominek.deviceAddress,deviceAddress,8) == 0) 		{a=0.97;	b=1.65;	devices = true;}

	if (memcmp(device.tRozdzielacze.deviceAddress,deviceAddress,8) == 0) 	{a=0.96; 	b=2.2;	devices = true;}
	if (memcmp(device.tPowrotParter.deviceAddress,deviceAddress,8) == 0) 	{a=0.1;		b=0;	devices = true;}
	if (memcmp(device.tPowrotPietro.deviceAddress,deviceAddress,8) == 0) 	{a=0.1;		b=0;	devices = true;}

	if (buffers) {
		tempC = sensorsBuffers.getTempC(deviceAddress);
	}
	if (devices) {
		tempC = sensorsDevices.getTempC(deviceAddress);	}

	if (tempC<5) {
		thermometer->errorCount++;
		//zapisz liczbe maksymalna zlych odczytow z rzedu
		if (thermometer->maxErrorCount < thermometer->errorCount) thermometer->maxErrorCount = thermometer->errorCount;
		//Jeœli termometr ZASILANIE po trzech probach nie udanych wysli 100.00 stC
		if ((memcmp(device.tZasilanie.deviceAddress,deviceAddress,8) == 0)) {
			if (thermometer->errorCount > 3) thermometer->isTemp = 100.00;
		}
		else
			if (thermometer->errorCount > 20) thermometer->isTemp = 100.00;
	}
	else {
		//zerowanie liczby bledow
		thermometer->errorCount = 0;
		thermometer->isTemp = a * tempC + b;;
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
	if ((UDPdata.deviceType == ID_MOD_VENT)
			&& (UDPdata.deviceNo == 0)
			&& (UDPdata.frameNo == 0))
		getVentParams();

	resetNewData();
}

void getMasterDeviceOrder() {
	if (UDPdata.data[0] == 3) {
		device.cheapTariffOnly = UDPbitStatus(UDPdata.data[1],2);
		device.heatingActivated = UDPbitStatus(UDPdata.data[1],1);
		device.antyLegionellia = UDPbitStatus(UDPdata.data[1],0);
		//save value to eeprom
		EEpromWrite(1, device.cheapTariffOnly);
		EEpromWrite(2, device.heatingActivated);
	}
	if (UDPdata.data[0] == 6) {
		device.reqTempBuforCO = (float)(UDPdata.data[1])/2;
		//save value to eeprom
		EEpromWrite(3, UDPdata.data[1]);

	}
	if (UDPdata.data[0] == 7) {
		device.reqTempBuforCWU = (float)(UDPdata.data[1])/2;
		//save value to eeprom
		EEpromWrite(4, UDPdata.data[1]);

	}
	if (UDPdata.data[0] == 21) {
		device.heatPumpAlarmTemperature = UDPdata.data[1];
		//save value to eeprom
		EEpromWrite(5, UDPdata.data[1]);

	}
	setUDPdata();
	forceStandardUDP();
}

void getComfortParams(){
	device.zone[ID_SALON].isTemp = UDPdata.data[0]+(UDPdata.data[1]/10.0);
	device.zone[ID_SALON].reqTemp = UDPdata.data[2]/2;

	device.zone[ID_PRALNIA].isTemp = UDPdata.data[4]+(UDPdata.data[5]/10.0);
	device.zone[ID_PRALNIA].reqTemp = UDPdata.data[6]/2;

	device.zone[ID_LAZ_DOL].isTemp = UDPdata.data[8]+(UDPdata.data[9]/10.0);
	device.zone[ID_LAZ_DOL].reqTemp = UDPdata.data[10]/2;

	device.zone[ID_RODZICE].isTemp = UDPdata.data[12]+(UDPdata.data[13]/10.0);
	device.zone[ID_RODZICE].reqTemp = UDPdata.data[14]/2;

	device.zone[ID_NATALIA].isTemp = UDPdata.data[16]+(UDPdata.data[17]/10.0);
	device.zone[ID_NATALIA].reqTemp = UDPdata.data[18]/2;

	device.zone[ID_KAROLINA].isTemp = UDPdata.data[20]+(UDPdata.data[21]/10.0);
	device.zone[ID_KAROLINA].reqTemp = UDPdata.data[22]/2;

	device.zone[ID_LAZ_GORA].isTemp = UDPdata.data[24]+(UDPdata.data[25]/10.0);
	device.zone[ID_LAZ_GORA].reqTemp = UDPdata.data[26]/2;

	if ((getDateTime().hour >= 22 ) || (getDateTime().hour < 6) ||
		((getDateTime().hour >=13) && (getDateTime().hour <15)) ||
		(getDateTime().weekDay==6) || (getDateTime().weekDay==0))		// cheap tariff at the weekend as well 6-Saturday 0-Sunday
	{
		device.zone[0].reqTemp += 0.5;
		device.zone[1].reqTemp += 0.5;
		device.zone[2].reqTemp += 0.5;
		device.zone[3].reqTemp += 0.5;
		device.zone[4].reqTemp += 0.5;
		device.zone[5].reqTemp += 0.5;
		device.zone[6].reqTemp += 0.5;
	}
}

void getVentParams() {
	ventData.reqColdWater = UDPbitStatus(UDPdata.data[0],4);
	ventData.reqHotWater = UDPbitStatus(UDPdata.data[0],3);
}

void valves() {
	// set Valve from distributor if temperature to low including hysteresis. Else, when temperature achieve setTemp turn off circuit valve

	//Zone circuits
	for (int i=0; i<ZONE_QUANTITY; i++) {
		if ((float)device.zone[i].isTemp < (device.zone[i].reqTemp-(float)TEMP_HYSTERESIS))
			device.zone[i].circuit = true;
		if (device.zone[i].isTemp >= device.zone[i].reqTemp)
			device.zone[i].circuit = false;
	}

	//If system is OFF (according to device mode) set all circuit OFF!
	if ((!device.heatingActivated) || (device.valve_3way == CWU))
		for (int i=0; i<ZONE_QUANTITY; i++)
			device.zone[i].circuit = false;

	//count number of circuit on
	// get last amount of circuit open
	lastCircuitOnAmount = device.circuitOnAmount;

	device.circuitOnAmount = 0;
	for (int i=0; i<ZONE_QUANTITY; i++) {
		if (device.zone[i].circuit) device.circuitOnAmount++;
	}

	//Other valves
	// if all of distribution valves are closed, bypass MUST BE OPEN to avoid water flow blocking
	currentMillis = millis();

	// recalculate how many circuit open
	if (device.circuitOnAmount == 0) {
		device.valve_bypass = 40;
		valve_bypass_CloseDelayActive = false;
	}
	if (device.circuitOnAmount>0) {

		// Delay to avoid bypass close when thermoheads are cold and not open. Delay should be not less then 180s.
		// After this time bypass valve can be fully open
		if ((!valve_bypass_CloseDelayActive) && (lastCircuitOnAmount == 0)) {
			valve_bypass_CloseDelayActive = true;
			valve_bypass_CloseDelayMillis = currentMillis;
		}
		if ((valve_bypass_CloseDelayActive) && ((currentMillis-valve_bypass_CloseDelayMillis) < DELAY_THERMO_HEADS)) {
			device.valve_bypass = 40;
			return;
		}

		// When delay is run out already, set start pos to bypass depending on actual heat source
		if (valve_bypass_CloseDelayActive) {
			if (device.heatSourceActive == HEAT_PUMP) device.valve_bypass = HEAT_PUMP_BYP_START_POS;
			else device.valve_bypass = 0;
			valve_bypass_CloseDelayActive = false;
		}

		currentMillis = millis();
		if (device.tZasilanie.isTemp != 0) {
			if (device.tZasilanie.isTemp > HEAT_PUMP_WARNING_TEMPERATURE+HEAT_PUMP_HISTERESIS_HI) {
				if ((currentMillis-heatPumpTempCorrectionMillis) > DELAY_HEAT_PUMP_OUT_TEMP_TO_HIGH) {
					device.valve_bypass += HEAT_PUMP_BYP_UPSTEP_WHEN_HI;
					heatPumpTempCorrectionMillis = currentMillis;
				}
			}
			if (device.tZasilanie.isTemp < HEAT_PUMP_WARNING_TEMPERATURE-HEAT_PUMP_HISTERESIS_LOW) {
				if ((currentMillis-heatPumpTempCorrectionMillis) > DELAY_HEAT_PUMP_OUT_TEMP_TO_LOW) {
					device.valve_bypass-=HEAT_PUMP_BYP_DOWNSTEP_WHEN_LOW;
					heatPumpTempCorrectionMillis = currentMillis;
				}
			}
		}

		//min-max values of bypass 0-40
		if (device.valve_bypass>40) device.valve_bypass=40;
		if (device.valve_bypass<0) device.valve_bypass=0;

		// kiedy uruchomiona pompa obiegowa a pompa zalacza sie ze zwloka, przejedz bypass do pozycji start z ktorej bedzie regulowany
		if ((device.pump_UnderGround) && (!device.reqHeatPumpOn)) {
			device.valve_bypass = HEAT_PUMP_BYP_START_POS;
		}
		//Zamknij bypass jak pompa ciepla nie dziala
		if (!device.pump_UnderGround) device.valve_bypass = 0;
		//Nie koryguj bypass jak pompa ciepla nie laduje ciepla. (moze miec przestuj. Po wznowie za szybko nabije cieplo)
		if ((device.pump_UnderGround) && (device.reqHeatPumpOn) && (!heatPumpIsHeating)) {
			device.valve_bypass = HEAT_PUMP_BYP_START_POS;
			if (!device.valve_bypass_reachPos) Serial.println("Pompa nie grzeje. [HEAT_PUMP_BYP_START_POS]");
		}
		//Kiedy temperatura alarm-2 temp zmniejsz szybko bypass o 10
		if (device.tZasilanie.isTemp == device.heatPumpAlarmTemperature-2) device.valve_bypass -=10;

		//TMP!!! When thermometer fault
		if ((device.reqHeatPumpOn) && (device.tZasilanie.errorCount >= 3)
				&& (device.valve_bypass_realPos != HEAT_PUMP_BYP_START_POS)) {
			if (device.valve_bypass != HEAT_PUMP_BYP_START_POS) addLog("Blad odczytu temperatury po zrodlach. [HEAT_PUMP_BYP_START_POS]");
			device.valve_bypass = HEAT_PUMP_BYP_START_POS;
		}
	}

	// Valve 3_way is set, when hot water is to cold (check on top of buffer) 3-way valves priority set to heat CWU (hot water to use)
	if ((reqCWUload) && (device.heatSourceActive != BUFFER_CO) && (!reqCOloadToCold)) {
		if ((device.valve_3way != CWU) && (device.heatingActivated)) addLog("device.valve_3way = CWU");
		device.valve_3way = CWU;
	}

	// When water temperature measured bottom of buffer CWU achieve set temperature, 3-way run to position HOUSE HEATING.
	if (((!reqCWUload) || (reqCOloadToCold)) && (device.heatingActivated))  {
		if ((device.valve_3way != CO) && (device.heatingActivated)) addLog("device.valve_3way = CO");
		device.valve_3way = CO;
	}

	// Apart from all if winter season is OFF, do not heat CO buffer or floor heating distributor!
	if (!device.heatingActivated) {
		if (device.valve_3way != CWU) addLog("device.valve_3way = CWU");
		device.valve_3way = CWU;
	}
}

void buffers() {
	// Requirements to load CO and CWU buffers include hysteresis.

	// number of each 10 minutes left in cheap tariff
	// heat pump need 10 minutes to warm up buffer for 1K degree
	int timeLeft = 0;
	if (getDateTime().hour == 5) timeLeft = (int)((60 - getDateTime().minute) / 10);
	// difference between setTemperature and temperature in buffer
	float temperatureDiff = device.reqTempBuforCO - device.tBuffCOsrodek.isTemp;
	if ((timeLeft != 0) && (temperatureDiff >= timeLeft)) {

		if (!reqCOloadByNight) addLog("CO - dogrzewanie przed koncem nocy");
		reqCOloadByNight = true;
	}
	if (reqCOloadByNight) {
		reqCOload = true;
	}

	if ((device.tBuffCOgora.isTemp<device.reqTempBuforCO-(TEMP_CO_HYSTERESIS/2)) || (device.tBuffCOsrodek.isTemp<=device.reqTempBuforCO-TEMP_CO_HYSTERESIS)) {
		//TMP
		if (!reqCOload) {
			String temp;
			temp = "CO - dogrzewanie -";
			temp += "COdol:";
			temp += device.tBuffCOdol.isTemp;
			temp+= " COsr:";
			temp+= device.tBuffCOsrodek.isTemp;
			temp+= " COgora:";
			temp+= device.tBuffCOgora.isTemp;
			temp+= " req:";
			temp+= device.reqTempBuforCO;
			addLog(temp);
		}

		reqCOload = true;
	}
	// force to load CO during day in cheap tariff
	if ((getDateTime().hour == 13) && (getDateTime().minute <1)) {
		if (!reqCOload) addLog("CO - dogrzewanie poludniowe");

		reqCOload = true;
	}
	if ((device.tBuffCOdol.isTemp>=device.reqTempBuforCO) && (device.tBuffCOsrodek.isTemp>device.reqTempBuforCO) && (device.tBuffCOgora.isTemp>device.reqTempBuforCO)) {
		if (reqCOload) {
			String temp;
			temp = "CO - dogrzewanie KONIEC -";
			temp += "COdol:";
			temp += device.tBuffCOdol.isTemp;
			temp+= " COsr:";
			temp+= device.tBuffCOsrodek.isTemp;
			temp+= " COgora:";
			temp+= device.tBuffCOgora.isTemp;
			temp+= " req:";
			temp+= device.reqTempBuforCO;
			addLog(temp);
		}
		reqCOload = false;
	}

	if (!reqCOload) {
		if (reqCOloadByNight) addLog("CO - koniec dogrzewania nocnego");
		reqCOloadByNight=false;
	}

	//Emergency load when more heating required and time not within Tariff II hours
	if (device.tBuffCOgora.isTemp<TEMP_MIN_ON_DISTRIBUTOR) {
		if (!reqCOloadToCold) addLog("CO - dogrzewanie - Za niska temperatura");

		reqCOloadToCold = true;
	}
	if ((device.tBuffCOdol.isTemp>=TEMP_MIN_ON_DISTRIBUTOR+TEMP_CO_EMERGENCY_HYSTERESIS) &&
			(device.tBuffCOgora.isTemp>=TEMP_MIN_ON_DISTRIBUTOR+TEMP_CO_EMERGENCY_HYSTERESIS)) {
		if (reqCOloadToCold) addLog("CO - dogrzewanie KONIEC - Za niska temperatura");
		reqCOloadToCold = false;
	}

	// jesli temperatura w srodku spadnie lub dol bedzie ponizej 20st wlacz grzanie CWU
	if ((device.tBuffCWUgora.isTemp<(device.reqTempBuforCWU)) || (device.tBuffCWUdol.isTemp<15)) {
		if (!reqCWUload) {
			String temp;
			temp = "CWU - dogrzewanie -";
			temp += "CWUdol:";
			temp += device.tBuffCWUdol.isTemp;
			temp+= " CWUsr:";
			temp+= device.tBuffCWUsrodek.isTemp;
			temp+= " CWUgora:";
			temp+= device.tBuffCWUgora.isTemp;
			temp+= " req:";
			temp+= device.reqTempBuforCWU;
			addLog(temp);
		}
		reqCWUload = true;
	}
	// force to load CWU during day in cheap tariff
	if ((getDateTime().hour == 14) && (getDateTime().minute == 0)) {
		if (!reqCWUload) addLog("CWU - dogrzewanie poludniowe");
		reqCWUload = true;
	}

	// force to load CWU during night in cheap tariff
	if ((getDateTime().hour == 4) && (getDateTime().minute == 30)) {
		if (!reqCWUload) addLog("CWU - dogrzewanie nocne");
		reqCWUload = true;
	}
	//Sprawdzanie temperatury tylko na gorze przy warunku ze na dole przekroczyla pewna stala wartosc
	if ((!cheapTariffHoursActive && device.tBuffCWUgora.isTemp>=(device.reqTempBuforCWU+2))
		|| (cheapTariffHoursActive && (device.tBuffCWUgora.isTemp>=device.heatPumpAlarmTemperature-0.5))
		|| (device.tZasilanie.isTemp==device.heatPumpAlarmTemperature))
	{
		if (reqCWUload) {
			String temp;
			temp = "CWU - dogrzewanie KONIEC -";
			temp += "CWUdol:";
			temp += device.tBuffCWUdol.isTemp;
			temp+= " CWUsr:";
			temp+= device.tBuffCWUsrodek.isTemp;
			temp+= " CWUgora:";
			temp+= device.tBuffCWUgora.isTemp;
			temp+= " req:";
			temp+= device.reqTempBuforCWU;
			addLog(temp);
		}
		reqCWUload = false;
	}

	//Antylegionellia
//	if ((getDateTime().weekDay==6) && (getDateTime().hour==1) && (getDateTime().minute==1)) {
//		if (!device.antyLegionellia) addLog("Antylegionellia - Start");
//		device.antyLegionellia = true;
//	}
//
//	if ((getDateTime().weekDay==6) && (getDateTime().hour==12) && (getDateTime().minute<20)) {
//		if (device.antyLegionellia) addLog("Antylegionellia - Koniec");
//		device.antyLegionellia = false;
//	}
//
//	if ((getDateTime().weekDay < 6) && (getDateTime().hour==5) && (getDateTime().minute==59)) {
//		if (device.antyLegionellia) addLog("Antylegionellia - Wy³¹czenie przed koñcem taryfy nocnej");
//		device.antyLegionellia = false;
//	}

	// in case when heat pump temperature is too high, reset buffers heat requirements
	if ((device.tZasilanie.isTemp >= device.heatPumpAlarmTemperature) && (device.tZasilanie.isTemp<100)) {
		//TMP
		if ((reqCOload || reqCWUload) && (!heatPumpOverheat)) addLog("ALARM OVERHEAT ");

		reqCOload = false;
		reqCWUload = false;

		//heat pump reach alarm temperature
		heatPumpOverheat = true;
		// 15 minutes break when heat pump has over-heated
		heatPumpOverheatDelay = millis();
	}

	// after delay alarm off
	if ((millis() > (heatPumpOverheatDelay + 150000)) && (heatPumpOverheat)) {
		if (heatPumpOverheat) addLog("OVERHEAT Koniec");
		heatPumpOverheat = false;
	}
}

void heatingAndPumps() {
	// Init values
	// First set pumps on OFF
	device.pump_InHouse = false;
	device.pump_UnderGround = false;

	if (((getDateTime().hour >= 22 ) || (getDateTime().hour < 6) ||
		((getDateTime().hour >=13) && (getDateTime().hour <15))) || (getDateTime().weekDay==0) || getDateTime().weekDay==6)	// cheap tariff at the weekend as well
		cheapTariffHoursActive = true;	// cheap tariff hours active
	else cheapTariffHoursActive = false;

	if (device.heatSourceActive != FIREPLACE) device.heatSourceActive = BUFFER_CO;

	if ((device.tZasilanie.isTemp-device.tPowrot.isTemp>3) && (device.reqHeatPumpOn)) heatPumpIsHeating = true;
	else heatPumpIsHeating = false;

	//MODE
	//Independent from system, if KOMINEK working and bring heat to the system, inHouse PUMP must be ON
	//KOMINEK
	if ((device.tKominek.isTemp > 50) && (device.tKominek.isTemp>(device.tPowrot.isTemp+5)))
		device.heatSourceActive = FIREPLACE;
	if ((device.tKominek.isTemp<device.tPowrot.isTemp) || (device.tKominek.isTemp<48)) device.heatSourceActive = BUFFER_CO;

	if (device.heatSourceActive != FIREPLACE) {
		if (!device.cheapTariffOnly)
			device.heatSourceActive = HEAT_PUMP;
		if (device.cheapTariffOnly) {
			if (cheapTariffHoursActive)		// cheap tariff at the weekend as well
				device.heatSourceActive = HEAT_PUMP;	// cheap tariff Heating pump as active heating source
			if ((!cheapTariffHoursActive) && (reqCOloadToCold))				// in case when buffer is too cold in normal hours
				device.heatSourceActive = HEAT_PUMP;
			if ((!cheapTariffHoursActive) && (reqCWUload))				// in case when buffer is too cold in normal hours
				device.heatSourceActive = HEAT_PUMP;
		}
	}

	//CIRCUIT PUMPS
	//CO
	if ((device.circuitOnAmount > 0) ||
			((device.circuitOnAmount == 0) && (device.valve_bypass == 40) && (device.heatingActivated) &&
				(reqCOload) && (device.heatSourceActive != BUFFER_CO)) ||
			(device.heatSourceActive == FIREPLACE))
				device.pump_InHouse = true;

	//If vent system required hot water
	if (ventData.reqHotWater)
		device.pump_InHouse = true;

	//CWU
	if ((reqCWUload) && (device.valve_3way == CWU) && (device.heatSourceActive != BUFFER_CO)) {
			device.pump_InHouse = true;
	}

	//PC Over heated . pumpInhouse working time extend
	if ((heatPumpOverheat) && (millis() < (heatPumpOverheatDelay + 60000))) {
		device.pump_InHouse = true;
	}

	//Request pump underGround if heat pump is required
	if ((device.pump_InHouse) && (device.heatSourceActive == HEAT_PUMP)) {
		if (((reqCOload) || (reqCWUload)) && device.pump_InHouse)
			device.pump_UnderGround= true;
	}

	//If vent system required cold water
	if (ventData.reqColdWater)
		device.pump_UnderGround = true;

	if ((device.pump_UnderGround) && (!heatPumpDelayActiv)) {
		heatPumpDelay = millis()+DELAY_HEAT_PUMP;
		heatPumpDelayActiv = true;;
	}

	currentMillis = millis();
	//If Underground pump is on, shut PC after delay
	if ((device.heatSourceActive == HEAT_PUMP) && (device.pump_UnderGround) && (device.pump_InHouse)
			&& (currentMillis>heatPumpDelay)) {
		device.reqHeatPumpOn = true;
	}
	if (!device.pump_UnderGround || !device.pump_InHouse || ((device.tZasilanie.isTemp>=device.heatPumpAlarmTemperature) && (device.tZasilanie.isTemp<100))) {
		device.reqHeatPumpOn = false;
		heatPumpDelayActiv = false;
	}
}

void outputs() {
	//															WARNING!!!
	//							!!!SIGNALS ARE NEGATIVE BECAUSE OF RELAYS TYPE! RELAYS ARE TRIGGERD WITH SIGNAL LOW!!!

	//VALVES
	//3_WAY
	if ((device.valve_3way == CO) && (device.valve_3wayLastState == CWU)) {
		digitalWrite(VALVE_3WAY_CWU,1);
		delay(100);
		digitalWrite(VALVE_3WAY_CO,0);
		delay(100);

		device.valve_3wayLastState =CO;
		valve_3way_activatedMillis = millis();

		//TMP
		Serial.println("PRZEJAZD 3WAY DO CO");
	}
	if ((device.valve_3way == CWU) && (device.valve_3wayLastState == CO)) {
		digitalWrite(VALVE_3WAY_CO,1);
		delay(100);
		digitalWrite(VALVE_3WAY_CWU,0);
		delay(100);
		device.valve_3wayLastState =CWU;
		valve_3way_activatedMillis = millis();

		//TMP
		Serial.println("PRZEJAZD 3WAY DO CWU");
	}

	currentMillis = millis();

	//Turn relays off when 3-WAY rich end position
	if (currentMillis-valve_3way_activatedMillis > VALVE_3WAY_RIDE_TIME) {
		digitalWrite(VALVE_3WAY_CWU,1);
		digitalWrite(VALVE_3WAY_CO,1);

	}

	//BYPASS

	if ((!device.valve_bypass_reachPos) && (device.valve_bypass == device.valve_bypass_realPos)) {
		device.valve_bypass_reachPos = true;
		digitalWrite(VALVE_BYPASS_CLOSE,1);
		digitalWrite(VALVE_BYPASS_OPEN,1);

		//TMP
		Serial.println("KONIEC PRZEJAZDU BYPASS");

		device.valve_bypass_moveFlag = false;
		delay(100);
		// If reach end positions. Run 5s longer to reduce position tolerance
		if (device.valve_bypass_realPos == 0) {
			digitalWrite(VALVE_BYPASS_CLOSE,0);
			delay(7000);
			digitalWrite(VALVE_BYPASS_CLOSE,1);
			digitalWrite(VALVE_BYPASS_OPEN,1);
		}
		if (device.valve_bypass_realPos == 40) {
			digitalWrite(VALVE_BYPASS_OPEN,0);
			delay(7000);
			digitalWrite(VALVE_BYPASS_CLOSE,1);
			digitalWrite(VALVE_BYPASS_OPEN,1);
		}
	}

	currentMillis = millis();
	if (device.valve_bypass < device.valve_bypass_realPos) {
		digitalWrite(VALVE_BYPASS_OPEN,1);
		if (digitalRead(VALVE_BYPASS_OPEN)) digitalWrite(VALVE_BYPASS_CLOSE,0);
		device.valve_bypass_reachPos = false;
		if ((device.valve_bypass_moveFlag) && ((millis()-valve_bypass_inMove) >= BYPASS_STEP)) {
			//TMP
			int temp = millis()-valve_bypass_inMove;
			Serial.printf("Bypass przejazd [%d]",temp);
			Serial.println();
			if (temp>1800) {
				device.valve_bypass_realPos-=2;
				device.valve_bypass--;
			}
			else device.valve_bypass_realPos--;

			device.valve_bypass_moveFlag = false;
		}
		if (!device.valve_bypass_moveFlag) {
			valve_bypass_inMove = millis();
			device.valve_bypass_moveFlag = true;
		}
	}

	if (device.valve_bypass > device.valve_bypass_realPos) {
		digitalWrite(VALVE_BYPASS_CLOSE,1);
		if (digitalRead(VALVE_BYPASS_CLOSE)) digitalWrite(VALVE_BYPASS_OPEN,0);
		device.valve_bypass_reachPos = false;
		if (!device.valve_bypass_moveFlag) {
			valve_bypass_inMove = millis();
			device.valve_bypass_moveFlag = true;
		}
		if ((device.valve_bypass_moveFlag) && ((millis()-valve_bypass_inMove) >= BYPASS_STEP)) {
			int temp = millis()-valve_bypass_inMove;
			Serial.printf("Bypass przejazd [%d]",temp);
			Serial.println();
			if (temp>1800) {
				device.valve_bypass_realPos+=2;
				device.valve_bypass++;
			}
			else device.valve_bypass_realPos++;

			device.valve_bypass_moveFlag = false;
		}
	}

	// CIRCUIT RELAYS
	//ZONES
	digitalWrite(ZONE0,!device.zone[0].circuit);
	digitalWrite(ZONE1,!device.zone[1].circuit);
	digitalWrite(ZONE2,!device.zone[2].circuit);
	digitalWrite(ZONE3,!device.zone[3].circuit);
	digitalWrite(ZONE4,!device.zone[4].circuit);
	digitalWrite(ZONE5,!device.zone[5].circuit);
	digitalWrite(ZONE6,!device.zone[6].circuit);

	//Request heating pump ON (CO)
	digitalWrite(RELAY_HEAT_PUMP_ON,!device.reqHeatPumpOn);

	//CIRCULATION PUMP
	digitalWrite(PUMP_IN_HOUSE,!device.pump_InHouse);
	digitalWrite(PUMP_UNDER_HOUSE,!device.pump_UnderGround);

	//RELAY ANTILEGIONELLIA
	digitalWrite(RELAY_ANTILEGIONELLIA, !device.antyLegionellia);
}


void setUDPdata() {
	int size = 19;
	byte dataWrite[size];

	dataWrite[0] = (device.heatSourceActive << 6) | (device.pump_InHouse << 5) | (device.pump_UnderGround << 4) | (device.reqHeatPumpOn << 3) |
			(device.cheapTariffOnly << 2) | (device.heatingActivated << 1) | (device.antyLegionellia) << 0;

	dataWrite[1] = (device.valve_3way << 6) | (device.valve_bypass << 0);
	dataWrite[2] = (device.zone[0].circuit << 7) | (device.zone[1].circuit << 6) | (device.zone[2].circuit << 5) | (device.zone[3].circuit << 4)
					| (device.zone[4].circuit << 3) | (device.zone[5].circuit << 2) | (device.zone[6].circuit << 1);
	dataWrite[3] = device.reqTempBuforCO*2;
	dataWrite[4] = device.reqTempBuforCWU*2;

	dataWrite[5] = device.tBuffCOdol.isTemp*2;
	dataWrite[6] = device.tBuffCOsrodek.isTemp*2;
	dataWrite[7] = device.tBuffCOgora.isTemp*2;

	dataWrite[8] = device.tBuffCWUdol.isTemp*2;
	dataWrite[9] = device.tBuffCWUsrodek.isTemp*2;
	dataWrite[10] = device.tBuffCWUgora.isTemp*2;

	dataWrite[11] = device.tZasilanie.isTemp*2;
	dataWrite[12] = device.tPowrot.isTemp*2;
	dataWrite[13] = device.tDolneZrodlo.isTemp*2;
	dataWrite[14] = device.tKominek.isTemp*2;
	dataWrite[15] = device.tRozdzielacze.isTemp*2;
	dataWrite[16] = device.tPowrotParter.isTemp*2;
	dataWrite[17] = device.tPowrotPietro.isTemp*2;
	dataWrite[18] = device.heatPumpAlarmTemperature;

	setUDPdata(0, dataWrite,size);

}

void statusUpdate() {
	String status;
	status = "Zasilanie:";
	if (device.heatSourceActive == 1) status += " PCi";
	if (device.heatSourceActive == 2) status += " Bufor";
	if (device.heatSourceActive == 3) status += " Kominek";
	status += "\tpompa inHouse["; status += device.pump_InHouse; status += "]";
	status +="\tpompa underGround["; status += device.pump_UnderGround; status += "]\n";

	status += "IItaryfa["; status += device.cheapTariffOnly; status += "]";
	status += "\tOgrz.Aktyw["; status += device.heatingActivated; status += "]";
	status += "\treqPCi["; status += device.reqHeatPumpOn; status += "]";
	status += "\tantyLegio["; status += device.antyLegionellia; status += "]";
	status += "\talarmHeatingPumpTemp["; status+= device.heatPumpAlarmTemperature; status += "]";
	if (device.valve_3way == 1) status += "\t3way[CO]";
	if (device.valve_3way == 2) status += "\t3way[CWU]";

	status += "\nByPass["; status += device.valve_bypass; status += "]\tReal["; status += device.valve_bypass_realPos; status += "]\tReach[";
	status += device.valve_bypass_reachPos;  status += "]\tlastCorrectReadTempZrodla[";
	status += "]\tFlag["; status += device.valve_bypass_moveFlag;  status += "]";

	status += "\nCIRCUIT";
	if (device.circuitOnAmount != 0) { status +="\tcirOnAmo["; status +=device.circuitOnAmount; status +="]";}
	for (int i=0; i<ZONE_QUANTITY; i++)
		if (device.zone[i].circuit != 0) { status +=" Zone"; status +=i; status +="["; status +=device.zone[i].circuit; status +="]";}
	status +="Amount["; status +=device.circuitOnAmount; status +="]";

	status += "\nPOWROT T>";
	status +="\tTpowrotParter["; status +=device.tPowrotParter.isTemp; status +="]["; status += device.tPowrotParter.maxErrorCount; status +="]";
	status +="\tTpowrotPietro["; status +=device.tPowrotPietro.isTemp; status +="]["; status += device.tPowrotPietro.maxErrorCount; status +="]";

	status += "\nTEMP OBIEGU";
	status +="\tTZasilanie["; status +=device.tZasilanie.isTemp; status +="]["; status += device.tZasilanie.maxErrorCount; status +="]";
	status +="\tTpowrot["; status +=device.tPowrot.isTemp; status +="]["; status += device.tPowrot.maxErrorCount; status +="]";
	status +="\tTKominek["; status +=device.tKominek.isTemp; status +="]["; status += device.tKominek.maxErrorCount; status +="]";
	status +="\tTrozdz["; status +=device.tRozdzielacze.isTemp; status +="]["; status += device.tRozdzielacze.maxErrorCount; status +="]";

	status += "\nBUFORY";
	status +="\tTCOd["; status +=device.tBuffCOdol.isTemp; status +="]["; status += device.tBuffCOdol.maxErrorCount; status +="]";
	status +="\tTCOs["; status +=device.tBuffCOsrodek.isTemp; status +="]["; status += device.tBuffCOsrodek.maxErrorCount; status +="]";
	status +="\tTCOg["; status +=device.tBuffCOgora.isTemp; status +="]["; status += device.tBuffCOgora.maxErrorCount; status +="]";
	status +="\tTCWUd["; status +=device.tBuffCWUdol.isTemp; status +="]["; status += device.tBuffCWUdol.maxErrorCount; status +="]";
	status +="\tTCWUs["; status +=device.tBuffCWUsrodek.isTemp; status +="]["; status += device.tBuffCWUsrodek.maxErrorCount; status +="]";
	status +="\tTCWUg["; status +=device.tBuffCWUgora.isTemp; status +="]["; status += device.tBuffCWUgora.maxErrorCount; status +="]";

	status += "\nBUFOR SET";
	status +="\treqCO["; status +=device.reqTempBuforCO; status +="]";
	status +="\treqCWU["; status +=device.reqTempBuforCWU; status +="]";

	for (int i=0; i<7; i++) {
		status +="\nZone["; status +=i; status += "]:\t\t T="; status +=device.zone[i].isTemp; status +="[stC]\treqT="; status+=device.zone[i].reqTemp; status +="[stC]";
	}

	status += "\nVent Params";
	status += addStatus("\nreqColdWater", ventData.reqColdWater);
	status += addStatus("\nreqHotWater", ventData.reqHotWater);

	setStatus(status);
}
