#include "config.h"

#include "common.h"
#include "hash_map.h"
#include "log.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static struct hash_map *registry;
static const char *filename = "config.cfg";

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

void config_register_entry(char *name, int value)
{
	int restored;

	restored = (int)(intptr_t)(hash_map_at(registry, name));
	value = (restored >= 0) ? restored : value;
	hash_map_insert(registry, name, (void *)(intptr_t)(value));
	LOG_INFO("Added entry: %s - %d", name, value);
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
	struct hash_map_iter *iter;
	char *key;
	void *val;

	fp = fopen(filename, "w");
	if (!fp) {
		LOG_WARN("Failed at writing config file to disk.");
		return;
	}
	iter = hash_map_iter_create(registry);
	while (hash_map_iter_next(iter, &key, &val) >= 0) {
		fprintf(fp, "%s=%d\n", key, (int)(intptr_t)(val));
	}
	hash_map_iter_destroy(iter);
	fclose(fp);
}

void config_free(void)
{
	hash_map_destroy(registry);
}
