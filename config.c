#include "config.h"

#include "common.h"
#include "hash_map.h"
#include "log.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static struct hash_map *registry;
static struct config_entry *entries;
static const char *filename = "config.cfg";

static void parse_config_line(char *ln)
{
	char *name, *val;

	name = str_split(&ln, "=");
	if (!name) {
		return;
	}
	val = str_split(&ln, "=");
	if (!val) {
		return;
	}
	/* IMPROVE: Move from atoi() to strtol(). Safer. */
	hash_map_insert(registry, name, (void *)(intptr_t)(atoi(val)));
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

	for (curr = entries; curr; curr = curr->next) {
		if ((!strcmp(curr->name, name)) || (curr == entry)) {
			return curr;
		}
	}
	return NULL;
}

void config_entry_register(struct config_entry *entry, char *name, int value)
{
	struct config_entry *found;
	int restored;

	if ((found = find_entry(entry, name))) {
		LOG_INFO("Config entry '%s' already registered.", found->name);
		return;
	}
	restored = (int)(intptr_t)(hash_map_at(registry, name));
	entry->name = name;
	entry->value = restored ? restored : value;
	entry->next = entries;
	entries = entry;
}

void config_load(void)
{
	FILE *fp;
	long len;
	char *str;

	registry = hash_map_create();
	if (!registry) {
		LOG_ERROR("Failed to create config registry.");
		exit(EXIT_FAILURE);
	}
	fp = fopen(filename, "r");
	if (!fp) {
		LOG_WARN("Failed to load config. Falling back to defaults.");
		return;
	}
	fseek(fp, 0, SEEK_END);
	len = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	str = malloc((size_t)(len + 1));
	if (!str) {
		LOG_WARN("Failed at reading config file.");
		fclose(fp);
		return;
	}
	fread(str, (size_t)(len), 1, fp);
	str[len] = '\0';
	parse_config_text(str);
	free(str);
	fclose(fp);
}

void config_save(void)
{
	FILE *fp;
	struct config_entry *curr;

	fp = fopen(filename, "w");
	if (!fp) {
		LOG_WARN("Failed at writing config file to disk.");
		return;
	}
	for (curr = entries; curr; curr = curr->next) {
		/* OPTIMIZE: Overwrite only modified entries. */
		fprintf(fp, "%s=%d\n", curr->name, curr->value);
	}
	LOG_INFO("Saved current settings to config file '%s'", filename);
	fclose(fp);
}

void config_free(void)
{
	hash_map_destroy(registry);
}
