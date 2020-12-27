#include "Basic.h"

void WiFi_init();
void UDPsendStandardFrame(Device *device);
void UDPread(Device *device, Humidity *humidity, Temperature *temperature, DateTime *dateTime);
void UDPsendDiagnoseFrame(Device *device);

