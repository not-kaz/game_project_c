#ifndef CONFIG_H
#define CONFIG_H

void config_load(void);
void config_register_entry(char *name, int value);
void config_save(void);
void config_free(void);

#endif
