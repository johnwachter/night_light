/* Simple HTTP + SSL Server Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <esp_wifi.h>
#include <esp_event_loop.h>
#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include <esp_https_server.h>

#include "ws2812.h"
#include "filesystem.h"
#include "color.h"
#include "template_engine.h"

#define WS2812_PIN 18
/* A simple example that demonstrates how to create GET and POST
 * handlers for the web server.
 * The examples use simple WiFi configuration that you can set via
 * 'make menuconfig'.
 * If you'd rather not, just change the below entries to strings
 * with the config you want -
 * ie. #define EXAMPLE_WIFI_SSID "mywifissid"
*/
#define EXAMPLE_WIFI_SSID CONFIG_WIFI_SSID
#define EXAMPLE_WIFI_PASS CONFIG_WIFI_PASSWORD

QueueHandle_t xLEDBrightnessQueue = NULL;
QueueHandle_t xLEDColorQueue = NULL;

te_t index_te;
uint32_t index_values[4] = {0,0,0,0};  //TODO: make this a struct
#define DIMMER 0
#define LED_R 1
#define LED_G 2
#define LED_B 3

/*LED task*/
void rainbow(void *pvParameters){
    uint32_t dimmer = 50;
	const uint8_t pixel_count = 12;
	const uint8_t delay = 50;
    color_t colors;
	rgbVal *pixels;

	pixels = malloc(sizeof(rgbVal) * pixel_count);
    for(uint8_t i=0; i < pixel_count; i++){
        pixels[i].r = (index_values[LED_R]*dimmer)/100;
        pixels[i].g = (index_values[LED_G]*dimmer)/100;
        pixels[i].b = (index_values[LED_B]*dimmer)/100;
    }

	while (1){
        if(xQueueReceive(xLEDBrightnessQueue, &dimmer, 0) || xQueueReceive(xLEDColorQueue, &colors, 0)){
            index_values[DIMMER] = dimmer;
            index_values[LED_R] = colors.r;
            index_values[LED_G] = colors.g;
            index_values[LED_B] = colors.b;

	    	for(uint8_t i=0; i < pixel_count; i++){
                pixels[i].r = (index_values[LED_R]*dimmer)/100;
                pixels[i].g = (index_values[LED_G]*dimmer)/100;
                pixels[i].b = (index_values[LED_B]*dimmer)/100;
            }
        }
        ws2812_setColors(pixel_count, pixels);

        vTaskDelay(delay/ portTICK_PERIOD_MS);
    }
}

 
static const char *TAG="APP";


/* An HTTP GET handler */
esp_err_t root_get_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "httpd request info mehtod: %d, uri: %s", req->method, req->uri);

    FILE* fp = fopen("/spiffs/index.html", "r");
    if(fp == NULL){
        ESP_LOGE(TAG,"Failed to open index");
    }
    uint32_t size = fs_get_file_size(fp); 
    ESP_LOGI(TAG,"File size: %d", size);
    char *buffer = (char*)calloc(size, sizeof(char));  //Get the memory
    if(buffer == NULL){
        httpd_resp_set_type(req, "text/html");
        httpd_resp_send(req, "<h1>404 Motherfer</h1>", -1); // -1 = use strlen()
        return ESP_FAIL;
    }
    fread(buffer, sizeof(char), size, fp);
    fclose(fp);
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, buffer, size); // -1 = use strlen()

    free(buffer);
    return ESP_OK;
}

const httpd_uri_t root = {
    .uri       = "/",
    .method    = HTTP_GET,
    .handler   = root_get_handler
};

esp_err_t root_css_get_handler(httpd_req_t *req)
{
    char tempUri[HTTPD_MAX_URI_LEN+1];
    char filePath[sizeof("/spiffs")+sizeof(tempUri)];
    memcpy(filePath, "/spiffs", sizeof("/spiffs"));

    ESP_LOGI(TAG, "httpd request info mehtod: %d, uri: %s", req->method, req->uri);
    memcpy(tempUri, req->uri, sizeof(tempUri));
    strcat(filePath,tempUri);
    FILE* fp = fopen(filePath, "r");
    if(fp == NULL){
        httpd_resp_set_type(req, "text/html");
        httpd_resp_send(req, "<h1>404 Motherfer</h1>", -1); // -1 = use strlen()
        return ESP_OK;
    }
    uint32_t size = fs_get_file_size(fp); 
    ESP_LOGI(TAG,"File size: %d", size);
    char *buffer = (char*)calloc(size, sizeof(char));  //Get the memory
    if(buffer == NULL){
        httpd_resp_set_type(req, "text/html");
        httpd_resp_send(req, "<h1>404 Motherfer</h1>", -1); // -1 = use strlen()
        return ESP_FAIL;
    }
    fread(buffer, sizeof(char), size, fp);
    fclose(fp);
    httpd_resp_set_type(req, "text/css");
    httpd_resp_send(req, buffer, size); // -1 = use strlen()

    free(buffer);
    return ESP_OK;
}

const httpd_uri_t css_root = {
    .uri       = "/style.css",
    .method    = HTTP_GET,
    .handler   = root_css_get_handler
};

/* An HTTP GET handler */
esp_err_t js_get_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "httpd request info mehtod: %d, uri: %s", req->method, req->uri);

    FILE* fp = fopen("/spiffs/main.js", "r");
    if(fp == NULL){
        ESP_LOGE(TAG,"Failed to open index");
    }
    uint32_t size = fs_get_file_size(fp); 
    ESP_LOGI(TAG,"File size: %d", size);
    char *buffer = (char*)calloc(size, sizeof(char));  //Get the memory
    if(buffer == NULL){
        httpd_resp_set_type(req, "text/html");
        httpd_resp_send(req, "<h1>404 Motherfer</h1>", -1); // -1 = use strlen()
        return ESP_FAIL;
    }
    fread(buffer, sizeof(char), size, fp);
    fclose(fp);
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, buffer, size); // -1 = use strlen()

    free(buffer);
    return ESP_OK;
}

const httpd_uri_t js_root = {
    .uri       = "/main.js",
    .method    = HTTP_GET,
    .handler   = root_css_get_handler
};

const char dimmer_str[] = "dimmer=";
uint32_t lastBright = 50;
uint16_t dimmer_val;
char dimmer_val_str[5];
esp_err_t post_on_off_handler(httpd_req_t *req){
    char content [100];

    memset(dimmer_val_str, 0x0, sizeof(dimmer_val_str));
    /* Truncate if content length larger than the buffer */
    size_t recv_size = MIN(req->content_len, sizeof(content));

    int ret = httpd_req_recv(req, content, recv_size);
    if (ret <= 0) {  /* 0 return value indicates connection closed */
            /* Check if timeout occurred */
            if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
                        httpd_resp_send_408(req);
            }
            return ESP_FAIL;
    }
    ESP_LOGI(TAG,"Dimmer Request Received length: %d, message: %s", req->content_len, content);
    
    if(strncmp(content, dimmer_str, sizeof(dimmer_str)-1)==0){
        for(uint8_t i=0; i<5; i++){
            if((content[sizeof(dimmer_str)-1+i] >= '0') && (content[sizeof(dimmer_str)-1+i] <= '9')){
                dimmer_val_str[i]=content[sizeof(dimmer_str)-1+i];
            }else{
                break;
            }
        }
        dimmer_val = atoi(dimmer_val_str);
        ESP_LOGI(TAG, "LED dimmed to %d", dimmer_val);
        lastBright = 0;
        index_values[DIMMER] = dimmer_val;
        xQueueSend(xLEDBrightnessQueue, &dimmer_val, 0);
    }else{
        ESP_LOGI(TAG, "Unrecognized POST request: %s", content);
    }
    //Send the updated values
    te_update_template(&index_te, index_values, sizeof(index_values)/sizeof(uint32_t));
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, index_te.template_str, index_te.template_len); // -1 = use strlen()
    return ESP_OK;
}
//UI settings
const httpd_uri_t post_on_off = {
    .uri       = "/dimmer",  //TODO: we might want toget this specific to each button
    .method    = HTTP_POST,
    .handler   = post_on_off_handler
};

esp_err_t post_color_handler(httpd_req_t *req){
    char content [100];
    uint8_t colors[3];
    char color_val_str[5];
    uint8_t color_val_str_cnt=0;
    uint8_t color_cnt=0;    

    memset(color_val_str, 0x0, sizeof(color_val_str));
    /* Truncate if content length larger than the buffer */
    size_t recv_size = MIN(req->content_len, sizeof(content));

    int ret = httpd_req_recv(req, content, recv_size);
    if (ret <= 0) {  /* 0 return value indicates connection closed */
            /* Check if timeout occurred */
            if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
                        httpd_resp_send_408(req);
            }
            return ESP_FAIL;
    }
    ESP_LOGI(TAG,"Color Request Received length: %d, message: %s", req->content_len, content);

    for(uint8_t i=2; i<recv_size && color_cnt < 3; i++){  //We start at 2 to ignore the "r="
        if((content[i] >= '0') && (content[i] <= '9')){
            color_val_str[color_val_str_cnt]=content[i];
            color_val_str_cnt++;
        }else{
            colors[color_cnt]=atoi(color_val_str);
            memset(color_val_str, 0x0, sizeof(color_val_str));
            color_cnt++;
            i+=2;  //Increment past the 'c=' in the string
            color_val_str_cnt=0;
        }
    }
    color_t color_queue;
    color_queue.r=colors[0];
    color_queue.g=colors[1];
    color_queue.b=colors[2];
    ESP_LOGI(TAG,"LED Colors: red=%d, green=%d, blue=%d",color_queue.r, color_queue.g, color_queue.b);
    xQueueSend(xLEDColorQueue, &color_queue, 0);
    //httpd_resp_send_chunk(req, NULL, 0);
    httpd_resp_set_status(req, "303 See Other");
    httpd_resp_set_hdr(req, "Location", "/");
    httpd_resp_sendstr(req, "LED updated");
    return ESP_OK;
}
const httpd_uri_t post_color= {
    .uri       = "/color",  //TODO: we might want toget this specific to each button
    .method    = HTTP_POST,
    .handler   = post_color_handler
};

httpd_handle_t start_webserver(void)
{
    httpd_handle_t server = NULL;

    // Start the httpd server
    ESP_LOGI(TAG, "Starting server");

    httpd_ssl_config_t conf = HTTPD_SSL_CONFIG_DEFAULT();

    extern const unsigned char cacert_pem_start[] asm("_binary_cacert_pem_start");
    extern const unsigned char cacert_pem_end[]   asm("_binary_cacert_pem_end");
    conf.cacert_pem = cacert_pem_start;
    conf.cacert_len = cacert_pem_end - cacert_pem_start;

    extern const unsigned char prvtkey_pem_start[] asm("_binary_prvtkey_pem_start");
    extern const unsigned char prvtkey_pem_end[]   asm("_binary_prvtkey_pem_end");
    conf.prvtkey_pem = prvtkey_pem_start;
    conf.prvtkey_len = prvtkey_pem_end - prvtkey_pem_start;

    esp_err_t ret = httpd_ssl_start(&server, &conf);
    if (ESP_OK != ret) {
        ESP_LOGI(TAG, "Error starting server!");
        return NULL;
    }

    // Set URI handlers
    ESP_LOGI(TAG, "Registering URI handlers");
    httpd_register_uri_handler(server, &root);
    httpd_register_uri_handler(server, &css_root);
    httpd_register_uri_handler(server, &post_on_off);
    httpd_register_uri_handler(server, &post_color);
    httpd_register_uri_handler(server, &js_root);
    return server;
}

void stop_webserver(httpd_handle_t server)
{
    // Stop the httpd server
    httpd_ssl_stop(server);
}




// ------------------------- application boilerplate ------------------------

static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    httpd_handle_t *server = (httpd_handle_t *) ctx;

    switch(event->event_id) {
    case SYSTEM_EVENT_STA_START:
        ESP_LOGI(TAG, "SYSTEM_EVENT_STA_START");
        ESP_ERROR_CHECK(esp_wifi_connect());
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        ESP_LOGI(TAG, "SYSTEM_EVENT_STA_GOT_IP");
        ESP_LOGI(TAG, "Got IP: '%s'",
                ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));

        /* Start the web server */
        if (*server == NULL) {
            *server = start_webserver();
        }
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        ESP_LOGI(TAG, "SYSTEM_EVENT_STA_DISCONNECTED");
        ESP_ERROR_CHECK(esp_wifi_connect());

        /* Stop the web server */
        if (*server) {
            stop_webserver(*server);
            *server = NULL;
        }
        break;
    default:
        break;
    }
    return ESP_OK;
}

static void initialise_wifi(void *arg)
{
    tcpip_adapter_init();
    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, arg));
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = EXAMPLE_WIFI_SSID,
            .password = EXAMPLE_WIFI_PASS,
        },
    };
    ESP_LOGI(TAG, "Setting WiFi configuration SSID %s...", wifi_config.sta.ssid);
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
}

void app_main()
{
    static httpd_handle_t server = NULL;
    ESP_ERROR_CHECK(nvs_flash_init());
    fs_init();
    ws2812_init(WS2812_PIN);
    xLEDBrightnessQueue = xQueueCreate(5, sizeof(uint32_t));  //5 max items 
    xLEDColorQueue = xQueueCreate(5, sizeof(color_t));  //5 max items 
    if(xLEDBrightnessQueue == NULL){
        ESP_LOGE(TAG,"Failed to create queue for brightness");
        while(1);
    }
    xTaskCreate(rainbow, "ws2812 driver", 4096, NULL, 10, NULL);
    initialise_wifi(&server);
}
