/*
 * ESP32 RTC
 * Tongo
 */

#include <stdio.h>
#include "rtc_module.hpp"
#include "http_server.hpp"

extern "C" void app_main(void)
{
    printf("Hello RTC!\n");

    RtcModule* rtc = new RtcModule();
    rtc->setEspTimestamp();
    HttpServer* httpServer = new HttpServer(rtc);
    httpServer->startWebserver();
}
