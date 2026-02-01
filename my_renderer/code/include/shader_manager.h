#ifndef MY_RENDERER_SHADER_MANAGER_H
#define MY_RENDERER_SHADER_MANAGER_H

#include <webgpu/webgpu.h>
#include "types/primitives/float3.h"
#include "gpu.h"


// basic shader structs
typedef struct vertex_input {
    float3 position;
    float3 normal;
    float3 color;
} vertex_input_t;

typedef struct my_uniforms {
    float4x4 projection;
    float4x4 view;
    float4x4 model;
    float4 color;
    float time;
    float _padding[3]; // Pad to 16-byte alignment (WebGPU requirement)
} my_uniforms_t;

typedef struct shader shader_t;
typedef struct shader_cache shader_cache_t;

struct shader_manager {
    void (*init_shaders)(gpu_context_t * context);
};
typedef struct shader_manager * shader_manager_t;


typedef struct pipeline_desc {
    shader_t * vertex;
    shader_t * fragment;
    WGPUVertexBufferLayout * vertex_layouts;
    uint32_t vertex_layout_count;
    WGPUBindGroupLayout * bind_group_layouts;
    uint32_t bind_group_layout_count;
    WGPUTextureFormat color_format;
} pipeline_desc_t;

typedef struct pipeline_handle {
    shader_t * vertex;
    shader_t * fragment;
    WGPURenderPipeline pipeline;
} pipeline_handle_t;


shader_manager_t new_shader_manager(void);

shader_t * shader_cache_get_or_create(WGPUDevice device, WGPUShaderStage stage, char const * source_code);
void destroy_shader(shader_t * shader);
void shader_cache_destroy(shader_cache_t * cache);

uint64_t pipeline_hash(shader_t * vertex, shader_t * fragment, WGPUTextureFormat color_format);


WGPUShaderModule shader_get_module(shader_t * shader);

shader_t * shader_file_to_module(char const * filename, WGPUDevice device);

#endif // MY_RENDERER_SHADER_MANAGER_H