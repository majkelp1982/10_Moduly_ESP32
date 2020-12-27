#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "Basic.h"
#include "Module.h"
#include "Status.h"

OneWire oneWireBuffers(ONE_WIRE_BUFFERS);
OneWire oneWireDevices(ONE_WIRE_DEVICES);
DallasTemperature sensorsBuffers(&oneWireBuffers);					// initialized 1-WIRE for buffers
DallasTemperature sensorsDevices(&oneWireDevices);					// initialized 1-WIRE for devices

DateTime dtTime;

// Variables
int lastCircuitOnAmount = 0;
bool valve_bypass_CloseDelayActive = false;

//Heat pump
bool cheapTariffHoursActive = false;
bool heatPumpIsHeating = false;
bool heatPumpDelayActiv = false;
bool heatPumpOverheat = false;

//Buffers
bool reqCOload = false;
bool reqCOloadToCold = false;
bool reqCOloadByNight = false;

bool reqCWUload = false;
bool reqCWUloadByNight = false;

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

//Functions
void DALLAS18b20Read (Device *device, Thermometer *thermometer);
void DALLAS18b20ReadTemperatures(Device *device);
void Valves_init(Device *device);
void Valves(Device *device);
void Buffers(Device *device);
void HeatingAndPumps(Device *device);
void Outputs(Device *device);

void Module_init() {
	sensorsBuffers.begin();
	sensorsDevices.begin();
	sensorsBuffers.setResolution(10);
	sensorsDevices.setResolution(10);
	sensorsBuffers.setWaitForConversion(false);
	sensorsDevices.setWaitForConversion(false);
}

void Module(Device *device, DateTime datTime) {
	// Get actual temperatures
	dtTime = datTime;
	DALLAS18b20ReadTemperatures(device);
	Valves(device);
	Buffers(device);
	HeatingAndPumps(device);
	Outputs(device);
}

void Valves_init(Device *device) {
	//Drive valve to position full open and set real position on 20
	Serial.println("Valve Bypass & 3-way (CO/CWU) initialization...");
	digitalWrite(VALVE_BYPASS_CLOSE,1);
	digitalWrite(VALVE_3WAY_CO,1);
	while (!digitalRead(VALVE_BYPASS_CLOSE));
	while (!digitalRead(VALVE_3WAY_CO));
	delay(1000);
	digitalWrite(VALVE_3WAY_CWU,0);
	digitalWrite(VALVE_BYPASS_OPEN,0);
	//40 second needed to full open from close position
	for (int i=40; i>=0; i--) {
		delay(1000);
		addLog((String)i, dtTime);
	}
	digitalWrite(VALVE_BYPASS_OPEN,1);
	digitalWrite(VALVE_3WAY_CWU,1);
	device->valve_3wayLastState = CWU;
	device->valve_bypass_realPos = 40;
	device->valve_bypass = 40;
	Serial.println("Valve Bypass OK");
}

void Valves(Device *device) {
	// set Valve from distributor if temperature to low including hysteresis. Else, when temperature achieve setTemp turn off circuit valve

	//Zone circuits
	for (int i=0; i<ZONE_QUANTITY; i++) {
		if ((float)device->zone[i].isTemp < (device->zone[i].reqTemp-(float)TEMP_HYSTERESIS)) device->zone[i].circuit = true;
		if (device->zone[i].isTemp >= device->zone[i].reqTemp) device->zone[i].circuit = false;
	}

	//If system is OFF (according to device mode) set all circuit OFF!
	if ((!device->heatingActivated) || (device->valve_3way == CWU))
		for (int i=0; i<ZONE_QUANTITY; i++)
			device->zone[i].circuit = false;

	//count number of circuit on
	// get last amount of circuit open
	lastCircuitOnAmount = device->circuitOnAmount;

	device->circuitOnAmount = 0;
	for (int i=0; i<ZONE_QUANTITY; i++) {
		if (device->zone[i].circuit) device->circuitOnAmount++;
	}

	//Other valves
	// if all of distribution valves are closed, bypass MUST BE OPEN to avoid water flow blocking
	currentMillis = millis();

	// recalculate how many circuit open
	if (device->circuitOnAmount == 0) {
		device->valve_bypass = 40;
		valve_bypass_CloseDelayActive = false;
	}
	if (device->circuitOnAmount>0) {

		// Delay to avoid bypass close when thermoheads are cold and not open. Delay should be not less then 180s.
		// After this time bypass valve can be fully open
		if ((!valve_bypass_CloseDelayActive) && (lastCircuitOnAmount == 0)) {
			valve_bypass_CloseDelayActive = true;
			valve_bypass_CloseDelayMillis = currentMillis;
		}
		if ((valve_bypass_CloseDelayActive) && ((currentMillis-valve_bypass_CloseDelayMillis) < DELAY_THERMO_HEADS)) {
			device->valve_bypass = 40;
			return;
		}

		// When delay is run out already, set start pos to bypass depending on actual heat source
		if (valve_bypass_CloseDelayActive) {
			if (device->heatSourceActive == HEAT_PUMP) device->valve_bypass = HEAT_PUMP_BYP_START_POS;
			else device->valve_bypass = 0;
			valve_bypass_CloseDelayActive = false;
		}

		currentMillis = millis();
		if (device->tZasilanie.isTemp != 0) {
			if (device->tZasilanie.isTemp > HEAT_PUMP_WARNING_TEMPERATURE+HEAT_PUMP_HISTERESIS_HI) {
				if ((currentMillis-heatPumpTempCorrectionMillis) > DELAY_HEAT_PUMP_OUT_TEMP_TO_HIGH) {
					device->valve_bypass += HEAT_PUMP_BYP_UPSTEP_WHEN_HI;
					heatPumpTempCorrectionMillis = currentMillis;
				}
			}
			if (device->tZasilanie.isTemp < HEAT_PUMP_WARNING_TEMPERATURE-HEAT_PUMP_HISTERESIS_LOW) {
				if ((currentMillis-heatPumpTempCorrectionMillis) > DELAY_HEAT_PUMP_OUT_TEMP_TO_LOW) {
					device->valve_bypass-=HEAT_PUMP_BYP_DOWNSTEP_WHEN_LOW;
					heatPumpTempCorrectionMillis = currentMillis;
				}
			}
		}

		//min-max values of bypass 0-40
		if (device->valve_bypass>40) device->valve_bypass=40;
		if (device->valve_bypass<0) device->valve_bypass=0;

		// kiedy uruchomiona pompa obiegowa a pompa zalacza sie ze zwloka, przejedz bypass do pozycji start z ktorej bedzie regulowany
		if ((device->pump_UnderGround) && (!device->reqHeatPumpOn)) {
			device->valve_bypass = HEAT_PUMP_BYP_START_POS;
		}
		//Zamknij bypass jak pompa ciepla nie dziala
		if (!device->pump_UnderGround) device->valve_bypass = 0;
		//Nie koryguj bypass jak pompa ciepla nie laduje ciepla. (moze miec przestuj. Po wznowie za szybko nabije cieplo)
		if ((device->pump_UnderGround) && (device->reqHeatPumpOn) && (!heatPumpIsHeating)) {
			device->valve_bypass = HEAT_PUMP_BYP_START_POS;
			if (!device->valve_bypass_reachPos) Serial.println("Pompa nie grzeje. [HEAT_PUMP_BYP_START_POS]");
		}
		//Kiedy temperatura alarm-2 temp zmniejsz szybko bypass o 10
		if (device->tZasilanie.isTemp == device->heatPumpAlarmTemperature-2) device->valve_bypass -=10;

		//TMP!!! When thermometer fault
		if ((device->reqHeatPumpOn) && (device->tZasilanie.errorCount >= 3)
				&& (device->valve_bypass_realPos != HEAT_PUMP_BYP_START_POS)) {
			if (device->valve_bypass != HEAT_PUMP_BYP_START_POS) addLog("Blad odczytu temperatury po zrodlach. [HEAT_PUMP_BYP_START_POS]", dtTime);
			device->valve_bypass = HEAT_PUMP_BYP_START_POS;
		}
	}

	// Valve 3_way is set, when hot water is to cold (check on top of buffer) 3-way valves priority set to heat CWU (hot water to use)
	if ((reqCWUload) && (device->heatSourceActive != BUFFER_CO) && (!reqCOloadToCold)) {
		if ((device->valve_3way != CWU) && (device->heatingActivated)) addLog("device->valve_3way = CWU", dtTime);
		device->valve_3way = CWU;
	}

	// When water temperature measured bottom of buffer CWU achieve set temperature, 3-way run to position HOUSE HEATING.
	if (((!reqCWUload) || (reqCOloadToCold)) && (device->heatingActivated))  {
		if ((device->valve_3way != CO) && (device->heatingActivated)) addLog("device->valve_3way = CO", dtTime);
		device->valve_3way = CO;
	}

	// Apart from all if winter season is OFF, do not heat CO buffer or floor heating distributor!
	if (!device->heatingActivated) {
		if (device->valve_3way != CWU) addLog("device->valve_3way = CWU", dtTime);
		device->valve_3way = CWU;
	}
}

void Buffers(Device *device) {
	// Requirements to load CO and CWU buffers include hysteresis.

	// number of each 10 minutes left in cheap tariff
	// heat pump need 10 minutes to warm up buffer for 1K degree
	int timeLeft = 0;
	if (dtTime.hour == 5) timeLeft = (int)((60 - dtTime.minute) / 10);
	// difference between setTemperature and temperature in buffer
	float temperatureDiff = device->reqTempBuforCO - device->tBuffCOsrodek.isTemp;
	if ((timeLeft != 0) && (temperatureDiff >= timeLeft)) {

		if (!reqCOloadByNight) addLog("CO - dogrzewanie przed koncem nocy", dtTime);
		reqCOloadByNight = true;
	}
	if (reqCOloadByNight) {
		reqCOload = true;
	}

	if ((device->tBuffCOgora.isTemp<device->reqTempBuforCO) || (device->tBuffCOsrodek.isTemp<=device->reqTempBuforCO-TEMP_CO_HYSTERESIS)) {
		//TMP
		if (!reqCOload) {
			String temp;
			temp = "CO - dogrzewanie -";
			temp += "COdol:";
			temp += device->tBuffCOdol.isTemp;
			temp+= " COsr:";
			temp+= device->tBuffCOsrodek.isTemp;
			temp+= " COgora:";
			temp+= device->tBuffCOgora.isTemp;
			temp+= " req:";
			temp+= device->reqTempBuforCO;
			addLog(temp, dtTime);
		}

		reqCOload = true;
	}
	// force to load CO during day in cheap tariff
	if ((dtTime.hour == 13) && (dtTime.minute <2)) {
		if (!reqCOload) addLog("CO - dogrzewanie poludniowe", dtTime);

		reqCOload = true;
	}
	if ((device->tBuffCOdol.isTemp>=device->reqTempBuforCO) && (device->tBuffCOsrodek.isTemp>device->reqTempBuforCO) && (device->tBuffCOgora.isTemp>device->reqTempBuforCO)) {
		if (reqCOload) {
			String temp;
			temp = "CO - dogrzewanie KONIEC -";
			temp += "COdol:";
			temp += device->tBuffCOdol.isTemp;
			temp+= " COsr:";
			temp+= device->tBuffCOsrodek.isTemp;
			temp+= " COgora:";
			temp+= device->tBuffCOgora.isTemp;
			temp+= " req:";
			temp+= device->reqTempBuforCO;
			addLog(temp, dtTime);
		}
		reqCOload = false;
	}

	if (!reqCOload) {
		if (reqCOloadByNight) addLog("CO - koniec dogrzewania nocnego", dtTime);
		reqCOloadByNight=false;
	}

	//Emergency load when more heating required and time not within Tariff II hours
	if (device->tBuffCOgora.isTemp<TEMP_MIN_ON_DISTRIBUTOR) {
		if (!reqCOloadToCold) addLog("CO - dogrzewanie - Za niska temperatura", dtTime);

		reqCOloadToCold = true;
	}
	if ((device->tBuffCOdol.isTemp>=TEMP_MIN_ON_DISTRIBUTOR+TEMP_CO_EMERGENCY_HYSTERESIS) &&
			(device->tBuffCOgora.isTemp>=TEMP_MIN_ON_DISTRIBUTOR+TEMP_CO_EMERGENCY_HYSTERESIS)) {
		if (reqCOloadToCold) addLog("CO - dogrzewanie KONIEC - Za niska temperatura", dtTime);
		reqCOloadToCold = false;
	}

	// jesli temperatura w srodku spadnie lub dol bedzie ponizej 20st wlacz grzanie CWU
	if ((device->tBuffCWUsrodek.isTemp<(device->reqTempBuforCWU-TEMP_CWU_HYSTERESIS)) || (device->tBuffCWUdol.isTemp<15)) {
		if (!reqCWUload) {
			String temp;
			temp = "CWU - dogrzewanie -";
			temp += "CWUdol:";
			temp += device->tBuffCWUdol.isTemp;
			temp+= " CWUsr:";
			temp+= device->tBuffCWUsrodek.isTemp;
			temp+= " CWUgora:";
			temp+= device->tBuffCWUgora.isTemp;
			temp+= " req:";
			temp+= device->reqTempBuforCWU;
			addLog(temp, dtTime);
		}
		reqCWUload = true;
	}
	// force to load CWU during day in cheap tariff
	if ((dtTime.hour == 13) && (dtTime.minute <2)) {
		if (!reqCWUload) addLog("CWU - dogrzewanie poludniowe", dtTime);
		reqCWUload = true;
	}

	//Sprawdzanie temperatury tylko na gorze przy warunku ze na dole przekroczyla pewna stala wartosc
	if (((device->tBuffCWUdol.isTemp>=43) || (device->tBuffCWUdol.isTemp>=device->reqTempBuforCWU))
			&& (device->tBuffCWUsrodek.isTemp>=device->reqTempBuforCWU)
			&& (device->tBuffCWUgora.isTemp>=device->reqTempBuforCWU))
	{
		if (reqCWUload) {
			String temp;
			temp = "CWU - dogrzewanie KONIEC -";
			temp += "CWUdol:";
			temp += device->tBuffCWUdol.isTemp;
			temp+= " CWUsr:";
			temp+= device->tBuffCWUsrodek.isTemp;
			temp+= " CWUgora:";
			temp+= device->tBuffCWUgora.isTemp;
			temp+= " req:";
			temp+= device->reqTempBuforCWU;
			addLog(temp, dtTime);
		}
		reqCWUload = false;
	}

	//Antylegionellia
	if ((dtTime.weekDay==6) && (dtTime.hour==1) && (dtTime.minute<10)) {
		if (!device->antyLegionellia) addLog("Antylegionellia - Start", dtTime);
		device->antyLegionellia = true;
	}

	if ((dtTime.weekDay==6) && (dtTime.hour==12) && (dtTime.minute<20)) {
		if (device->antyLegionellia) addLog("Antylegionellia - Koniec", dtTime);
		device->antyLegionellia = false;
	}

	// in case when heat pump temperature is too high, reset buffers heat requirements
	if ((device->tZasilanie.isTemp >= device->heatPumpAlarmTemperature) && (device->tZasilanie.isTemp<100)) {
		//TMP
		if ((reqCOload || reqCWUload) && (!heatPumpOverheat)) addLog("ALARM OVERHEAT ",dtTime);

		reqCOload = false;
		reqCWUload = false;

		//heat pump reach alarm temperature
		heatPumpOverheat = true;
		// 15 minutes break when heat pump has over-heated
		heatPumpOverheatDelay = millis();
	}

	// after delay alarm off
	if ((millis() > (heatPumpOverheatDelay + 150000)) && (heatPumpOverheat)) {
		if (heatPumpOverheat) addLog("OVERHEAT Koniec",dtTime);
		heatPumpOverheat = false;
	}
}

void HeatingAndPumps(Device *device) {
	// Init values
	// First set pumps on OFF
	device->pump_InHouse = false;
	device->pump_UnderGround = false;

	if (((dtTime.hour >= 22 ) || (dtTime.hour < 6) ||
		((dtTime.hour >=13) && (dtTime.hour <15))) || (dtTime.weekDay==0) || dtTime.weekDay==6)	// cheap tariff at the weekend as well
		cheapTariffHoursActive = true;	// cheap tariff hours active
	else cheapTariffHoursActive = false;

	if (device->heatSourceActive != FIREPLACE) device->heatSourceActive = BUFFER_CO;

	if ((device->tZasilanie.isTemp-device->tPowrot.isTemp>3) && (device->reqHeatPumpOn)) heatPumpIsHeating = true;
	else heatPumpIsHeating = false;

	//MODE
	//Independent from system, if KOMINEK working and bring heat to the system, inHouse PUMP must be ON
	//KOMINEK
	if ((device->tKominek.isTemp > 50) && (device->tKominek.isTemp>(device->tPowrot.isTemp+5)))
		device->heatSourceActive = FIREPLACE;
	if ((device->tKominek.isTemp<device->tPowrot.isTemp) || (device->tKominek.isTemp<48)) device->heatSourceActive = BUFFER_CO;

	if (device->heatSourceActive != FIREPLACE) {
		if (!device->cheapTariffOnly)
			device->heatSourceActive = HEAT_PUMP;
		if (device->cheapTariffOnly) {
			if (cheapTariffHoursActive)		// cheap tariff at the weekend as well
				device->heatSourceActive = HEAT_PUMP;	// cheap tariff Heating pump as active heating source
			if ((!cheapTariffHoursActive) && (reqCOloadToCold))				// in case when buffer is too cold in normal hours
				device->heatSourceActive = HEAT_PUMP;
			if ((!cheapTariffHoursActive) && (reqCWUload))				// in case when buffer is too cold in normal hours
				device->heatSourceActive = HEAT_PUMP;
		}
	}

	//CIRCUIT PUMPS
	//CO
	if ((device->circuitOnAmount > 0) ||
			((device->circuitOnAmount == 0) && (device->valve_bypass == 40) && (device->heatingActivated) &&
				(reqCOload) && (device->heatSourceActive != BUFFER_CO)) ||
			(device->heatSourceActive == FIREPLACE))
				device->pump_InHouse = true;

	//CWU
	if ((reqCWUload) && (device->valve_3way == CWU) && (device->heatSourceActive != BUFFER_CO)) {
			device->pump_InHouse = true;
	}

	//PC Over heated -> pumpInhouse working time extend
	if ((heatPumpOverheat) && (millis() < (heatPumpOverheatDelay + 60000))) {
		device -> pump_InHouse = true;
	}


	//Request to turn pump underGround if heat pump is required
	if ((device->pump_InHouse) && (device->heatSourceActive == HEAT_PUMP)) {
		if ((reqCOload) || (reqCWUload))
			device->pump_UnderGround= true;
	}

	if ((device->pump_UnderGround) && (!heatPumpDelayActiv)) {
		heatPumpDelay = millis()+DELAY_HEAT_PUMP;
		heatPumpDelayActiv = true;
	}

	currentMillis = millis();
	//If Underground pump is on, shut PC after delay
	if ((device->heatSourceActive == HEAT_PUMP) && (device->pump_UnderGround) && (device->pump_InHouse)
			&& (currentMillis>heatPumpDelay)) {
		device->reqHeatPumpOn = true;
	}
	if ((!device->pump_UnderGround) || ((device->tZasilanie.isTemp>=device->heatPumpAlarmTemperature) && (device->tZasilanie.isTemp<100))) {
		device->reqHeatPumpOn = false;
		heatPumpDelayActiv = false;
	}
}

void Outputs(Device *device) {
	//															WARNING!!!
	//							!!!SIGNALS ARE NEGATIVE BECAUSE OF RELAYS TYPE! RELAYS ARE TRIGGERD WITH SIGNAL LOW!!!

	//VALVES
	//3_WAY
	if ((device->valve_3way == CO) && (device->valve_3wayLastState == CWU)) {
		digitalWrite(VALVE_3WAY_CWU,1);
		delay(100);
		digitalWrite(VALVE_3WAY_CO,0);
		delay(100);

		device->valve_3wayLastState =CO;
		valve_3way_activatedMillis = millis();

		//TMP
		Serial.println("PRZEJAZD 3WAY DO CO");
	}
	if ((device->valve_3way == CWU) && (device->valve_3wayLastState == CO)) {
		digitalWrite(VALVE_3WAY_CO,1);
		delay(100);
		digitalWrite(VALVE_3WAY_CWU,0);
		delay(100);
		device->valve_3wayLastState =CWU;
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

	if ((!device->valve_bypass_reachPos) && (device->valve_bypass == device->valve_bypass_realPos)) {
		device->valve_bypass_reachPos = true;
		digitalWrite(VALVE_BYPASS_CLOSE,1);
		digitalWrite(VALVE_BYPASS_OPEN,1);

		//TMP
		Serial.println("KONIEC PRZEJAZDU BYPASS");

		device->valve_bypass_moveFlag = false;
		delay(100);
		// If reach end positions. Run 5s longer to reduce position tolerance
		if (device->valve_bypass_realPos == 0) {
			digitalWrite(VALVE_BYPASS_CLOSE,0);
			delay(7000);
			digitalWrite(VALVE_BYPASS_CLOSE,1);
			digitalWrite(VALVE_BYPASS_OPEN,1);
		}
		if (device->valve_bypass_realPos == 40) {
			digitalWrite(VALVE_BYPASS_OPEN,0);
			delay(7000);
			digitalWrite(VALVE_BYPASS_CLOSE,1);
			digitalWrite(VALVE_BYPASS_OPEN,1);
		}
	}

	currentMillis = millis();
	if (device->valve_bypass < device->valve_bypass_realPos) {
		digitalWrite(VALVE_BYPASS_OPEN,1);
		if (digitalRead(VALVE_BYPASS_OPEN)) digitalWrite(VALVE_BYPASS_CLOSE,0);
		device->valve_bypass_reachPos = false;
		if ((device->valve_bypass_moveFlag) && ((millis()-valve_bypass_inMove) >= BYPASS_STEP)) {
			//TMP
			int temp = millis()-valve_bypass_inMove;
			Serial.printf("Bypass przejazd [%d]",temp);
			Serial.println();
			if (temp>1800) {
				device->valve_bypass_realPos-=2;
				device->valve_bypass--;
			}
			else device->valve_bypass_realPos--;

			device->valve_bypass_moveFlag = false;
		}
		if (!device->valve_bypass_moveFlag) {
			valve_bypass_inMove = millis();
			device->valve_bypass_moveFlag = true;
		}
	}

	if (device->valve_bypass > device->valve_bypass_realPos) {
		digitalWrite(VALVE_BYPASS_CLOSE,1);
		if (digitalRead(VALVE_BYPASS_CLOSE)) digitalWrite(VALVE_BYPASS_OPEN,0);
		device->valve_bypass_reachPos = false;
		if (!device->valve_bypass_moveFlag) {
			valve_bypass_inMove = millis();
			device->valve_bypass_moveFlag = true;
		}
		if ((device->valve_bypass_moveFlag) && ((millis()-valve_bypass_inMove) >= BYPASS_STEP)) {
			int temp = millis()-valve_bypass_inMove;
			Serial.printf("Bypass przejazd [%d]",temp);
			Serial.println();
			if (temp>1800) {
				device->valve_bypass_realPos+=2;
				device->valve_bypass++;
			}
			else device->valve_bypass_realPos++;

			device->valve_bypass_moveFlag = false;
		}
	}

	// CIRCUIT RELAYS
	//ZONES
	digitalWrite(ZONE0,!device->zone[0].circuit);
	digitalWrite(ZONE1,!device->zone[1].circuit);
	digitalWrite(ZONE2,!device->zone[2].circuit);
	digitalWrite(ZONE3,!device->zone[3].circuit);
	digitalWrite(ZONE4,!device->zone[4].circuit);
	digitalWrite(ZONE5,!device->zone[5].circuit);
	digitalWrite(ZONE6,!device->zone[6].circuit);

	//Request heating pump ON (CO)
	digitalWrite(RELAY_HEAT_PUMP_ON,!device->reqHeatPumpOn);

	//CIRCULATION PUMP
	digitalWrite(PUMP_IN_HOUSE,!device->pump_InHouse);
	digitalWrite(PUMP_UNDER_HOUSE,!device->pump_UnderGround);

	//RELAY ANTILEGIONELLIA
	digitalWrite(RELAY_ANTILEGIONELLIA, !device->antyLegionellia);
}

void DALLAS18b20ReadTemperatures(Device *device)
{
	if (Sleep(&dallasReadMillis, DELAY_DS18B20_READ)) return;

	// Get actual temperatures and compensate
	DALLAS18b20Read(device, &device->tBuffCOdol);
	DALLAS18b20Read(device, &device->tBuffCOsrodek);
	DALLAS18b20Read(device, &device->tBuffCOgora);

	DALLAS18b20Read(device, &device->tBuffCWUdol);
	DALLAS18b20Read(device, &device->tBuffCWUsrodek);
	DALLAS18b20Read(device, &device->tBuffCWUgora);

	DALLAS18b20Read(device, &device->tZasilanie);
	DALLAS18b20Read(device, &device->tPowrot);
	DALLAS18b20Read(device, &device->tDolneZrodlo);

	DALLAS18b20Read(device, &device->tKominek);
	DALLAS18b20Read(device, &device->tRozdzielacze);
	DALLAS18b20Read(device, &device->tPowrotParter);
	DALLAS18b20Read(device, &device->tPowrotPietro);

	sensorsBuffers.requestTemperatures();				// Request temperature
	sensorsDevices.requestTemperatures();				// Request temperature
}

void DALLAS18b20Read (Device *device, Thermometer *thermometer) {
	DeviceAddress deviceAddress;
	for (int i=0; i<8; i++) deviceAddress[i] = thermometer->deviceAddress[i];
	float tempC = 0;
	float a=0;
	float b=0;
	bool buffers = false;
	bool devices = false;


	if (memcmp(device->tBuffCOdol.deviceAddress,deviceAddress,8) == 0) 		{a=0.9; b=7.7;	buffers = true;}
	if (memcmp(device->tBuffCOsrodek.deviceAddress,deviceAddress,8) == 0) 	{a=0.935; b=4.7;buffers = true;}
	if (memcmp(device->tBuffCOgora.deviceAddress,deviceAddress,8) == 0) 	{a=0.9; b=6.2;	buffers = true;}
	if (memcmp(device->tBuffCWUdol.deviceAddress,deviceAddress,8) == 0) 	{a=1.005; b=3.5;buffers = true;}
	if (memcmp(device->tBuffCWUsrodek.deviceAddress,deviceAddress,8) == 0) 	{a=0.97; b=3.5;	buffers = true;}
	if (memcmp(device->tBuffCWUgora.deviceAddress,deviceAddress,8) == 0) 	{a=0.99; b=3.5;	buffers = true;}

	if (memcmp(device->tZasilanie.deviceAddress,deviceAddress,8) == 0) 		{a=0.92; b=5;	devices = true;}
	if (memcmp(device->tPowrot.deviceAddress,deviceAddress,8) == 0) 		{a=0.955; b=2.7;devices = true;}
	if (memcmp(device->tDolneZrodlo.deviceAddress,deviceAddress,8) == 0) 	{a=0.1;	b=0;	devices = true;}
	if (memcmp(device->tKominek.deviceAddress,deviceAddress,8) == 0) 		{a=0.97; b=1.65;devices = true;}

	if (memcmp(device->tRozdzielacze.deviceAddress,deviceAddress,8) == 0) 	{a=0.96; b=2.2;	devices = true;}
	if (memcmp(device->tPowrotParter.deviceAddress,deviceAddress,8) == 0) 	{a=0.1;	b=0;	devices = true;}
	if (memcmp(device->tPowrotPietro.deviceAddress,deviceAddress,8) == 0) 	{a=0.1;	b=0;	devices = true;}


	if (buffers) tempC = sensorsBuffers.getTempC(deviceAddress);
	if (devices) tempC = sensorsDevices.getTempC(deviceAddress);

	if ((tempC<5) || (tempC>80)) {
		thermometer->errorCount++;
		//zapisz liczbe maksymalna zlych odczytow z rzedu
		if (thermometer->maxErrorCount < thermometer->errorCount) thermometer->maxErrorCount = thermometer->errorCount;
		//po trzech probach nie udanych wysli 100.00 stC
		if (thermometer->errorCount > 3) thermometer->isTemp = 100.00;
	}
	else {
		//przelicz temperature wedlug krzywej
		tempC = a * tempC + b;						// Temperature compensation
		//zerowanie liczby bledow
		thermometer->errorCount = 0;

		thermometer->isTemp = tempC;
	}
}


