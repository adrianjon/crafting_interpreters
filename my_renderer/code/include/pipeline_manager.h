#ifndef MY_RENDERER_PIPELINE_MANAGER_H
#define MY_RENDERER_PIPELINE_MANAGER_H

#include <webgpu/webgpu.h>
#include "shader_manager.h"

typedef enum {
    PIPELINE_3D_PRIMARY,
    PIPELINE_COUNT
} pipeline_id_t;

struct pipeline_manager {
    void (*init_pipelines)(gpu_context_t * context);
    void (*destroy_pipelines)(void);
    void (*set_active_pipeline)(pipeline_id_t id);
    WGPURenderPipeline (*get_active_pipeline)(void);
};

typedef struct pipeline_manager * pipeline_manager_t;
typedef struct pipeline_cache pipeline_cache_t;

typedef struct pipeline_config {
    WGPUPipelineLayout layout; // optional
    shader_t * vertex_shader;
    shader_t * fragment_shader;
    WGPUTextureFormat color_format;
    WGPUVertexAttribute * attributes;
    uint32_t attribute_count;
    uint64_t array_stride;
} pipeline_config_t;

pipeline_manager_t new_pipeline_manager(void);

WGPURenderPipelineDescriptor create_pipeline_descriptor(pipeline_config_t const * config);

void pipeline_cache_destroy(pipeline_cache_t * cache);

pipeline_handle_t create_pipeline_handle(WGPUDevice device, char const * vertex_source, char const * fragment_source, WGPUTextureFormat color_format);

WGPURenderPipeline pipeline_get(pipeline_id_t id);

#endif // MY_RENDERER_PIPELINE_MANAGER_H


//// main.c

//pipeline_manager_t pipeline_manager = new_pipeline_manager(); // prints warning if already initialized