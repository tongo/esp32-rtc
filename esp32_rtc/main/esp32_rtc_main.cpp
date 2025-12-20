/*
 * ESP32 RTC
 * Tongo
 */

#include <stdio.h>
#include "esp-idf-ds3231.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "rtc_module.hpp"

extern "C" void app_main(void)
{
    printf("Hello RTC!\n");

    RtcModule rtc = RtcModule();

    time_t now;
    char strftime_buf[64];
    struct tm timeinfo;

    now = rtc.getModuleTimestamp();
    localtime_r(&now, &timeinfo);
    
    // Configurazione ora ESP da RTC module
    printf("Init timestamp");
    rtc.setModuleTimestamp(rtc.getInitialTimestamp());
    rtc.setEspTime();
    
    struct timeval tv;
    for (;;) {
        gettimeofday(&tv, NULL);
        localtime_r(&tv.tv_sec, &timeinfo);
        strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
        printf("The current time (from ESP32): %s - %ld\n", strftime_buf, (long)tv.tv_usec);

        float temp = rtc.getTemperature();
        printf("Temperature: %f\n", temp);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
