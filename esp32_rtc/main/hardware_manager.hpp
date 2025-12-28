#pragma once

#include <string>
#include "esp_vfs_fat.h"
#include "driver/sdspi_host.h"
#include "sdmmc_cmd.h"

class HardwareManager {
public:
    static HardwareManager& getInstance() {
        static HardwareManager instance;
        return instance;
    }

    esp_err_t initSd(int mosi, int miso, int clk, int cs, const std::string& mountPoint = "/sdcard");
    void deinitSd();

    bool isSdReady() const { return _sdReady; }
    std::string getMountPoint() const { return _mountPoint; }

private:
    HardwareManager() = default;
    
    bool _sdReady = false;
    std::string _mountPoint;
    sdmmc_card_t* _card = nullptr;
    const spi_host_device_t _hostId = SPI2_HOST;
    static constexpr const char* TAG = "hardware_manager";
};