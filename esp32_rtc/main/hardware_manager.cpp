#include "hardware_manager.hpp"
#include "esp_log.h"

esp_err_t HardwareManager::initSd(int mosi, int miso, int clk, int cs, const std::string& mountPoint) {
    if (_sdReady) return ESP_OK;
    _mountPoint = mountPoint;

    // Configurazione del bus SPI
    spi_bus_config_t busCfg = {
        .mosi_io_num = mosi,
        .miso_io_num = miso,
        .sclk_io_num = clk,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4000,
    };

    esp_err_t ret = spi_bus_initialize(_hostId, &busCfg, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Inizializzazione SPI fallita");
        return ret;
    }

    // Configurazione del montaggio FAT FS
    esp_vfs_fat_sdmmc_mount_config_t mountCfg = {
        .format_if_mount_failed = true,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024
    };

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    host.slot = _hostId;

    sdspi_device_config_t slotCfg = SDSPI_DEVICE_CONFIG_DEFAULT();
    slotCfg.gpio_cs = static_cast<gpio_num_t>(cs);
    slotCfg.host_id = _hostId;

    ret = esp_vfs_fat_sdspi_mount(_mountPoint.c_str(), &host, &slotCfg, &mountCfg, &_card);
    
    if (ret == ESP_OK) {
        _sdReady = true;
        ESP_LOGI(TAG, "SD montata correttamente in %s", _mountPoint.c_str());
    } else {
        ESP_LOGE(TAG, "Errore montaggio SD: %s", esp_err_to_name(ret));
        spi_bus_free(_hostId);
    }

    return ret;
}

void HardwareManager::deinitSd() {
    if (_sdReady) {
        esp_vfs_fat_sdcard_unmount(_mountPoint.c_str(), _card);
        spi_bus_free(_hostId);
        _sdReady = false;
        _card = nullptr;
        ESP_LOGI(TAG, "SD smontata");
    }
}