#include "config.h"

#include "common.h"
#include "log.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_SAVED_ENTRIES 256

/* TODO: Move to dynamic array or hash table. */
struct {
	char *name;
	char *value;
} saved_entries[MAX_SAVED_ENTRIES];

static struct config_entry *config_entries;
static const char *config_filename = "config.cfg";

static void parse_config_line(char *ln)
{
	/* TODO: Find a way to avoid memory allocation w/ str_duplicate() */
	char *name, *val;

	name = str_split(&ln, "=");
	if (!name) {
		return;
	}
	val = str_split(&ln, "=");
	if (!val) {
		return;
	}
	for (int i = 0; i < MAX_SAVED_ENTRIES; i++) {
		if (!saved_entries[i].name || !saved_entries[i].value) {
			saved_entries[i].name = str_duplicate(name);
			saved_entries[i].value = str_duplicate(val);
			break;
		}
	}
}

static void parse_config_text(char *str)
{
	char *buf, *ptr, *ln;

	buf = str_duplicate(str);
	if (!buf) {
		LOG_WARN("Failed to parse configuration text file.");
		return;
	}
	ptr = buf;
	while ((ln = str_split(&ptr, "\n"))) {
		parse_config_line(ln);
	}
	free(buf);
}

static struct config_entry *find_entry(struct config_entry *entry, char *name)
{
	struct config_entry *curr;

	for (curr = config_entries; curr; curr = curr->next) {
		if ((!strcmp(curr->name, name)) || (curr == entry)) {
			return curr;
		}
	}
	return NULL;
}

void config_entry_register(struct config_entry *entry, char *name, int value)
{
	struct config_entry *found;

	if ((found = find_entry(entry, name))) {
		LOG_INFO("Config entry '%s' already registered.", found->name);
		return;
	}
	for (int i = 0; i < MAX_SAVED_ENTRIES; i++) {
		/* OPTIMIZE: String comparing is not very performant. */
		if (!saved_entries[i].name && !saved_entries[i].value) {
			break;
		}
		if (!strcmp(name, saved_entries[i].name)) {
			int val;
			char *end;

			/* OPTIMIZE: If values are the same, don't reassign. */
			val = strtol(saved_entries[i].value, &end, 10);
			if (*end != '\0') {
				LOG_WARN("Input from config file was invalid.");
				LOG_INFO("%d", val);
				break;
			}
			value = val;
			LOG_INFO("Loaded new value from file, %s=%d", name,
				value);
		}
	}
	entry->name = name;
	entry->value = value;
	entry->next = config_entries;
	config_entries = entry;
}

void config_load(void)
{
	FILE *fp;
	long len;
	char *str;

	fp = fopen(config_filename, "r");
	if (!fp) {
		LOG_WARN("Failed to load config. Falling back to defaults.");
		return;
	}
	fseek(fp, 0, SEEK_END);
	len = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	str = malloc(len + 1);
	if (!str) {
		LOG_WARN("Failed at reading config file.");
		fclose(fp);
		return;
	}
	fread(str, len, 1, fp);
	str[len] = '\0';
	parse_config_text(str);
	free(str);
	fclose(fp);
}

void config_save(void)
{
	FILE *fp;
	struct config_entry *curr;

	fp = fopen(config_filename, "w");
	if (!fp) {
		LOG_WARN("Failed at writing config file to disk.");
		return;
	}
	for (curr = config_entries; curr; curr = curr->next) {
		/* TODO: Find out if overwriting everything is faster than
		 * parsing through and finding what to overwrite. */
		fprintf(fp, "%s=%d\n", curr->name, curr->value);
	}
	LOG_INFO("Saved current settings to config file '%s'", config_filename);
	fclose(fp);
}

void config_free(void)
{
	for (int i = 0; i < MAX_SAVED_ENTRIES; i++) {
		/* IMPROVE: Reset all pointers to NULL to prevent UB if somebody
		 * for some reason accesses data after free(). */
		if (!saved_entries[i].name || !saved_entries[i].value) {
			break;
		}
		free(saved_entries[i].name);
		free(saved_entries[i].value);
		saved_entries[i].name = NULL;
	}
}
