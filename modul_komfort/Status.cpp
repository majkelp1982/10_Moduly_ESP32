#include "Arduino.h"
#include "Basic.h"
#include "Status.h"

String main1;
String dt ;
String statusWIFI;
String logs[30] = "0";

unsigned long lastMillis = 0;

//TMP
void setStatus(Device device, DateTime dateTime) {
	main1 = "MAIN\n";

	for (int i=0; i<ZONE_QUANTITY; i++) {
		main1 +="Zone[";
		main1 +=i;
		main1 +="]	hum=";
		main1 +=(int)device.zone[i].humidity;
		main1 +="	isTemp=";
		main1 +=(double)device.zone[i].isTemp;
		main1 +="	reqTemp=";
		main1 +=(double)device.zone[i].reqTemp;
		main1 +="	ErrorMAX[";
		main1 +=device.zone[i].maxErrorCount;
		main1 +="]";
		main1 +="\n";
	}

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

