idf_component_register(SRCS "hello_word_main.c" "esp32_rtc_main.cpp" "rtc_module.cpp"
                       PRIV_REQUIRES spi_flash
                       INCLUDE_DIRS "")
