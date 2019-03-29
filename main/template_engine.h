#include <stdint.h>

typedef struct{
    int32_t var_loc;
    uint32_t cur_val;
    char ascii_str[34];  //Max size of 32 bit int plus - sign and a NULL char
    int32_t str_len;  //The actual length of the ascii string
} te_var;

typedef struct{
    char *template_str;
    uint32_t template_len;
    te_var *template_vars;
    uint32_t var_cnt;
} te_t;

int te_init_template(char* filePath, te_t *templatePtr, uint32_t *vars, uint32_t var_cnt);
int te_update_template(te_t *template_ptr, uint32_t *vars, uint32_t var_cnt);

