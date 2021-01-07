/*
 * Status.cpp
 *
 *  Created on: 23 gru 2020
 *      Author: spoma
 */

#include "Status.h"

String mainText;
String dt;
String statusWIFI;
String logs[30];
unsigned long lastMillis = 0;

Status::Status(boolean start) {
}

Status::~Status() {
}

void setStatus(String text) {
	mainText=text;

	DateTime dateTime = getDateTime();
	dt = "Data[";
	dt +=dateTime.day; dt +="."; dt +=dateTime.month; dt +="."; dt +=dateTime.year; dt +=" dT("; dt +=dateTime.weekDay; dt +=")] Czas[";
	dt +=dateTime.hour;dt +=":"; dt +=dateTime.minute; dt +=":"; dt +=dateTime.second; dt +="]";

	Diagnose diagnose = getDiagnose();
	statusWIFI = "IP ["; statusWIFI +=diagnose.ip[0]; statusWIFI +=".";
	statusWIFI +=diagnose.ip[1]; statusWIFI +="."; statusWIFI +=diagnose.ip[2];
	statusWIFI +="."; statusWIFI +=diagnose.ip[3]; statusWIFI +="]";
	if (diagnose.wifiConnected) statusWIFI +=" Connected[true] Connect_interrupted["; else statusWIFI +=" Connected[false] Connect_interrupted[";
	statusWIFI +=diagnose.wifiConnectionInterrupt; statusWIFI +="]";
}

void Status::addLog(String log) {
	if ((String)log[0] == log) return;
	for (int i=28; i>=0; i--)
		logs[i+1] = logs[i];
	logs[0]= getDateTime().year;
	logs[0] += ".";
	logs[0] += getDateTime().month;
	logs[0] += ".";
	logs[0] += getDateTime().day;
	logs[0] += " ";
	logs[0] += getDateTime().hour;
	logs[0] += ":";
	logs[0] += getDateTime().minute;
	logs[0] += ":";
	logs[0] += getDateTime().second;
	logs[0] += " ";
	logs[0] += log;
}

void Status::printStatus(int delay) {
	if (sleep(&lastMillis, delay)) return;
	Serial.print("\n\n");
	Serial.print(mainText+"\n");
	Serial.print(dt+"\n");
	Serial.print(statusWIFI+"\n");
	for (int i=0; i<20; i++)
		if (logs[i] != "")
			Serial.print(logs[i]+"\n");
}

String getHTMLStatus() {
	String status;
	status = "<h1>ESP32 Diagnose</h1>";
	status += "<h2> Type ";
	status += getModuleType();
	status += "</h2>\n";
	status += "<p>";
	for (int i=0; i<mainText.length(); i++)
		if (mainText[i]=='\n')
				status +="</p><p>";
		else if (mainText[i]=='\t')
			status +="&emsp;";
		else
			status += mainText[i];
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


