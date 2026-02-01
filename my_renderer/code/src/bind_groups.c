#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <webgpu/webgpu.h>
#include "bind_groups.h"
#include "shader_manager.h"
#include "utils.h"
#include "gpu.h"

static struct bg_layout_manager_private * g_bg_layout_manager = NULL; // global singleton for simplicity

struct bg_layout_manager_private {
    struct bg_layout_manager public_interface;
    WGPUDevice device;
    WGPUBindGroupLayout layouts[BGL_COUNT];
};

// Forward declarations
WGPUBindGroupLayout get_layout(bind_group_layout_id_t id);



bg_layout_manager_t new_layout_manager(void) {
    if (g_bg_layout_manager) {
        fprintf(stderr, "Bind group layout manager already exists! Returning existing instance.\n");
        return &g_bg_layout_manager->public_interface;
    }
    g_bg_layout_manager = malloc(sizeof(struct bg_layout_manager_private));
    g_bg_layout_manager->device = gpu_get_context()->device;
    memset(g_bg_layout_manager->layouts, 0, sizeof(g_bg_layout_manager->layouts));
    // Create bind group layouts
    g_bg_layout_manager->layouts[BGL_GLOBAL_UNIFORMS] = wgpuDeviceCreateBindGroupLayout(g_bg_layout_manager->device, &(WGPUBindGroupLayoutDescriptor){
        .entryCount = 1,
        .entries = &(WGPUBindGroupLayoutEntry){
            .binding = 0,
            .visibility = WGPUShaderStage_Vertex | WGPUShaderStage_Fragment,
            .buffer.type = WGPUBufferBindingType_Uniform,
            .buffer.hasDynamicOffset = false, // I want to keep dynamic offset
            .buffer.minBindingSize = sizeof(my_uniforms_t),
        },
    });

    g_bg_layout_manager->public_interface.get_layout = get_layout;
    g_bg_layout_manager->public_interface.destroy = NULL; // to be implemented
    return &g_bg_layout_manager->public_interface;
    
}
WGPUBindGroupLayout get_layout(bind_group_layout_id_t id) {
    if (!g_bg_layout_manager) {
        fprintf(stderr, "Bind group layout manager not initialized!\n");
        return NULL;
    }
    if (id >= BGL_COUNT) {
        fprintf(stderr, "Invalid bind group layout ID: %d\n", id);
        return NULL;
    }
    return g_bg_layout_manager->layouts[id];
}