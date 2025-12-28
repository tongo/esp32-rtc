idf_component_register(SRCS "esp32_rtc_main.cpp" "rtc_module.cpp" "http_server.cpp" "hardware_manager.cpp" "data_logger.cpp"
                       PRIV_REQUIRES spi_flash esp_wifi esp_http_server nvs_flash fatfs sdmmc driver
                       INCLUDE_DIRS "")
