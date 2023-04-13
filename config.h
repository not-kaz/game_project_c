#ifndef CONFIG_H
#define CONFIG_H

void config_load(void);
void config_save(void);
void config_set_entry_value(char *name, int value);
int config_get_entry_value(char *name);

#endif
