#include "config.h"
#include "common.h"
#include "log.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_CONFIG_ENTRIES 128
#define MAX_LINE_LEN 128

struct config_entry {
	char *key;
	int value;
};

/* TODO: Use a linked list or hash map instead of hard-coded array.
 *	 It would be more flexible and easier to modify later on. */
struct config_entry config_map[MAX_CONFIG_ENTRIES];
const char *config_filename = "options.cfg";
/* NOTICE: Must be in the same order as the config enum in the header. */
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
	char buf[MAX_LINE_LEN];
	char *k, *v;

	/* OPTIMIZE: Find a better way to copy a string or avoid doing it.
	 * This is safe but not the most performant, consider strdup()... */
	snprintf(buf, sizeof(buf), "%s", str);
	k = strtok(buf, "=");
	v = strtok(NULL, "=");
	if (!v) {
		/* Possible formatting failure? No delimiter or too many. */
		return;
	}
	for (int i = 0; i < ARRAY_SIZE(config_keys); i++) {
		if (strncmp(k, config_keys[i], strlen(k)) == 0) {
			LOG_INFO("key: %s=%s, value: %s=%d", k, config_keys[i],
				v, config_map[i].value);
			config_map[i].value = atoi(v);
		}
	}
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
