/*
 * ESP32 RTC
 * Tongo
 */

#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#include "rtc_module.hpp"
#include "http_server.hpp"
#include "hardware_manager.hpp"
#include "data_logger.hpp"
#include "data_types.hpp"

static constexpr const char* TAG = "main";

// Configurazione PIN SdCard
#define SD_PIN_MOSI 23
#define SD_PIN_MISO 19
#define SD_PIN_CLK  18
#define SD_PIN_CS   5
#define SD_MOUNT_POINT "/sdcard"

QueueHandle_t dataQueue;
TaskHandle_t loggerTaskHandle = NULL;

// Task che scrive su SD
void sdLoggerTask(void* pvParameters) {
    DataLogger* logger = (DataLogger*)pvParameters;
    DataPackage receivedData;
    char buffer[128];

    ESP_LOGI("TASK", "Task Logger avviato");

    while (true) {
        // Resta in attesa di dati dalla coda (tempo massimo di attesa: infinito)
        if (xQueueReceive(dataQueue, &receivedData, portMAX_DELAY) == pdPASS) {
            
            // Formattiamo la stringa solo qui, nel task a bassa priorità
            snprintf(buffer, sizeof(buffer), "%.3f %.2f %d %.1f\n", 
                     receivedData.timestamp / 1000.0f, 
                     receivedData.speed, 
                     receivedData.rpm, 
                     receivedData.temp);
            
            logger->writeRow(buffer);

            // Opzionale: flush ogni N messaggi ricevuti per sicurezza
            static int count = 0;
            if (++count >= 100) { 
                logger->flushLog(); 
                count = 0; 
            }
        }
    }
}

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

    dataQueue = xQueueCreate(500, sizeof(DataPackage));
    dataLogger->openLog();

    // Lanciamo il task
    xTaskCreate(sdLoggerTask, "sdLoggerTask", 4096, dataLogger, 3, &loggerTaskHandle);

    ESP_LOGI(TAG, "Logging avviato per 30 secondi...");

    uint32_t startMs = esp_log_timestamp();
    
    ESP_LOGI(TAG, "Logging avviato (Target: 3000 campioni)...");
    int droppedSamples = 0;
    int sentSamples = 0;

    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(10); // 10ms

    // Ciclo di 30 secondi
    while ((esp_log_timestamp() - startMs) < 30000) {
        DataPackage pkg;
        pkg.timestamp = esp_log_timestamp();
        pkg.speed = 100.0f;
        pkg.temp = rtc->getTemperature();
        pkg.rpm = 7000;

        if (xQueueSend(dataQueue, &pkg, pdMS_TO_TICKS(2))) {
            sentSamples++;
        } else {
            droppedSamples++;
            // Logghiamo ogni volta che perdiamo un pacchetto (attenzione: i log possono essere lenti)
            ESP_LOGW(TAG, "Coda piena! Pacchetto scartato. Totale persi: %d", droppedSamples);
        }
        vTaskDelayUntil(&xLastWakeTime, xFrequency); // 100Hz
    }

    // --- PROCEDURA DI CHIUSURA ---
    ESP_LOGI(TAG, "Tempo scaduto. Arresto in corso...");
    ESP_LOGI(TAG, "Fine 30s. Inviati con successo: %d, Persi: %d", sentSamples, droppedSamples);

    // 1. Eliminiamo il task per assicurarci che non acceda più al logger o alla SD
    if (loggerTaskHandle != NULL) {
        vTaskDelete(loggerTaskHandle);
        loggerTaskHandle = NULL;
    }

    DataPackage remainingPkg;
    char buffer[128];
    int recovered = 0;

    // uxQueueMessagesWaiting ci dice quanti pacchetti ci sono ancora
    while (xQueueReceive(dataQueue, &remainingPkg, 0) == pdPASS) {
        int len = snprintf(buffer, sizeof(buffer), "%.3f %.2f %d %.1f\n", 
                           remainingPkg.timestamp / 1000.0f, 
                           remainingPkg.speed, 
                           remainingPkg.rpm,
                           remainingPkg.temp);
        dataLogger->writeRow(std::string(buffer, len));
        recovered++;
    }

    ESP_LOGI(TAG, "Recuperati dallo svuotamento finale: %d", recovered);
    ESP_LOGI(TAG, "Totale finale su SD: %d", (sentSamples - recovered) + recovered); // Dovrebbe essere 3000

    dataLogger->flushLog();
    dataLogger->closeLog(); // Chiude il file in sicurezza
    delete dataLogger;      // Libera la memoria allocata con new

    ESP_LOGI(TAG, "Sessione salvata e memoria liberata.");

    // Il main resta attivo ma non fa più nulla
    while(1) { vTaskDelay(1000); }

    /*
    HttpServer* httpServer = new HttpServer(rtc);
    httpServer->startWebserver();
    */
}
