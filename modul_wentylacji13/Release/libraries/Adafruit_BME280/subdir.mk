################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
D:\Desktop\Eclipse-workspace\Adafruit_BME280\Adafruit_BME280.cpp 

LINK_OBJ += \
.\libraries\Adafruit_BME280\Adafruit_BME280.cpp.o 

CPP_DEPS += \
.\libraries\Adafruit_BME280\Adafruit_BME280.cpp.d 


# Each subdirectory must supply rules for building sources it contributes
libraries\Adafruit_BME280\Adafruit_BME280.cpp.o: D:\Desktop\Eclipse-workspace\Adafruit_BME280\Adafruit_BME280.cpp
	@echo 'Building file: $<'
	@echo 'Starting C++ compile'
	"C:\Sloeber-ESP\arduinoPlugin\packages\esp32\tools\xtensa-esp32-elf-gcc\1.22.0-80-g6c4433a-5.2.0/bin/xtensa-esp32-elf-g++" -DESP_PLATFORM -DMBEDTLS_CONFIG_FILE="mbedtls/esp_config.h" -DHAVE_CONFIG_H "-IC:\Sloeber-ESP\/arduinoPlugin/packages/esp32/hardware/esp32/1.0.0/tools/sdk/include/config" "-IC:\Sloeber-ESP\/arduinoPlugin/packages/esp32/hardware/esp32/1.0.0/tools/sdk/include/bluedroid" "-IC:\Sloeber-ESP\/arduinoPlugin/packages/esp32/hardware/esp32/1.0.0/tools/sdk/include/bluedroid/api" "-IC:\Sloeber-ESP\/arduinoPlugin/packages/esp32/hardware/esp32/1.0.0/tools/sdk/include/app_trace" "-IC:\Sloeber-ESP\/arduinoPlugin/packages/esp32/hardware/esp32/1.0.0/tools/sdk/include/app_update" "-IC:\Sloeber-ESP\/arduinoPlugin/packages/esp32/hardware/esp32/1.0.0/tools/sdk/include/bootloader_support" "-IC:\Sloeber-ESP\/arduinoPlugin/packages/esp32/hardware/esp32/1.0.0/tools/sdk/include/bt" "-IC:\Sloeber-ESP\/arduinoPlugin/packages/esp32/hardware/esp32/1.0.0/tools/sdk/include/driver" "-IC:\Sloeber-ESP\/arduinoPlugin/packages/esp32/hardware/esp32/1.0.0/tools/sdk/include/esp32" "-IC:\Sloeber-ESP\/arduinoPlugin/packages/esp32/hardware/esp32/1.0.0/tools/sdk/include/esp_adc_cal" "-IC:\Sloeber-ESP\/arduinoPlugin/packages/esp32/hardware/esp32/1.0.0/tools/sdk/include/esp_http_client" "-IC:\Sloeber-ESP\/arduinoPlugin/packages/esp32/hardware/esp32/1.0.0/tools/sdk/include/esp-tls" "-IC:\Sloeber-ESP\/arduinoPlugin/packages/esp32/hardware/esp32/1.0.0/tools/sdk/include/ethernet" "-IC:\Sloeber-ESP\/arduinoPlugin/packages/esp32/hardware/esp32/1.0.0/tools/sdk/include/fatfs" "-IC:\Sloeber-ESP\/arduinoPlugin/packages/esp32/hardware/esp32/1.0.0/tools/sdk/include/freertos" "-IC:\Sloeber-ESP\/arduinoPlugin/packages/esp32/hardware/esp32/1.0.0/tools/sdk/include/heap" "-IC:\Sloeber-ESP\/arduinoPlugin/packages/esp32/hardware/esp32/1.0.0/tools/sdk/include/jsmn" "-IC:\Sloeber-ESP\/arduinoPlugin/packages/esp32/hardware/esp32/1.0.0/tools/sdk/include/log" "-IC:\Sloeber-ESP\/arduinoPlugin/packages/esp32/hardware/esp32/1.0.0/tools/sdk/include/mdns" "-IC:\Sloeber-ESP\/arduinoPlugin/packages/esp32/hardware/esp32/1.0.0/tools/sdk/include/mbedtls" "-IC:\Sloeber-ESP\/arduinoPlugin/packages/esp32/hardware/esp32/1.0.0/tools/sdk/include/mbedtls_port" "-IC:\Sloeber-ESP\/arduinoPlugin/packages/esp32/hardware/esp32/1.0.0/tools/sdk/include/newlib" "-IC:\Sloeber-ESP\/arduinoPlugin/packages/esp32/hardware/esp32/1.0.0/tools/sdk/include/nvs_flash" "-IC:\Sloeber-ESP\/arduinoPlugin/packages/esp32/hardware/esp32/1.0.0/tools/sdk/include/openssl" "-IC:\Sloeber-ESP\/arduinoPlugin/packages/esp32/hardware/esp32/1.0.0/tools/sdk/include/spi_flash" "-IC:\Sloeber-ESP\/arduinoPlugin/packages/esp32/hardware/esp32/1.0.0/tools/sdk/include/sdmmc" "-IC:\Sloeber-ESP\/arduinoPlugin/packages/esp32/hardware/esp32/1.0.0/tools/sdk/include/smartconfig_ack" "-IC:\Sloeber-ESP\/arduinoPlugin/packages/esp32/hardware/esp32/1.0.0/tools/sdk/include/spiffs" "-IC:\Sloeber-ESP\/arduinoPlugin/packages/esp32/hardware/esp32/1.0.0/tools/sdk/include/tcpip_adapter" "-IC:\Sloeber-ESP\/arduinoPlugin/packages/esp32/hardware/esp32/1.0.0/tools/sdk/include/ulp" "-IC:\Sloeber-ESP\/arduinoPlugin/packages/esp32/hardware/esp32/1.0.0/tools/sdk/include/vfs" "-IC:\Sloeber-ESP\/arduinoPlugin/packages/esp32/hardware/esp32/1.0.0/tools/sdk/include/wear_levelling" "-IC:\Sloeber-ESP\/arduinoPlugin/packages/esp32/hardware/esp32/1.0.0/tools/sdk/include/xtensa-debug-module" "-IC:\Sloeber-ESP\/arduinoPlugin/packages/esp32/hardware/esp32/1.0.0/tools/sdk/include/coap" "-IC:\Sloeber-ESP\/arduinoPlugin/packages/esp32/hardware/esp32/1.0.0/tools/sdk/include/console" "-IC:\Sloeber-ESP\/arduinoPlugin/packages/esp32/hardware/esp32/1.0.0/tools/sdk/include/expat" "-IC:\Sloeber-ESP\/arduinoPlugin/packages/esp32/hardware/esp32/1.0.0/tools/sdk/include/json" "-IC:\Sloeber-ESP\/arduinoPlugin/packages/esp32/hardware/esp32/1.0.0/tools/sdk/include/lwip" "-IC:\Sloeber-ESP\/arduinoPlugin/packages/esp32/hardware/esp32/1.0.0/tools/sdk/include/newlib" "-IC:\Sloeber-ESP\/arduinoPlugin/packages/esp32/hardware/esp32/1.0.0/tools/sdk/include/nghttp" "-IC:\Sloeber-ESP\/arduinoPlugin/packages/esp32/hardware/esp32/1.0.0/tools/sdk/include/soc" "-IC:\Sloeber-ESP\/arduinoPlugin/packages/esp32/hardware/esp32/1.0.0/tools/sdk/include/wpa_supplicant" -std=gnu++11 -fno-exceptions -Os -g3 -Wpointer-arith -fexceptions -fstack-protector -ffunction-sections -fdata-sections -fstrict-volatile-bitfields -mlongcalls -nostdlib -Wall -Werror=all -Wextra -Wno-error=unused-function -Wno-error=unused-but-set-variable -Wno-error=unused-variable -Wno-error=deprecated-declarations -Wno-unused-parameter -Wno-sign-compare -fno-rtti -MMD -c -DF_CPU=240000000L -DARDUINO=10802 -DARDUINO_NodeMCU_32S -DARDUINO_ARCH_ESP32 -DARDUINO_BOARD="NodeMCU_32S" -DARDUINO_VARIANT="nodemcu-32s"  -DESP32 -DCORE_DEBUG_LEVEL=0    -I"C:\Sloeber-ESP\arduinoPlugin\packages\esp32\hardware\esp32\1.0.0\cores\esp32" -I"C:\Sloeber-ESP\arduinoPlugin\packages\esp32\hardware\esp32\1.0.0\variants\nodemcu-32s" -I"D:\Desktop\Eclipse-workspace\Adafruit_BME280" -I"C:\Google_Drive\Dom\SmartHouse\10_Moduly-ESP32\Module_LIBS\Basic" -I"C:\Google_Drive\Dom\SmartHouse\10_Moduly-ESP32\Module_LIBS\OTA" -I"C:\Sloeber-ESP\arduinoPlugin\libraries\Adafruit_Unified_Sensor\1.0.2" -I"C:\Sloeber-ESP\arduinoPlugin\packages\esp32\hardware\esp32\1.0.0\libraries\ESPmDNS\src" -I"C:\Sloeber-ESP\arduinoPlugin\packages\esp32\hardware\esp32\1.0.0\libraries\SPI\src" -I"C:\Google_Drive\Dom\SmartHouse\10_Moduly-ESP32\Module_LIBS\Status" -I"C:\Sloeber-ESP\arduinoPlugin\packages\esp32\hardware\esp32\1.0.0\libraries\Update\src" -I"C:\Sloeber-ESP\arduinoPlugin\packages\esp32\hardware\esp32\1.0.0\libraries\WebServer\src" -I"C:\Sloeber-ESP\arduinoPlugin\packages\esp32\hardware\esp32\1.0.0\libraries\WiFi\src" -I"C:\Sloeber-ESP\arduinoPlugin\packages\esp32\hardware\esp32\1.0.0\libraries\Wire\src" -I"C:\Sloeber-ESP\arduinoPlugin\packages\esp32\hardware\esp32\1.0.0\libraries\FS\src" -I"C:\Google_Drive\Dom\SmartHouse\10_Moduly-ESP32\Module_LIBS\Communication" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -D__IN_ECLIPSE__=1 -x c++ "$<"  -o  "$@"
	@echo 'Finished building: $<'
	@echo ' '


