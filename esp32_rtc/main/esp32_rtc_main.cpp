/*
 * ESP32 RTC
 * Tongo
 */

#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>

#include "rtc_module.hpp"
#include "http_server.hpp"
#include "hardware_manager.hpp"
#include "data_logger.hpp"

static constexpr const char* TAG = "main";

// Configurazione PIN SdCard
#define SD_PIN_MOSI 23
#define SD_PIN_MISO 19
#define SD_PIN_CLK  18
#define SD_PIN_CS   5
#define SD_MOUNT_POINT "/sdcard"

extern "C" void app_main(void)
{
    printf("Hello RTC!\n");

    HardwareManager& hwManager = HardwareManager::getInstance();
    if (hwManager.initSd(SD_PIN_MOSI, SD_PIN_MISO, SD_PIN_CLK, SD_PIN_CS, SD_MOUNT_POINT) != ESP_OK) {
        ESP_LOGE(TAG, "Hardware non pronto. Sistema arrestato.");
        return;
    }

    RtcModule* rtc = new RtcModule();
    rtc->setEspTimestamp();

    DataLogger* dataLogger = new DataLogger();
    std::vector<std::string> channels = {"time", "velocity kmh", "rpm", "temp"};
    dataLogger->initLogFile(channels);

    HttpServer* httpServer = new HttpServer(rtc);
    httpServer->startWebserver();
}
