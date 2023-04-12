#include "config.h"
#include "common.h"
#include "log.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_CONFIG_ENTRIES 128
#define MAX_LINE_LEN 128

struct config_entry {
	char *key;
	int value;
	bool modified;
	struct config_entry *next;
};

struct config_entry config_map[MAX_CONFIG_ENTRIES];
const char *config_filename = "options.cfg";
const char *config_keys[] = {
	"resolution_x",
	"resolution_y",
	"vertical_sync"
};

static void assign_defaults(void)
{
	int ww, wh;

	//sdl_get_window_size(&ww, &wh);
	config_map[CONFIG_RES_X].key = (char *)(config_keys[CONFIG_RES_X]);
	//config_map[CONFIG_RES_X].value = ww;
	config_map[CONFIG_RES_X].value = 1024;
	config_map[CONFIG_RES_Y].key = (char *)(config_keys[CONFIG_RES_Y]);
	//config_map[CONFIG_RES_Y].value = wh;
	config_map[CONFIG_RES_Y].value = 720;
	config_map[CONFIG_VSYNC].key = (char *)(config_keys[CONFIG_VSYNC]);
	config_map[CONFIG_VSYNC].value = 1;
}

static void parse_config_pair(char *str)
{
	char *buf, *p;
	char *key, *val;

	buf = str_duplicate(str);
	if (!buf) {
		return;
	}
	p = buf;
	key = str_split(&p, "=");
	if (!key) {
		free(buf);
		return;
	}
	val = str_split(&p, "=");
	if (!val) {
		free(buf);
		return;
	}
	for (int i = 0; i < ARRAY_SIZE(config_keys); i++) {
		if (strncmp(key, config_keys[i], strlen(key)) == 0) {
			LOG_INFO("key: %s=%s, value: %s=%d", key, config_keys[i],
				val, config_map[i].value);
			config_map[i].value = atoi(val);
		}
	}
	free(buf);
}

void config_load(void)
{
	FILE *fp;
	char ln[MAX_LINE_LEN];

	assign_defaults();
	fp = fopen(config_filename, "r");
	if (!fp) {
		LOG_WARN("Could not load '%s' file, falling back to defaults.",
			config_filename);
		return;
	}
	while (fgets(ln, sizeof(ln), fp)) {
		ln[strcspn(ln, "\n")] = '\0';
		parse_config_pair(ln);
	}
	fclose(fp);
}

void config_save(void)
{
	FILE *fp;

	fp = fopen(config_filename, "w");
	if (!fp) {
		LOG_WARN("Could not save options to '%s' file!");
		return;
	}
	for (int i = 0; i < MAX_CONFIG_ENTRIES; i++) {
		/* Entry has no key assigned, assume entry is empty. */
		if (!config_map[i].key) {
			break;
		}
		/* TODO: Do error checking for this fprintf? */
		fprintf(fp, "%s=%d\n", config_map[i].key, config_map[i].value);
	}
	LOG_INFO("Saved current settings to '%s' file to game directory.",
		config_filename);
	fclose(fp);
}
