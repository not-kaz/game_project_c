#include "config.h"
#include "common.h"
#include "log.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LEN 128

struct config_entry {
	char *name;
	int value;
	struct config_entry *next;
};

/* Globals */
static struct config_entry *config_entries = NULL;
static const char *config_filename = "settings.cfg";
static bool safe_mode = false;
/* Initial config entries */
static struct config_entry res_x;
static struct config_entry res_y;
static struct config_entry vsync;

static struct config_entry *find_entry(char *name)
{
	struct config_entry *curr;

	for (curr = config_entries; curr; curr = curr->next) {
		if (strcmp(name, curr->name) == 0) {
			return curr;
		}
	}
	return NULL;
}

static void register_entry(struct config_entry *entry, char *name, int value)
{
	if (find_entry(name)) {
		LOG_WARN("Config entry '%s' is already registered.", name);
		return;
	}
	/* FIXME: Name should be str_duplicated() but then we need to free it.
	 * Find a clean solution to this. For now, point to literal. */
	entry->name = name;
	entry->value = value;
	entry->next = config_entries;
	config_entries = entry;
}

static void register_entries(void)
{
	int rx, ry, rv;

	/* HACK: If statement should check is safe_mode is on, not opposite.
	 * Currently used like this for testing purposes.
	 * if (safe_mode) { */
	if (!safe_mode) {
		rx = 640;
		ry = 480;
		rv = 1;
	} else {
		/* TODO: Implement SDL display mode reading to fill variables
		 * with sane defaults depending on user display etc. */
	}
	register_entry(&res_x, "res_x", rx);
	register_entry(&res_y, "res_y", ry);
	register_entry(&vsync, "vsync", rv);
}

static void parse_config_pair(char *str)
{
	char *buf, *ptr;
	char *name, *val;
	struct config_entry *curr;

	buf = str_duplicate(str);
	if (!buf) {
		return;
	}
	ptr = buf;
	name = str_split(&ptr, "=");
	if (!name) {
		free(buf);
		return;
	}
	val = str_split(&ptr, "=");
	if (!val) {
		free(buf);
		return;
	}
	for (curr = config_entries; curr; curr = curr->next) {
		if (strcmp(name, curr->name) == 0) {
			LOG_INFO("Loaded a saved value for key %s.", name);
			config_set_entry_value(curr->name, atoi(val));
		}
	}
	free(buf);
}

void config_load(void)
{
	FILE *fp;
	char ln[MAX_LINE_LEN];

	register_entries();
	fp = fopen(config_filename, "r");
	if (!fp) {
		LOG_WARN("No '%s' file found, falling back to defaults.",
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
	struct config_entry *curr;

	fp = fopen(config_filename, "w");
	if (!fp) {
		LOG_WARN("Could not save options to '%s' file!");
		return;
	}
	for (curr = config_entries; curr; curr = curr->next) {
		fprintf(fp, "%s=%d\n", curr->name, curr->value);
	}
	LOG_INFO("Saved current settings to '%s' file to game directory.",
		config_filename);
	fclose(fp);
}

void config_set_entry_value(char *name, int value)
{
	struct config_entry *curr;

	curr = find_entry(name);
	if (!curr) {
		LOG_WARN("Failed to set config variable '%s'. Not found.",
			name);
		return;
	}
	curr->value = value;
}

int config_get_entry_value(char *name)
{
	struct config_entry *curr;

	curr = find_entry(name);
	if (!curr) {
		return 0;
	}
	return curr->value;
}
