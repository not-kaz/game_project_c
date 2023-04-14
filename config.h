#ifndef CONFIG_H
#define CONFIG_H

struct config_entry {
	char *name;
	int value;
	struct config_entry *next;
};

void config_entry_register(struct config_entry *entry, char *name, int value);
void config_load(void);
void config_save(void);
void config_free(void);

#endif
