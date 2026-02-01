
#include "pipeline_manager.h"
#include "gpu.h"
#include "types/primitives/float3.h"
#include "utils.h"
#include "shader_manager.h"
#include "bind_groups.h"
#include <webgpu/webgpu.h>
#include <stdlib.h>


static struct pipeline_manager_private * g_pipeline_manager = NULL; // global singleton for simplicity

struct pipeline_cache_entry {
    uint64_t hash;
    WGPURenderPipeline pipeline;
};

struct pipeline_cache {
    struct pipeline_cache_entry * entries;
    size_t count;
    size_t capacity;
};

struct pipeline_manager_private {
    struct pipeline_manager public_interface;
    pipeline_cache_t pipeline_cache;
    WGPURenderPipeline pipelines[PIPELINE_COUNT];
    pipeline_id_t active_pipeline;
};


// Forward declarations
WGPURenderPipeline pipeline_cache_get_or_create(WGPUDevice device, pipeline_config_t * const config);

static void active_pipeline(pipeline_id_t id);
WGPURenderPipeline get_active_pipeline(void);

static void pipeline_manager_destroy() {
    if (!g_pipeline_manager) return;
    for (uint32_t i = 0; i < PIPELINE_COUNT; ++i) {
        if (g_pipeline_manager->pipelines[i]) {
            wgpuRenderPipelineRelease(g_pipeline_manager->pipelines[i]);
        }
    }
    free(g_pipeline_manager);
    g_pipeline_manager = NULL;
    printf("Pipeline manager destroyed.\n");
}
// shader_t * shader_file_to_shader(char const * filename, WGPUDevice device, gfx_context_t * shader_manager);


static void pipeline_manager_init(gpu_context_t * context) {
    
     if (!g_pipeline_manager) {
        fprintf(stderr, "Pipeline manager not created yet!\n");
        return;
    } // already initialized
    

    WGPUDevice device = context->device;
    
    pipeline_config_t configs[PIPELINE_COUNT] = {
        // PIPELINE_3D_PRIMARY configuration - uses the proper shader with uniforms
        {
            .layout = NULL, // Will be created with bind group layouts
            .vertex_shader = shader_file_to_module("demo/shader.wgsl", context->device),
            .fragment_shader = shader_file_to_module("demo/shader.wgsl", context->device),
            .attribute_count = 3,
            .attributes = (WGPUVertexAttribute []){
                {
                    .shaderLocation = 0,
                    .offset = offsetof(vertex_input_t, position),
                    .format = WGPUVertexFormat_Float32x3,
                },
                {
                    .shaderLocation = 1,
                    .offset = offsetof(vertex_input_t, normal),
                    .format = WGPUVertexFormat_Float32x3,
                },
                {
                    .shaderLocation = 2,
                    .offset = offsetof(vertex_input_t, color),
                    .format = WGPUVertexFormat_Float32x3,
                },
            },
            .array_stride = sizeof(vertex_input_t),
            .color_format = context->surface_format,
        }
    };

    for (uint32_t i = 0; i < PIPELINE_COUNT; ++i) {
        printf("Creating pipeline %d (3D_PRIMARY)...\n", i);
        printf("3D Pipeline shaders - vertex: %p, fragment: %p\n", 
               (void*)configs[i].vertex_shader, 
               (void*)configs[i].fragment_shader);
        g_pipeline_manager->pipelines[i] = pipeline_cache_get_or_create(device, &configs[i]);
        printf("Pipeline %d created: %p\n", i, (void*)g_pipeline_manager->pipelines[i]);
    }
    
    printf("Pipeline manager initialized with %d pipelines.\n", PIPELINE_COUNT);

}

WGPURenderPipelineDescriptor create_pipeline_descriptor(pipeline_config_t const * config) {
    // Create pipeline layout
    WGPUPipelineLayout layout = config->layout;
    if (!layout) {
        // For 3D pipeline, use the same bind group layout as the bind groups manager
        if (config->attribute_count == 3) {
            // Create a temporary bind group layout manager to get the layout
            bg_layout_manager_t bg_manager = new_layout_manager(); // This will return existing instance
            if (bg_manager) {
                WGPUBindGroupLayout uniform_layout = bg_manager->get_layout(BGL_GLOBAL_UNIFORMS);
                if (uniform_layout) {
                    layout = wgpuDeviceCreatePipelineLayout(gpu_get_context()->device, &(WGPUPipelineLayoutDescriptor){
                        .bindGroupLayoutCount = 1,
                        .bindGroupLayouts = &uniform_layout,
                    });
                    printf("Created pipeline layout with uniform bind group layout\n");
                }
            }
        }
        
        // Fallback to automatic layout
        if (!layout) {
            layout = wgpuDeviceCreatePipelineLayout(gpu_get_context()->device, &(WGPUPipelineLayoutDescriptor){
                .bindGroupLayoutCount = 0,
                .bindGroupLayouts = NULL,
            });
            printf("Created pipeline layout with no bind groups\\n");
        }
    }
    
    return (WGPURenderPipelineDescriptor){
        .layout = layout,
        .vertex.bufferCount = 1,
        .vertex.buffers = &(WGPUVertexBufferLayout){
            .attributeCount = config->attribute_count,
            .attributes = config->attributes,
            .arrayStride = config->array_stride,
            .stepMode = WGPUVertexStepMode_Vertex,
        },
        .vertex.module = shader_get_module(config->vertex_shader),
        .vertex.entryPoint = toWGPUStringView("vs_main"),
        .primitive.topology = WGPUPrimitiveTopology_TriangleList,
        .primitive.frontFace = WGPUFrontFace_CCW,
        .primitive.cullMode = WGPUCullMode_None,
        .fragment = &(WGPUFragmentState){
            .module = shader_get_module(config->fragment_shader),
            .entryPoint = toWGPUStringView("fs_main"),
            .targetCount = 1,
            .targets = &(WGPUColorTargetState){
                .format = config->color_format,
                .blend = &(WGPUBlendState){
                    .color.srcFactor = WGPUBlendFactor_SrcAlpha,
                    .color.dstFactor = WGPUBlendFactor_OneMinusSrcAlpha,
                    .color.operation = WGPUBlendOperation_Add,
                    .alpha.srcFactor = WGPUBlendFactor_Zero,
                    .alpha.dstFactor = WGPUBlendFactor_One,
                    .alpha.operation = WGPUBlendOperation_Add,
                },
                .writeMask = WGPUColorWriteMask_All,
            },
        },
        .multisample.count = 1,
        .multisample.mask = ~0,
    };
}


pipeline_manager_t new_pipeline_manager() {
    if (g_pipeline_manager) {
        fprintf(stderr, "Pipeline manager already exists! Returning existing instance.\n");
        return &g_pipeline_manager->public_interface;
    }
    g_pipeline_manager = malloc(sizeof(struct pipeline_manager_private));
    g_pipeline_manager->pipeline_cache = (struct pipeline_cache){0};
    g_pipeline_manager->public_interface.init_pipelines = pipeline_manager_init;
    g_pipeline_manager->public_interface.destroy_pipelines = pipeline_manager_destroy;
    g_pipeline_manager->public_interface.set_active_pipeline = active_pipeline;
    g_pipeline_manager->public_interface.get_active_pipeline = get_active_pipeline;
    return &g_pipeline_manager->public_interface;
}


WGPURenderPipeline pipeline_cache_get_or_create(WGPUDevice device, pipeline_config_t * const config) {
    uint64_t hash = pipeline_hash(config->vertex_shader, config->fragment_shader, config->color_format);


    for (size_t i = 0; i < g_pipeline_manager->pipeline_cache.count; ++i) {
        if (g_pipeline_manager->pipeline_cache.entries[i].hash == hash) {
            return g_pipeline_manager->pipeline_cache.entries[i].pipeline;
        }
    }

    WGPURenderPipelineDescriptor desc = create_pipeline_descriptor(config);
    WGPURenderPipeline pipeline = wgpuDeviceCreateRenderPipeline(device, &desc);

    
    if (g_pipeline_manager->pipeline_cache.count >= g_pipeline_manager->pipeline_cache.capacity) {
        
        
        size_t new_capacity = g_pipeline_manager->pipeline_cache.capacity == 0 ? 4 : g_pipeline_manager->pipeline_cache.capacity * 2;

        g_pipeline_manager->pipeline_cache.entries = realloc(g_pipeline_manager->pipeline_cache.entries, new_capacity * sizeof(struct pipeline_cache_entry));
        g_pipeline_manager->pipeline_cache.capacity = new_capacity;
    }
    
    g_pipeline_manager->pipeline_cache.entries[g_pipeline_manager->pipeline_cache.count++] = (struct pipeline_cache_entry){
        .hash = hash,
        .pipeline = pipeline,
    };

    return pipeline;
}
void pipeline_cache_destroy(struct pipeline_cache *cache) {
    if (!cache) return;
    for (size_t i = 0; i < cache->count; i++) {
        if (cache->entries[i].pipeline) {
            wgpuRenderPipelineRelease(cache->entries[i].pipeline);
        }
    }
    free(cache->entries);
    cache->entries = NULL;
    cache->count = 0;
    cache->capacity = 0;
}

pipeline_handle_t create_pipeline_handle(WGPUDevice device, char const * vertex_source, char const * fragment_source, WGPUTextureFormat color_format) {
    shader_t * vs = shader_cache_get_or_create(device, WGPUShaderStage_Vertex, vertex_source);
    shader_t * fs = shader_cache_get_or_create(device, WGPUShaderStage_Fragment, fragment_source);

    pipeline_config_t config = {
        .vertex_shader = vs,
        .fragment_shader = fs,
        .color_format = color_format,
        .attributes = NULL,
        .attribute_count = 0,
        .array_stride = 0,
    };

    WGPURenderPipeline pipeline = pipeline_cache_get_or_create(device, &config);

    return (pipeline_handle_t){
        .vertex = vs,
        .fragment = fs,
        .pipeline = pipeline,
    };
}

WGPURenderPipeline pipeline_get(pipeline_id_t id) {
    if (!g_pipeline_manager) {
        fprintf(stderr, "Pipeline manager not initialized!\n");
        return NULL;
    }
    if (id >= PIPELINE_COUNT) {
        fprintf(stderr, "Invalid pipeline ID: %d\n", id);
        return NULL;
    }
    return g_pipeline_manager->pipelines[id];
}

static void active_pipeline(pipeline_id_t id) {
    if (!g_pipeline_manager) {
        fprintf(stderr, "Pipeline manager not initialized!\n");
        return;
    }
    if (id >= PIPELINE_COUNT) {
        fprintf(stderr, "Invalid pipeline ID: %d\n", id);
        return;
    }
    g_pipeline_manager->active_pipeline = id;
}

WGPURenderPipeline get_active_pipeline(void) {
    if (!g_pipeline_manager) {
        fprintf(stderr, "Pipeline manager not initialized!\n");
        return NULL;
    }
    return g_pipeline_manager->pipelines[g_pipeline_manager->active_pipeline];
}