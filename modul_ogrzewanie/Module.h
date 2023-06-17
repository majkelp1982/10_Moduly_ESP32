#include <Arduino.h>
#include <Basic.h>
#include <Status.h>
#include <DallasTemperature.h>
#include <OneWire.h>

#pragma once

//MODULES ID
#define ID_MOD_MAIN							1
#define ID_MOD_COMFORT						10
#define ID_MOD_VENT							13

//PIN DECLARATION
//ONE WIRE
#define ONE_WIRE_BUFFERS					22
#define ONE_WIRE_DEVICES					21

//circuit RELAYS
#define ZONE0								15
#define ZONE1								2
#define ZONE2								4
#define ZONE3								16
#define ZONE4								17
#define ZONE5								5
#define ZONE6								18

//ZONES NAMES
#define ID_SALON							0
#define ID_PRALNIA							1
#define ID_LAZ_DOL							2
#define ID_RODZICE							3
#define ID_NATALIA							4
#define ID_KAROLINA							5
#define ID_LAZ_GORA							6

//RELAYs
//FOR HEATING PUMP ON REQUEST
#define RELAY_HEAT_PUMP_ON					32
//FOR ANTILEGIONELLIA
#define RELAY_ANTILEGIONELLIA				33

//CIRCULATION PUMP
#define PUMP_IN_HOUSE						26
#define PUMP_UNDER_HOUSE					25

//VALVES
#define VALVE_3WAY_CO						13
#define VALVE_3WAY_CWU						23
#define VALVE_BYPASS_OPEN					27
#define VALVE_BYPASS_CLOSE					14

//CONST
#define	ZONE_QUANTITY						7		// quantity of zones

//3 WAY VALVE
#define CO									1
#define CWU									2
#define VALVE_3WAY_RIDE_TIME				50000

//DALLAS 1-wire
#define DELAY_DS18B20_READ					5		// delay between sensors reading [s]

//VALVE
#define DELAY_THERMO_HEADS					180000
#define HEAT_PUMP_BYP_START_POS			 		20					// Pos. from which Bypass start to adjust

// HEAT PUMP
#define HEAT_PUMP_WARNING_TEMPERATURE			53					// optimal temperature on tempZrodelCiepla
#define HEAT_PUMP_HISTERESIS_HI					1					// correction hysteresis
#define HEAT_PUMP_HISTERESIS_LOW				2					// correction hysteresis
#define DELAY_HEAT_PUMP_OUT_TEMP_TO_HIGH 		25000				// if temp to high after this time open a bit bypass
#define DELAY_HEAT_PUMP_OUT_TEMP_TO_LOW 		60000				// if temp to low after this time close a bit bypass
#define HEAT_PUMP_BYP_UPSTEP_WHEN_HI			2					// number of step during this correction
#define HEAT_PUMP_BYP_DOWNSTEP_WHEN_LOW			1					// number of step during this correction
#define HEAT_PUMP_START_POS_AFTER_TEMP_NO_READ	15000				// After this time Bypass set start pos when no correct temperature read
#define DELAY_HEAT_PUMP							30000

//OTHERS
#define TEMP_MIN_ON_DISTRIBUTOR				33
#define TEMP_CO_EMERGENCY_HYSTERESIS		2
#define TEMP_CWU_HYSTERESIS					2
#define TEMP_CO_HYSTERESIS					5

//MODE
#define HEAT_PUMP							1
#define BUFFER_CO							2
#define FIREPLACE							3

//BYPASS VALVE
#define BYPASS_STEP							1000


//OTHERS
#define TEMP_HYSTERESIS						0.3

struct Thermometer {						// Thermometer
	DeviceAddress deviceAddress;
	float isTemp=0;							// measured temperature
	int errorCount = 0;						// number of faulty reading
	int maxErrorCount = 0;
};

//Zone data
struct Zone {
	int errorCount 		=0;					// number of faulty reading
	float isTemp		=0;					// measured temperature
	float reqTemp		=0;					// required temperature
	bool circuit		=false;				// circuit state (true is on)
};

//Vent
struct VentData {
	bool reqColdWater = false;
	unsigned long reqColdWaterMillis;
	bool reqHotWater = false;
	unsigned long reqHotWaterMillis;
};

struct Device {
	byte heatSourceActive 		= 1;			// 1-Heat Pump, 2-BufferCO, 3-Fireplace
	boolean pump_InHouse 		= false;
	boolean pump_UnderGround 	= false;
	boolean reqHeatPumpOn 		= false;
	boolean cheapTariffOnly 	= true;
	boolean heatingActivated 	= false;
	boolean antyLegionellia 	= false;
	byte valve_3way = CWU;					// 1-CO 2-CWU 0-position undefined
	int valve_bypass = 0;					// 0-40 (value 40 = 100% fully open)
	//Bypass
	int valve_bypass_realPos;
	bool valve_bypass_reachPos = false;
	//3-way
	byte valve_3wayLastState = CWU;							// valves
	bool valve_bypass_moveFlag = false;
	Zone zone[ZONE_QUANTITY];

	float reqTempBuforCO = 35;
	float reqTempBuforCWU = 43;
	byte heatPumpAlarmTemperature;

	Thermometer tBuffCOdol;
	Thermometer tBuffCOsrodek;
	Thermometer tBuffCOgora;
	Thermometer tBuffCWUdol;
	Thermometer tBuffCWUsrodek;
	Thermometer tBuffCWUgora;
	Thermometer tZasilanie;
	Thermometer tPowrot;
	Thermometer tDolneZrodlo;
	Thermometer tKominek;
	Thermometer tRozdzielacze;
	Thermometer tPowrotParter;
	Thermometer tPowrotPietro;
	int circuitOnAmount = 0;
};

void module_init();
void module();
void setReqColdWater();
void setReqWarmWater();

