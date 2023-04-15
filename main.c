#include "config.h"
#include "hash_map.h"
#include "log.h"

#include <stdlib.h>

static struct config_entry res_x;
static struct config_entry res_y;
static struct config_entry vsync;

int main(void)
{
	struct hash_map *map;

	map = hash_map_create();
	hash_map_insert(map, "res_x", (void *)(intptr_t)(1280));
	hash_map_insert(map, "res_y", (void *)(intptr_t)(1280));
	hash_map_insert(map, "vsync", (void *)(intptr_t)(1280));
	hash_map_insert(map, "volume", (void *)(intptr_t)(1280));
	hash_map_insert(map, "gamma", (void *)(intptr_t)(1280));
	hash_map_insert(map, "mouse_sens", (void *)(intptr_t)(1280));
	hash_map_destroy(map);
	config_load();
	config_entry_register(&res_x, "res_x", 1280);
	config_entry_register(&res_y, "res_y", 720);
	config_entry_register(&vsync, "vsync", 1);
	config_save();
	config_free();
	return 0;
}
