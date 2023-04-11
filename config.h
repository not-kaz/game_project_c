#ifndef CONFIG_H
#define CONFIG_H

enum {
	CONFIG_RES_X,
	CONFIG_RES_Y,
	CONFIG_VSYNC
};

void config_load(void);
void config_save(void);

#endif
