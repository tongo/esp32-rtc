#include "data_logger.hpp"
#include "esp_log.h"
#include <sys/stat.h>
#include "hardware_manager.hpp"

#define LOG_DATA_FOLDER "data"
#define LOG_FILE_NAME "log.vbo"

DataLogger::DataLogger() {}

DataLogger::~DataLogger() {
    closeLog();
}

esp_err_t DataLogger::initLogFile(const std::vector<std::string>& channels) {
    time_t now;
    time(&now);

    HardwareManager& hwManager = HardwareManager::getInstance();
    std::string logFolderPath = createLogFolder(hwManager.getMountPoint(), now);
    if (logFolderPath == "") return ESP_FAIL;
    std::string logFullPath = logFolderPath + "/" + LOG_FILE_NAME;
    if (createLogFile(logFullPath) != ESP_OK) return ESP_FAIL;
    if (initVboHeader(now, channels) != ESP_OK) return ESP_FAIL;
    return ESP_OK;
}

std::string DataLogger::createLogFolder(std::string basePath, time_t timestamp) {
    std::string dataRoot = basePath + "/" + LOG_DATA_FOLDER;
    
    // Controllo esistenza cartella base
    struct stat st;
    if (stat(dataRoot.c_str(), &st) != 0) {
        mkdir(dataRoot.c_str(), 0775);
    }

    struct tm timeinfo;
    localtime_r(&timestamp, &timeinfo);

    char buf[64];
    // Formato cartella: YYYY-MM-DD_HHMMSS
    strftime(buf, sizeof(buf), "%Y-%m-%d_%H%M%S", &timeinfo);
    std::string logFolderPath = dataRoot + "/" + buf;

    if (mkdir(logFolderPath.c_str(), 0775) == 0) {
        ESP_LOGI(TAG, "Cartella sessione creata: %s", logFolderPath.c_str());
        return logFolderPath;
    }
    
    ESP_LOGE(TAG, "Errore creazione cartella [%s]: %d", logFolderPath.c_str(), errno);
    return "";
}

esp_err_t DataLogger::createLogFile(std::string fullPath) {
    _logPath = fullPath;
    FILE* f = fopen(_logPath.c_str(), "w");
    if (f == nullptr) {
        ESP_LOGI(TAG, "Errore creazione log file: %s", fullPath.c_str());
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Log file creato: %s", fullPath.c_str());
    return ESP_OK;
}

esp_err_t DataLogger::initVboHeader(time_t timestamp, const std::vector<std::string>& channels) {
    openLog();

    struct tm timeinfo;
    localtime_r(&timestamp, &timeinfo);

    char dataBuf[64];
    // Formato sicuro per FAT: YYYY-MM-DD_HHMMSS
    strftime(dataBuf, sizeof(dataBuf), "%d/%m/%Y @ %H:%M:%S", &timeinfo);
    fprintf(_log, "File created on %s\n", dataBuf);
    
    // Scrittura Header VBOX
    fprintf(_log, "\n[header]\n");
    for (size_t i = 0; i < channels.size(); ++i) {
        fprintf(_log, "%s\n", channels[i].c_str());
    }
    fprintf(_log, "\n[data]\n");
    
    flushLog();
    closeLog();
    ESP_LOGI(TAG, "Header VBO scritto in %s", _logPath.c_str());
    return ESP_OK;
}

esp_err_t DataLogger::openLog() {
    if (_log) closeLog();
    
    _log = fopen(_logPath.c_str(), "a");
    if (_log) {
        // Ottimizzazione buffer di scrittura (2KB in RAM prima di scrivere fisicamente)
        setvbuf(_log, NULL, _IOFBF, 2048);
        return ESP_OK;
    }
    return ESP_FAIL;
}

esp_err_t DataLogger::writeRow(const std::string& data) {
    if (!_log) return ESP_FAIL;
    size_t written = fwrite(data.c_str(), 1, data.size(), _log);
    return (written == data.size()) ? ESP_OK : ESP_FAIL;
}

void DataLogger::flushLog() {
    if (_log) fflush(_log);
}

void DataLogger::closeLog() {
    if (_log) {
        fclose(_log);
        _log = nullptr;
    }
}