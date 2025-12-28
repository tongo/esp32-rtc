#pragma once

#include <string>
#include <vector>
#include <cstdio>
#include <ctime>
#include "esp_err.h"


class DataLogger {
public:
    DataLogger();
    ~DataLogger();

    esp_err_t initLogFile(const std::vector<std::string>& channels);
    esp_err_t openLog();
    esp_err_t writeRow(const std::string& data);
    void flushLog();
    void closeLog();

private:
    std::string _logPath;
    FILE* _log = nullptr;
    static constexpr const char* TAG = "data_logger";

    std::string createLogFolder(std::string basePath, time_t timestamp);
    esp_err_t createLogFile(const std::string fullPath);
    esp_err_t initVboHeader(time_t timestamp, const std::vector<std::string>& channels);
};