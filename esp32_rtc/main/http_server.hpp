#ifndef HTTP_SERVER_HPP
#define HTTP_SERVER_HPPs

#include "rtc_module.hpp"
#include "esp_http_server.h"

class HttpServer {
public:
    HttpServer(RtcModule *RtcModule);
    virtual ~HttpServer();

    httpd_handle_t startWebserver();
    
private:
    RtcModule *_rtcModule;

    static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
    void handleWifiEvent(esp_event_base_t event_base, int32_t event_id, void* event_data);
    void wifiInitSoftap();

    httpd_uri_t get_home_uri;
    static esp_err_t get_home_handler(httpd_req_t *req);
    esp_err_t handleGetHomeContent(httpd_req_t *req);

    httpd_uri_t get_updatetimestamp_uri;
    static esp_err_t get_updatetimestamp_handler(httpd_req_t *req);
    esp_err_t handleGetUpdateTimestampContent(httpd_req_t *req);
};

#endif // HTTP_SERVER_HPP