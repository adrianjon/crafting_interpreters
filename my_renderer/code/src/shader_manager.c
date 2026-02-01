#include "shader_manager.h"
#include <webgpu/webgpu.h>
#include <stdbool.h>
#include <stdlib.h>
#include "utils.h"

/*

Shaders define interfaces
Pipelines define execution
Buffers define data
Draw calls only reference offsets

*/

static struct shader_manager_private * g_shader_manager = NULL; // global singleton for simplicity

struct shader_binding {
    uint32_t binding;
    WGPUBufferBindingType type;
    WGPUShaderStage stage;
};
struct shader_layout {
    struct shader_binding * bindings;
    uint32_t binding_count;
};

struct shader {
    WGPUShaderStage stage;
    WGPUShaderModule module;
    uint64_t hash;
};

struct shader_cache_entry {
    uint64_t hash;
    shader_t shader;
};

struct shader_cache {
    struct shader_cache_entry * entries;
    size_t count;
    size_t capacity;
};


struct shader_manager_private {
    struct shader_manager public_interface;
    shader_cache_t shader_cache;
    WGPUDevice device;
};


void unimplemented(gpu_context_t * context);


void shader_manager_destroy(void) {
    if (!g_shader_manager) return;
    shader_cache_destroy(&g_shader_manager->shader_cache);
    free(g_shader_manager);
    g_shader_manager = NULL;
}
// pipelines are: created from shaders, cached separately, immutable
WGPURenderPipeline create_pipeline(WGPUStringView label, WGPUDevice device, pipeline_desc_t const * desc) {
    // should probably do some more checking here
    return wgpuDeviceCreateRenderPipeline(device, &(WGPURenderPipelineDescriptor){
        .label = label,
        .layout = wgpuDeviceCreatePipelineLayout(device, &(WGPUPipelineLayoutDescriptor){
            .label = toWGPUStringView("pipeline layout"),
            .bindGroupLayoutCount = desc->bind_group_layout_count,
            .bindGroupLayouts = desc->bind_group_layouts,
        }),
        .vertex = {
            .module = desc->vertex->module,
            .entryPoint = toWGPUStringView("vs_main"),
            .bufferCount = desc->vertex_layout_count,
            .buffers = desc->vertex_layouts,
        },
        .primitive = {
            .topology = WGPUPrimitiveTopology_TriangleList,
            .stripIndexFormat = WGPUIndexFormat_Undefined,
            .frontFace = WGPUFrontFace_CCW,
            .cullMode = WGPUCullMode_None,
        },
        .depthStencil = NULL,
        .multisample = {
            .count = 1,
            .mask = (uint32_t)~0,
            .alphaToCoverageEnabled = false,
        },
        .fragment = &(WGPUFragmentState){
            .module = desc->fragment->module,
            .entryPoint = toWGPUStringView("fs_main"),
            .targetCount = 1,
            .targets = &(WGPUColorTargetState){
                .format = desc->color_format,
                .blend = NULL,
                .writeMask = WGPUColorWriteMask_All,
            },
        },
    });
}

shader_t shader_create_from_source(
    WGPUDevice device,
    WGPUShaderStage stage,
    char const * source_code
);

void shader_destroy(shader_t *shader);

shader_t * shader_cache_get_or_create(WGPUDevice device, WGPUShaderStage stage, char const * source_code) {
    
    if(!g_shader_manager) {
        fprintf(stderr, "Shader manager not initialized!\n");
        return NULL;
    }
    uint64_t h = fnv1a_hash(source_code);

    // Search cache
    for (size_t i = 0; i < g_shader_manager->shader_cache.count; ++i) {
        if (g_shader_manager->shader_cache.entries[i].hash == h) {
            return &g_shader_manager->shader_cache.entries[i].shader;
        }
    }
    // Not found, create new shader
    WGPUShaderModule module = wgpuDeviceCreateShaderModule(device, &(WGPUShaderModuleDescriptor){
        .label = toWGPUStringView("Shader Module"),
        .nextInChain = (WGPUChainedStruct*)&(WGPUShaderSourceWGSL){
            .chain = { .sType = WGPUSType_ShaderSourceWGSL, },
            .code = toWGPUStringView(source_code),
        },
    });
    shader_t new_shader = {
        .stage = stage,
        .module = module,
        .hash = h,
    };

    if (g_shader_manager->shader_cache.count >= g_shader_manager->shader_cache.capacity) {
        // Resize cache
        size_t new_capacity = g_shader_manager->shader_cache.capacity == 0 ? 4 : g_shader_manager->shader_cache.capacity * 2;
        g_shader_manager->shader_cache.entries = realloc(g_shader_manager->shader_cache.entries, new_capacity * sizeof(struct shader_cache_entry));
        g_shader_manager->shader_cache.capacity = new_capacity;
    }
    g_shader_manager->shader_cache.entries[g_shader_manager->shader_cache.count++] = (struct shader_cache_entry){
        .hash = h,
        .shader = new_shader,
    };
    return &g_shader_manager->shader_cache.entries[g_shader_manager->shader_cache.count - 1].shader;
}
void shader_cache_destroy(struct shader_cache * cache) {
    if (!cache) return;
    for (size_t i = 0; i < cache->count; ++i) {
        if (cache->entries[i].shader.module) {
            wgpuShaderModuleRelease(cache->entries[i].shader.module);
        }
    }
    free(cache->entries);
    cache->entries = NULL;
    cache->count = 0;
    cache->capacity = 0;
}

uint64_t pipeline_hash(shader_t * vertex, shader_t * fragment, WGPUTextureFormat color_format) {
    uint64_t hash = vertex->hash;
    hash ^= fragment->hash + 0x9e3779b97f4a7c15ULL + (hash << 6) + (hash >> 2);
    hash ^= (uint64_t)color_format + 0x9e3779b97f4a7c15ULL + (hash << 6) + (hash >> 2);
    return hash;
}



WGPUShaderModule shader_get_module(shader_t * shader) {
    return shader->module;
}

shader_t * shader_file_to_module(char const * filename, WGPUDevice device) {
    char * shader_source = malloc(1<<14); // 16 KB buffer
    if (!shader_source) {
        printf("ERROR: Failed to allocate memory for shader source\n");
        return NULL;
    }
    if(!read_shader_file(filename, shader_source, 1<<14)) {
        printf("ERROR: Failed to read shader file: %s\n", filename);
        free(shader_source);
        return NULL;
    }
    shader_t * shader = shader_cache_get_or_create(
        device, 
        WGPUShaderStage_Vertex | WGPUShaderStage_Fragment, 
        shader_source
    );
    free(shader_source);
    return shader;
}

shader_manager_t new_shader_manager(void) {
    if (g_shader_manager) {
        fprintf(stderr, "Shader manager already exists! Returning existing instance.\n");
        return &g_shader_manager->public_interface;
    }
    g_shader_manager = malloc(sizeof(struct shader_manager_private));
    g_shader_manager->shader_cache = (struct shader_cache){0};
    g_shader_manager->public_interface.init_shaders = unimplemented; // to be implemented
    return &g_shader_manager->public_interface;
}

void unimplemented(gpu_context_t * context) {
    (void)context;
    fprintf(stderr, "Shader manager init_shaders function not implemented!\n");
}

void shader_reload(shader_t * shader);