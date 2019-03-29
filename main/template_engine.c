#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <esp_log.h>

#include "filesystem.h"
#include "template_engine.h"

static const char *TAG="APP";

//Assumes null terminated string
int str_size(char * str){
    int i = 0;
    while(str[i] != 0x0){
        i++;
    }
    return i;
}
//TODO: Free the memory!!
int te_init_template(char* filePath, te_t *template_ptr, uint32_t *vars, uint32_t var_cnt){
    
    FILE* fp = fopen(filePath, "r");
    if(fp == NULL){
        ESP_LOGE(TAG,"Failed to open template %s",filePath);
    }
    
    template_ptr->var_cnt = 0;
    template_ptr->template_len = 0;
    template_ptr->template_str = malloc(1);
    signed char temp;
    do{
        temp = fgetc(fp);
        if(temp == EOF){
            break;
        }
        if(temp == '%'){
            te_var temp_var; //  = &template_ptr->template_vars[template_ptr->var_cnt];
            temp_var.var_loc = template_ptr->template_len;
            if(template_ptr->var_cnt > var_cnt){
                ESP_LOGE(TAG,"Not enough vars passed to init template function");
                while(1);  //TODO: Something better
            }else{
                temp_var.cur_val = vars[template_ptr->var_cnt];
            }
            memset(temp_var.ascii_str, 0x0, sizeof(temp_var.ascii_str));  //Clear the string
            itoa(temp_var.cur_val, temp_var.ascii_str, 10);
            temp_var.str_len = str_size(temp_var.ascii_str);
            template_ptr->var_cnt++;
            template_ptr->template_vars = realloc(template_ptr->template_vars, sizeof(te_var)*template_ptr->var_cnt);
            memcpy(&template_ptr->template_vars[template_ptr->var_cnt-1], &temp_var,sizeof(te_var));
            template_ptr->template_len += temp_var.str_len;
            template_ptr->template_str = realloc(template_ptr->template_str, template_ptr->template_len);
            memcpy(&template_ptr->template_str[template_ptr->template_len-temp_var.str_len],temp_var.ascii_str,temp_var.str_len);
            
        }else{
            template_ptr->template_len++;
            template_ptr->template_str = realloc(template_ptr->template_str, template_ptr->template_len);
            memcpy(&template_ptr->template_str[template_ptr->template_len-1],&temp,sizeof(temp));
        }
    }while(temp != EOF);

    printf("%.*s",template_ptr->template_len, template_ptr->template_str);
    return 0;
}

int te_update_template(te_t *template_ptr, uint32_t *vars, uint32_t var_cnt){
    ESP_LOGI("TAG","Template vars: %d, new vars: %d", template_ptr->var_cnt, var_cnt);
    if(template_ptr->var_cnt != var_cnt){
        return -1;
    }
    char* temp_str;
    for(uint32_t i=0; i<template_ptr->var_cnt; i++){
        if(template_ptr->template_vars[i].cur_val != vars[i]){
            int32_t old_var_size = template_ptr->template_vars[i].str_len;
            uint32_t hold_size = template_ptr->template_len - (template_ptr->template_vars[i].var_loc + template_ptr->template_vars[i].str_len);
            temp_str = malloc(hold_size);
            memcpy(temp_str, &template_ptr->template_str[template_ptr->template_vars[i].var_loc + template_ptr->template_vars[i].str_len],hold_size);
            itoa(vars[i], template_ptr->template_vars[i].ascii_str, 10);
            template_ptr->template_vars[i].str_len = str_size(template_ptr->template_vars[i].ascii_str);
            memcpy(&template_ptr->template_str[template_ptr->template_vars[i].var_loc],template_ptr->template_vars[i].ascii_str,template_ptr->template_vars[i].str_len);
            template_ptr->template_len = template_ptr->template_vars[i].var_loc+template_ptr->template_vars[i].str_len+hold_size;
            template_ptr->template_str = realloc(template_ptr->template_str,template_ptr->template_len);
            memcpy(&template_ptr->template_str[template_ptr->template_vars[i].var_loc+template_ptr->template_vars[i].str_len],temp_str,hold_size);
            //Resize the other var locations
            int32_t size_diff = template_ptr->template_vars[i].str_len - old_var_size;
            for(uint32_t j=i+1; j<template_ptr->var_cnt; j++){
                //ESP_LOGI(TAG,"Updated loc: %d", template_ptr->template_vars[j].var_loc);
                template_ptr->template_vars[j].var_loc += size_diff;
                //ESP_LOGI(TAG,"Updated loc: %d", template_ptr->template_vars[j].var_loc);
            }
            free(temp_str);
        }
    }
    printf("%.*s",template_ptr->template_len, template_ptr->template_str);
    return 0;
}
