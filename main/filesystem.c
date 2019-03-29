#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_spiffs.h"

#include "filesystem.h"

static const char *TAG="APP";

esp_err_t fs_init(void){
    ESP_LOGI(TAG, "Initializing SPIFFS");

    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = true
    };

    esp_err_t ret = esp_vfs_spiffs_register(&conf);

    if(ret != ESP_OK){
        if(ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
            return ESP_FAIL;
        }else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(TAG, "Failed to find SPIFFS partition");
            return ESP_FAIL;
        }else {
            ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
            return ESP_FAIL;
        }
        return ESP_FAIL;
    }

    size_t total = 0, used = 0;
    ret = esp_spiffs_info(NULL, &total, &used);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
        return ESP_FAIL;
    } else {
        ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
    }

    return ESP_OK;
}

uint32_t fs_get_file_size(FILE* fp){
    if(fp){
        fseek(fp, 0, SEEK_END);
        uint32_t fileSize = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        return fileSize;
    }else{
        return 0;
    }
}
