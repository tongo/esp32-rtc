/*
 * ESP32 RTC
 * Tongo
 */

#include <stdio.h>
#include "esp-idf-ds3231.h"
#include "time_utils.c"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void app_main(void)
{
    printf("Hello RTC!\n");

    // Allocte memory for the pointer of i2c_master_bus_handle_t
    i2c_master_bus_handle_t* bus_handle = 
        (i2c_master_bus_handle_t*)malloc(sizeof(i2c_master_bus_handle_t));
    // Create the i2c_master_bus_config_t struct and assign values.
    i2c_master_bus_config_t i2c_mst_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = -1,
        .scl_io_num = GPIO_NUM_22,
        .sda_io_num = GPIO_NUM_21,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    i2c_new_master_bus(&i2c_mst_config, bus_handle);
    rtc_handle_t* rtc_handle = ds3231_init(bus_handle);

    time_t now;
    char strftime_buf[64];
    struct tm timeinfo;

    now = ds3231_time_unix_get(rtc_handle);
    localtime_r(&now, &timeinfo);
    
    // Configurazione ora ESP da RTC module
    printf("Time out of date -> update with build time");
    time_t build_time = get_build_time();
    ds3231_time_time_t_set(rtc_handle, build_time);
    ds3231_set_esp_with_rtc(rtc_handle);
    
    struct timeval tv;
    for (;;) {
        gettimeofday(&tv, NULL);
        localtime_r(&tv.tv_sec, &timeinfo);
        strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
        printf("The current time (from ESP32): %s - %ld\n", strftime_buf, (long)tv.tv_usec);

        float temp = ds3231_temperature_get(rtc_handle);
        printf("Temperature: %f\n", temp);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
