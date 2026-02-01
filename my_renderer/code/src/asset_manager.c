#include "asset_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "shader_manager.h"


static struct asset_manager_private * g_asset_manager = NULL; // global singleton for simplicity

struct asset {
    vertex_input_t * data;
    size_t count;
    size_t capacity;
};

struct asset_manager_private {
    struct asset_manager public_interface;
    asset_t * assets;
    size_t asset_count;
    size_t asset_capacity;
    // Add any private members needed for asset management
};

// Forward declarations

void asset_manager_destroy(void);
void const * asset_manager_get_asset_data(char const * asset_name);

void asset_append_vertex(asset_t * array, vertex_input_t item);
void asset_free_asset(asset_t * array);
void asset_manager_append_asset(asset_t * asset);
void asset_manager_free_assets(void);

asset_manager_t new_asset_manager(void) {
    if (g_asset_manager) {
        fprintf(stderr, "Asset manager already exists! Returning existing instance.\n");
        return &g_asset_manager->public_interface;
    }
    g_asset_manager = malloc(sizeof(struct asset_manager_private));
    g_asset_manager->public_interface.load_mesh = NULL; // to be implemented
    g_asset_manager->public_interface.destroy = asset_manager_destroy; // to be implemented
    g_asset_manager->public_interface.get_asset_data = asset_manager_get_asset_data; // to be implemented
    g_asset_manager->public_interface.append_asset = asset_manager_append_asset; // to be implemented
    g_asset_manager->public_interface.free_assets = asset_manager_free_assets; // to be implemented
    return &g_asset_manager->public_interface;
}

void asset_manager_destroy(void) {
    if (!g_asset_manager) return;
    free(g_asset_manager);
    g_asset_manager = NULL;
}
void const * asset_manager_get_asset_data(char const * asset_name) {
    if (!g_asset_manager) {
        fprintf(stderr, "Asset manager not initialized!\n");
        return NULL;
    }
    
    fprintf(stderr, "Asset not found: %s\n", asset_name);
    return NULL;
}

void asset_append_vertex(asset_t * array, vertex_input_t item) {
    if (array->count >= array->capacity) {
        size_t new_capacity = array->capacity == 0 ? 4 : array->capacity * 2;
        array->data = realloc(array->data, new_capacity * sizeof(vertex_input_t));
        array->capacity = new_capacity;
    }
    array->data[array->count++] = item;
}
void asset_free_asset(asset_t * array) {
    if (!array) return;

    free(array->data);
    array->data = NULL;
    array->count = 0;
    array->capacity = 0;
}
void asset_manager_append_asset(asset_t * asset) {
    if (!g_asset_manager) return;
    if (g_asset_manager->asset_count >= g_asset_manager->asset_capacity) {
        size_t new_capacity = g_asset_manager->asset_capacity == 0 ? 4 : g_asset_manager->asset_capacity * 2;
        g_asset_manager->assets = realloc(g_asset_manager->assets, new_capacity * sizeof(asset_t));
        g_asset_manager->asset_capacity = new_capacity;
    }
    g_asset_manager->assets[g_asset_manager->asset_count++] = *asset;
}
void asset_manager_free_assets(void) {
    if (!g_asset_manager) return;
    for (size_t i = 0; i < g_asset_manager->asset_count; ++i) {
        asset_free_asset(&g_asset_manager->assets[i]);
    }
    free(g_asset_manager->assets);
    g_asset_manager->assets = NULL;
    g_asset_manager->asset_count = 0;
    g_asset_manager->asset_capacity = 0;
}