#include "buffer_manager.h"
#include <stdio.h>
#include <stdlib.h>


static struct buffer_manager_private * g_buffer_manager = NULL; // global singleton for simplicity

struct buffer_manager_private {
    struct buffer_manager public_interface;
    WGPUBuffer buffers[BUFFER_COUNT];
    size_t buffer_count;
};

// Forward declarations
static void buffer_manager_destroy(void);
static void buffer_manager_write_buffer(WGPUQueue queue, buffer_id_t id, size_t offset, const void * data, size_t size);
static WGPUBuffer buffer_manager_get_buffer(buffer_id_t id);
static void buffer_manager_initialize_buffers(gpu_context_t * context);

buffer_manager_t new_buffer_manager(void) {
    if (g_buffer_manager) {
        fprintf(stderr, "Buffer manager already exists! Returning existing instance.\n");
        return &g_buffer_manager->public_interface;
    }
    g_buffer_manager = malloc(sizeof(struct buffer_manager_private));
    g_buffer_manager->public_interface.create_buffer = NULL; // to be implemented
    g_buffer_manager->public_interface.destroy_buffer = NULL; // to be implemented
    g_buffer_manager->public_interface.write_buffer = buffer_manager_write_buffer;
    g_buffer_manager->public_interface.get_buffer = buffer_manager_get_buffer;
    g_buffer_manager->public_interface.destroy = buffer_manager_destroy;
    g_buffer_manager->public_interface.initialize_buffers = buffer_manager_initialize_buffers;
    return &g_buffer_manager->public_interface;
}

static void buffer_manager_destroy(void) {
    if (!g_buffer_manager) return;
    free(g_buffer_manager);
    g_buffer_manager = NULL;
}

static void buffer_manager_write_buffer(WGPUQueue queue, buffer_id_t id, size_t offset, const void * data, size_t size) {
    if (!g_buffer_manager) {
        fprintf(stderr, "Buffer manager not initialized!\n");
        return;
    }
    if (id >= BUFFER_COUNT) {
        fprintf(stderr, "Invalid buffer ID: %d\n", id);
        return;
    }
    WGPUBuffer buffer = g_buffer_manager->buffers[id];
    if (!buffer) {
        fprintf(stderr, "Buffer not created for ID: %d\n", id);
        return;
    }
    wgpuQueueWriteBuffer(queue, buffer, offset, data, size);
}

static WGPUBuffer buffer_manager_get_buffer(buffer_id_t id) {
    if (!g_buffer_manager) {
        fprintf(stderr, "Buffer manager not initialized!\n");
        return NULL;
    }
    if (id >= BUFFER_COUNT) {
        fprintf(stderr, "Invalid buffer ID: %d\n", id);
        return NULL;
    }
    return g_buffer_manager->buffers[id];
}

static void buffer_manager_initialize_buffers(gpu_context_t * context) {
    if (!g_buffer_manager) {
        fprintf(stderr, "Buffer manager not created yet!\n");
        return;
    } // already initialized

    WGPUDevice device = context->device;

    // Create default buffers
    g_buffer_manager->buffers[BUFFER_VERTEX] = wgpuDeviceCreateBuffer(device, &(WGPUBufferDescriptor){
        .size = 1024 * sizeof(float), // 1 KB for vertex data
        .usage = WGPUBufferUsage_Vertex | WGPUBufferUsage_CopyDst,
    });

    g_buffer_manager->buffers[BUFFER_INDEX] = wgpuDeviceCreateBuffer(device, &(WGPUBufferDescriptor){
        .size = 512 * sizeof(uint16_t), // 512 bytes for index data
        .usage = WGPUBufferUsage_Index | WGPUBufferUsage_CopyDst,
    });

    g_buffer_manager->buffers[BUFFER_UNIFORM] = wgpuDeviceCreateBuffer(device, &(WGPUBufferDescriptor){
        .size = 256 * sizeof(float), // 256 bytes for uniform data
        .usage = WGPUBufferUsage_Uniform | WGPUBufferUsage_CopyDst,
    });
}