#include "http_server.hpp"
#include "sdkconfig.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_http_server.h"
#include "nvs_flash.h"

#define ESP_WIFI_SSID "esp_rtc"
#define ESP_WIFI_PASS "password"
#define ESP_WIFI_CHANNEL 1
#define MAX_STA_CONN 2

static const char *TAG = "HttpServer";

HttpServer::HttpServer(RtcModule *rtcModule) {
    _rtcModule = rtcModule;

    get_home_uri = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = get_home_handler,
        .user_ctx = this
    };

    get_updatetimestamp_uri = {
        .uri = "/update-timestamp/*",
        .method = HTTP_GET,
        .handler = get_updatetimestamp_handler,
        .user_ctx = this
    };

    wifiInitSoftap();
}

HttpServer::~HttpServer() {
}

void HttpServer::wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    HttpServer* instance = static_cast<HttpServer*>(arg);
    instance->handleWifiEvent(event_base, event_id, event_data);
}

void HttpServer::handleWifiEvent(esp_event_base_t event_base, int32_t event_id, void* event_data) {
    printf("Event nr: %ld!\n", event_id);
}

void HttpServer::wifiInitSoftap() {
    nvs_flash_init();
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT(); // always start with this
    esp_wifi_init(&cfg);

    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL);
    wifi_config_t wifi_config = {
        .ap = {
            .ssid = ESP_WIFI_SSID,
            .password = ESP_WIFI_PASS,
            .ssid_len = strlen(ESP_WIFI_SSID),
            .channel = ESP_WIFI_CHANNEL,
            .authmode = WIFI_AUTH_WPA2_PSK,
            .max_connection = MAX_STA_CONN,
            .pmf_cfg = {
                .required = true,
            },
        },
    };


    esp_wifi_set_mode(WIFI_MODE_AP);
    esp_wifi_set_config(WIFI_IF_AP, &wifi_config);
    esp_wifi_start();

    ESP_LOGI(TAG, "wifi_init_softap finished. SSID:%s password:%s channel:%d",
             ESP_WIFI_SSID, ESP_WIFI_PASS, ESP_WIFI_CHANNEL);
}

/* An HTTP GET handler */

esp_err_t HttpServer::get_home_handler(httpd_req_t *req) {
    HttpServer* instance = static_cast<HttpServer*>(req->user_ctx);
    return instance->handleGetHomeContent(req);
}

esp_err_t HttpServer::handleGetHomeContent(httpd_req_t *req) {
    char response_buffer[1000]; // Assicurati che sia abbastanza grande
    
    tm time = _rtcModule->getEspTimestamp();
    snprintf(response_buffer, sizeof(response_buffer),
        "<!DOCTYPE html><html>"
        "<head><meta http-equiv=\"refresh\" content=\"1;url=/\"></head>"
        "<body>"
        "<h1>ESP32 RTC</h1>"
        "<ul>"
        "<li>Timestamp: %d/%d/%d %d:%d:%d</li>"
        "<li>Temperatura: %.2f C</li>"
        "</ul>"
        "<a href='#' id='syncLink' style='padding:10px; background:#ddd; text-decoration:none; border-radius:5px;'>Aggiorna timestamp</a>"
        "<script>"
            "document.getElementById('syncLink').addEventListener('click', function(e) {"
                "e.preventDefault();" // Impedisce al link di andare a '#'
                "const now = Math.floor(Date.now() / 1000);" // Genera Unix Timestamp
                "window.location.href = '/update-timestamp/' + now;" // Reindirizza
            "});"
        "</script>"
        "</body></html>",
        time.tm_mday, (time.tm_mon + 1), (time.tm_year + 1900), time.tm_hour, time.tm_min, time.tm_sec,
        _rtcModule->getTemperature()
    );
    // free(&time);

    httpd_resp_send(req, response_buffer, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

esp_err_t HttpServer::get_updatetimestamp_handler(httpd_req_t *req) {
    HttpServer* instance = static_cast<HttpServer*>(req->user_ctx);
    return instance->handleGetUpdateTimestampContent(req);
}

esp_err_t HttpServer::handleGetUpdateTimestampContent(httpd_req_t *req) {
    // 1. Otteniamo l'URL completo dalla richiesta
    const char* full_url = req->uri; 
    const char* base_route = "/update-timestamp/";

    // 2. Spostiamo il puntatore all'inizio del timestamp
    // Verifichiamo che l'URL inizi correttamente per evitare crash
    if (strncmp(full_url, base_route, strlen(base_route)) == 0) {
        const char* ts_str = full_url + strlen(base_route);

        if (strlen(ts_str) > 0) {
            // 3. Conversione in numero
            long long timestamp_raw = std::atoll(ts_str);
            time_t now = (time_t)timestamp_raw;

            printf("Sincronizzazione via URL: %lld\n", timestamp_raw);

            _rtcModule->setModuleTimestamp(now);
            _rtcModule->setEspTimestamp();

            const char* html_response =
                "<!DOCTYPE html><html>"
                "<head><meta http-equiv=\"refresh\" content=\"5;url=/\"></head>"
                "<body>"
                "<h1>Timestamp aggiornato</h1>"
                "</body></html>";
            httpd_resp_send(req, html_response, HTTPD_RESP_USE_STRLEN);

            return ESP_OK;
        }
    }

    httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Timestamp non trovato nell'URL");
    return ESP_FAIL;
}

httpd_handle_t HttpServer::startWebserver() {
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.uri_match_fn = httpd_uri_match_wildcard;
    //config.lru_purge_enable = true;

    if (httpd_start(&server, &config) == ESP_OK) {
        ESP_LOGI(TAG, "Server ok, registering the URI handlers...");
        httpd_register_uri_handler(server, &get_home_uri);
        httpd_register_uri_handler(server, &get_updatetimestamp_uri);
        
        return server;
    }
    ESP_LOGI(TAG, "Error starting server");
    return NULL;
}