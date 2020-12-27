#include "Status.h"

#include "Arduino.h"
#include "Basic.h"

String main1;
String main2;
String circuit;
String ret;
String mainT;
String buffT;
String buffSet;
String isT;
String reqT;
String dt ;
String statusWIFI;
String logs[30] = "0";

unsigned long lastMillis = 0;

void setStatus(Device device, DateTime dateTime) {
	main1 = "GLOWNE    ";
	if (device.heatSourceActive == 1) main1 += " PCi";
	if (device.heatSourceActive == 2) main1 += " Bufor";
	if (device.heatSourceActive == 3) main1 += " Kominek";
	main1 += "	inH["; main1 += device.pump_InHouse; main1 += "]";

	main1 +="  uG["; main1 += device.pump_UnderGround; main1 += "]";
	main1 += " IItar["; main1 += device.cheapTariffOnly; main1 += "]";
	main1 += " grzanie["; main1 += device.heatingActivated; main1 += "]";
	main1 += " reqPCi["; main1 += device.reqHeatPumpOn; main1 += "]";
	main1 += " antyLegio["; main1 += device.antyLegionellia; main1 += "]";
	main1 += " alarmHeatingPumpTemp["; main1+= device.heatPumpAlarmTemperature; main1 += "]";

	if (device.valve_3way == 1) main2 = "			3way[CO]";
	if (device.valve_3way == 2) main2 = "			3way[CWU]";
	main2 += "	ByPass["; main2 += device.valve_bypass; main2 += "] Real["; main2 += device.valve_bypass_realPos; main2 += "] Reach[";
	main2 += device.valve_bypass_reachPos;  main2 += "] lastCorrectReadTempZrodla[";
	main2 += "] Flag["; main2 += device.valve_bypass_moveFlag;  main2 += "]";

	circuit = "CIRCUIT   ";
	if (device.circuitOnAmount != 0) { circuit +=" cirOnAmo["; circuit +=device.circuitOnAmount; circuit +="]";}
	for (int i=0; i<ZONE_QUANTITY; i++)
		if (device.zone[i].circuit != 0) { circuit +=" Zone"; circuit +=i; circuit +="["; circuit +=device.zone[i].circuit; circuit +="]";}
	circuit +=" Amount["; circuit +=device.circuitOnAmount; circuit +="]";

 	ret = "POWROT T>   ";
	if ((int)device.tPowrotParter.isTemp != -127) { ret +=" TpowrotParter["; ret +=device.tPowrotParter.isTemp; ret +="]["; ret += device.tPowrotParter.maxErrorCount; ret +="]";}
	if ((int)device.tPowrotPietro.isTemp != -127) { ret +=" TpowrotPietro["; ret +=device.tPowrotPietro.isTemp; ret +="]["; ret += device.tPowrotPietro.maxErrorCount; ret +="]";}

	mainT = "TEMP OBIEGU       ";
	if ((int)device.tZasilanie.isTemp != -127) { mainT +=" TZasilanie["; mainT +=device.tZasilanie.isTemp; mainT +="]["; mainT += device.tZasilanie.maxErrorCount; mainT +="]";}
	if ((int)device.tPowrot.isTemp != -127) { mainT +=" Tpowrot["; mainT +=device.tPowrot.isTemp; mainT +="]["; mainT += device.tPowrot.maxErrorCount; mainT +="]";}
	if ((int)device.tKominek.isTemp != -127) { mainT +=" TKominek["; mainT +=device.tKominek.isTemp; mainT +="]["; mainT += device.tKominek.maxErrorCount; mainT +="]";}
	if ((int)device.tRozdzielacze.isTemp != -127) { mainT +=" Trozdz["; mainT +=device.tRozdzielacze.isTemp; mainT +="]["; mainT += device.tRozdzielacze.maxErrorCount; mainT +="]";}

	buffT = "BUFORY    ";
	if ((int)device.tBuffCOdol.isTemp != -127) { buffT +=" TCOd["; buffT +=device.tBuffCOdol.isTemp; buffT +="]["; buffT += device.tBuffCOdol.maxErrorCount; buffT +="]";}
	if ((int)device.tBuffCOsrodek.isTemp != -127) { buffT +=" TCOs["; buffT +=device.tBuffCOsrodek.isTemp; buffT +="]["; buffT += device.tBuffCOsrodek.maxErrorCount; buffT +="]";}
	if ((int)device.tBuffCOgora.isTemp != -127) { buffT +=" TCOg["; buffT +=device.tBuffCOgora.isTemp; buffT +="]["; buffT += device.tBuffCOgora.maxErrorCount; buffT +="]";}
	if ((int)device.tBuffCWUdol.isTemp != -127) { buffT +=" TCWUd["; buffT +=device.tBuffCWUdol.isTemp; buffT +="]["; buffT += device.tBuffCWUdol.maxErrorCount; buffT +="]";}
	if ((int)device.tBuffCWUsrodek.isTemp != -127) { buffT +=" TCWUs["; buffT +=device.tBuffCWUsrodek.isTemp; buffT +="]["; buffT += device.tBuffCWUsrodek.maxErrorCount; buffT +="]";}
	if ((int)device.tBuffCWUgora.isTemp != -127) { buffT +=" TCWUg["; buffT +=device.tBuffCWUgora.isTemp; buffT +="]["; buffT += device.tBuffCWUgora.maxErrorCount; buffT +="]";}

	buffSet = "BUFOR SET ";
	if (device.reqTempBuforCO != -127) {  buffSet +=" reqCO["; buffSet +=device.reqTempBuforCO; buffSet +="]";}
	if (device.reqTempBuforCWU != -127) { buffSet +=" reqCWU["; buffSet +=device.reqTempBuforCWU; buffSet +="]";}

	isT = "IS  Temp  ";
	if (device.zone[0].isTemp != 0) { isT +=" zone0["; isT +=device.zone[0].isTemp; isT +="]";}
	if (device.zone[1].isTemp != 0) { isT +=" zone1["; isT +=device.zone[1].isTemp; isT +="]";}
	if (device.zone[2].isTemp != 0) { isT +=" zone2["; isT +=device.zone[2].isTemp; isT +="]";}
	if (device.zone[3].isTemp != 0) { isT +=" zone3["; isT +=device.zone[3].isTemp; isT +="]";}
	if (device.zone[4].isTemp != 0) { isT +=" zone4["; isT +=device.zone[4].isTemp; isT +="]";}
	if (device.zone[5].isTemp != 0) { isT +=" zone5["; isT +=device.zone[5].isTemp; isT +="]";}
	if (device.zone[6].isTemp != 0) { isT +=" zone6["; isT +=device.zone[6].isTemp; isT +="]";}

	reqT = "REQ Temp  ";
	if (device.zone[0].reqTemp != 0) { reqT +=" zone0["; reqT +=device.zone[0].reqTemp; reqT +="]";}
	if (device.zone[1].reqTemp != 0) { reqT +=" zone1["; reqT +=device.zone[1].reqTemp; reqT +="]";}
	if (device.zone[2].reqTemp != 0) { reqT +=" zone2["; reqT +=device.zone[2].reqTemp; reqT +="]";}
	if (device.zone[3].reqTemp != 0) { reqT +=" zone3["; reqT +=device.zone[3].reqTemp; reqT +="]";}
	if (device.zone[4].reqTemp != 0) { reqT +=" zone4["; reqT +=device.zone[4].reqTemp; reqT +="]";}
	if (device.zone[5].reqTemp != 0) { reqT +=" zone5["; reqT +=device.zone[5].reqTemp; reqT +="]";}
	if (device.zone[6].reqTemp != 0) { reqT +=" zone6["; reqT +=device.zone[6].reqTemp; reqT +="]";}

	dt = "Data[";
	dt +=dateTime.day; dt +="."; dt +=dateTime.month; dt +="."; dt +=dateTime.year; dt +=" dT("; dt +=dateTime.weekDay; dt +=")] Czas[";
	dt +=dateTime.hour;dt +=":"; dt +=dateTime.minute; dt +=":"; dt +=dateTime.second; dt +="]";

	statusWIFI = "IP ["; statusWIFI +=device.diagnose.ip[0]; statusWIFI +=".";
	statusWIFI +=device.diagnose.ip[1]; statusWIFI +="."; statusWIFI +=device.diagnose.ip[2];
	statusWIFI +="."; statusWIFI +=device.diagnose.ip[3]; statusWIFI +="]";
	if (device.diagnose.wifiConnected) statusWIFI +=" Connected[true] Connect_interrupted["; else statusWIFI +=" Connected[false] Connect_interrupted[";
	statusWIFI +=device.diagnose.wifiConnectionInterrupt; statusWIFI +="]";

}

void addLog(String log, DateTime dateTime) {
	for (int i=28; i>=0; i--)
		logs[i+1] = logs[i];

	logs[0] = dateTime.year;
	logs[0] += ".";
	logs[0] += dateTime.month;
	logs[0] += ".";
	logs[0] += dateTime.day;
	logs[0] += " ";
	logs[0] += dateTime.hour;
	logs[0] += ":";
	logs[0] += dateTime.minute;
	logs[0] += ":";
	logs[0] += dateTime.second;
	logs[0] += " ";
	logs[0] += log;

	Serial.println(logs[0]);
}

void printStatus(Device device, DateTime dateTime) {
	if (Sleep(&lastMillis, 10)) return;
	setStatus(device, dateTime);

	Serial.print("\n\n");
	Serial.print(main1+"\n");
	Serial.print(main2+"\n");
	Serial.print(circuit+"\n");
	Serial.print(ret+"\n");
	Serial.print(mainT+"\n");
	Serial.print(buffT+"\n");
	Serial.print(buffSet+"\n");
	Serial.print(isT+"\n");
	Serial.print(reqT+"\n");
	Serial.print(dt+"\n");
	Serial.print(statusWIFI+"\n");
	for (int i=0; i<20; i++)
		Serial.print(logs[i]+"\n");
}

String getHTMLStatus() {
	String status;
	status = "<h1>ESP32 Diagnose</h1>";
	status += "<h2> Typ ";
	status += DEVICE_TYP;
	status += "</h2>\n";
	status += "<p>";
	status += main1;
	status += "</p>";
	status += "<p>";
	status += main2;
	status += "</p>";
	status += "<p>";
	status += circuit;
	status += "</p>";
	status += "<p>";
	status += ret;
	status += "</p>";
	status += "<p>";
	status += mainT;
	status += "</p>";
	status += "<p>";
	status += buffT;
	status += "</p>";
	status += "<p>";
	status += buffSet;
	status += "</p>";
	status += "<p>";
	status += isT;
	status += "</p>";
	status += "<p>";
	status += reqT;
	status += "</p>";
	status += "<p>";
	status += dt;
	status += "</p>";
	status += "<p>";
	status += statusWIFI;
	status += "</p>";
	for (int i=0; i<30; i++) {
		status += "<p>";
		status += logs[i];
		status += "\n";
		status += "</p>";
	}
	return status;
}


