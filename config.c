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

/* TODO: Move from int to int64_t in config entry struct! */

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

static void parse_config_line(char *ln)
{
	char *name, *val, *end;
	int num;

	name = str_split(&ln, "=");
	if (!name) {
		return;
	}
	val = str_split(&ln, "=");
	if (!val) {
		return;
	}
	/* FIXME: Possible loss of data w/ this cast. Larger to smaller type. */
	num = (int)(strtol(val, &end, 10));
	if (*end != '\0') {
		return;
	}
	hash_map_insert(registry, name, (void *)(intptr_t)(num));
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

static void reverse_entry_order(void)
{
	struct config_entry *curr, *next, *prev;

	curr = entries;
	next = prev = NULL;
	while (curr != NULL) {
		next = curr->next;
		curr->next = prev;
		prev = curr;
		curr = next;
	}
	entries = prev;
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
	entry->value = (restored >= 0) ? restored : value;
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
	/* HACK: This allows us to print out variables in the order they were
	 * registered, but it is not very performant. Reconsider. */
	reverse_entry_order();
	for (curr = entries; curr; curr = curr->next) {
		/* OPTIMIZE: Overwrite only modified entries. */
		fprintf(fp, "%s=%d\n", curr->name, curr->value);
	}
	/* LOG_INFO("Saved current settings to config file '%s'", filename); */
	fclose(fp);
}

void config_free(void)
{
	hash_map_destroy(registry);
}
