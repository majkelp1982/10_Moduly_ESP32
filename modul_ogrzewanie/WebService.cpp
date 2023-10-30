#include "WebService.h"
#include "Module.h"
#include <ArduinoJson.h>

WebServer server2(9090);
StaticJsonDocument<2500> doc;
String jsonPumpActionDeserialization(String json);
String jsonTempDeserialization(String json);
void webServerStart();
void webServerSetup();

void webService_setup() {
	Serial.println("webService_setup");
    webServerStart();
    webServerSetup();
}

void webServerStart() {
  	  /*use mdns for host name resolution*/
	  if (!MDNS.begin("esp32")) { //http://esp32.local
	    Serial.println("Error setting up MDNS responder!");
	    while (1) {
	      delay(1000);
	    }
	  }
	  Serial.println("mDNS responder started");
}

//Endpoints
// /   					GET		- login page
// /serverIndex 		GET		- upload page
// /update				GET		- handlin firmware OTA file
// /configuration		POST	- save configuration
// /action				POST	- handling pins/sensors action
// /params				GET		- retvieve macAddress, connection interruptions, etc.
// /unlockType			POST	- To change modul type, type need to be unlocked

void webServerSetup() {
	  //Service
	   server2.on("/action", HTTP_POST, []() {
		   Serial.println("Enter action endpoint");
		if (server2.hasArg("plain") == false) {
			server2.send(404, "text/plain", "JSON NOT FOUND");
			return;
		}
		String resp = jsonPumpActionDeserialization(server2.arg("plain"));
	    if (resp=="OK") server2.send(200, "text/plain", resp);
	    else server2.send(400, "text/plain", "FAULTY COMMAND");

	   });
	   server2.on("/comfort", HTTP_POST, []() {
		   Serial.println("Enter comfort endpoint");
		if (server2.hasArg("plain") == false) {
			server2.send(404, "text/plain", "JSON NOT FOUND");
			return;
		}
		String resp = jsonTempDeserialization(server2.arg("plain"));
	    if (resp=="OK") server2.send(200, "text/plain", resp);
	    else server2.send(400, "text/plain", "FAULTY COMMAND");

	   });
	  server2.begin();
}

void webService_run() {
    server2.handleClient();
}

String jsonPumpActionDeserialization(String json) {
	  doc.clear();
	  doc.garbageCollect();
	  DeserializationError error = deserializeJson(doc, json);
	  if (error) {
	    Serial.print(F("deserializeJson() failed: "));
	    Serial.println(error.f_str());
	    return "Deserialization failed";
	  }
	  String reqColdWater = doc["requiredColdWater"];
	  String reqWarmWater = doc["requiredWarmWater"];

	  Serial.printf("\nreqColdWater: %s, reqWarmWater:%s", reqColdWater.c_str(), reqWarmWater.c_str());
	  if (reqColdWater.startsWith("t") || reqColdWater.startsWith("T")) {
		  setReqColdWater();
	  }
	  if (reqWarmWater.startsWith("t") || reqWarmWater.startsWith("T")) {
		  setReqWarmWater();
	  }
	  return "OK";
}

String jsonTempDeserialization(String json) {
	  doc.clear();
	  doc.garbageCollect();
	  DeserializationError error = deserializeJson(doc, json);
	  if (error) {
	    Serial.print(F("deserializeJson() failed: "));
	    Serial.println(error.f_str());
	    return "Deserialization failed";
	  }
	  int zoneNumber = doc["zoneNumber"];
	  float temperature = doc["temperature"];
	  float reqTemperature = doc["reqTemperature"];
	  setComfortParams(zoneNumber, temperature, reqTemperature);

	  return "OK";
}

