idf_component_register(SRCS "esp32_rtc_main.cpp" "rtc_module.cpp" "http_server.cpp"
                       PRIV_REQUIRES spi_flash esp_wifi esp_http_server nvs_flash
                       INCLUDE_DIRS "")
