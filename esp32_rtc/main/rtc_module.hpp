#ifndef RTC_MODULE_HPP
#define RTC_MODULE_HPP

#include <sys/time.h>
#include "esp-idf-ds3231.h"

/**
 * @brief Classe per la gestione del tempo e dell'RTC su ESP32
 */
class RtcModule {
public:
    RtcModule();
    virtual ~RtcModule();

    time_t getInitialTimestamp();
    time_t getBuildTimestamp();
    time_t getModuleTimestamp();
    void setModuleTimestamp(time_t time);
    void setEspTime();

    float getTemperature();
    
private:
    i2c_master_bus_handle_t _bus_handle;
    rtc_handle_t* _rtc_handle;
};

#endif // RTC_MODULE_HPP