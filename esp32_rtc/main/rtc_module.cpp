#include "rtc_module.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG = "RtcModule";

RtcModule::RtcModule() {
    // Create the i2c_master_bus_config_t struct and assign values.
    i2c_master_bus_config_t i2c_mst_config = {
        .i2c_port = -1,
        .sda_io_num = GPIO_NUM_21,
        .scl_io_num = GPIO_NUM_22,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags {
            .enable_internal_pullup = true,
        }
    };
    i2c_new_master_bus(&i2c_mst_config, &_bus_handle);
    _rtc_handle = ds3231_init(&_bus_handle);
}

RtcModule::~RtcModule() {
}

time_t RtcModule::getInitialTimestamp() {
    struct tm time_tm = {
        .tm_sec = 0,
        .tm_min = 0,
        .tm_hour = 10,
        .tm_mday = 1,
        .tm_mon = 0,
        .tm_year = 2025 - 1900
    };
    return mktime(&time_tm);
}
time_t RtcModule::getBuildTimestamp() {
    struct tm build_tm = {0};
    char month_str[4];
    const char *months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", 
                            "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

    // __DATE__ ha il formato "Mmm dd yyyy" (es. "Dec 20 2025")
    // __TIME__ ha il formato "hh:mm:ss" (es. "11:42:00")
    sscanf(__DATE__, "%s %d %d", month_str, &build_tm.tm_mday, &build_tm.tm_year);
    sscanf(__TIME__, "%d:%d:%d", &build_tm.tm_hour, &build_tm.tm_min, &build_tm.tm_sec);

    // Convertiamo il nome del mese in indice (0-11)
    for (int i = 0; i < 12; i++) {
        if (strcmp(month_str, months[i]) == 0) {
            build_tm.tm_mon = i;
            break;
        }
    }

    // struct tm.tm_year richiede gli anni dal 1900
    build_tm.tm_year -= 1900;
    
    // mktime converte struct tm in time_t
    return mktime(&build_tm);
}

time_t RtcModule::getModuleTimestamp() {
    return ds3231_time_unix_get(_rtc_handle);
}

void RtcModule::setModuleTimestamp(time_t time) {
    ds3231_time_time_t_set(_rtc_handle, time);
}   

void RtcModule::setEspTimestamp() {
    ds3231_set_esp_with_rtc(_rtc_handle);
}

tm RtcModule::getEspTimestamp() {
    tm time;
    struct timeval tv;
    gettimeofday(&tv, NULL);
    localtime_r(&tv.tv_sec, &time);
    return time;
}

float RtcModule::getTemperature() {
    return ds3231_temperature_get(_rtc_handle);
}