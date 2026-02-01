#ifndef MY_RENDERER_UTILS_H
#define MY_RENDERER_UTILS_H

#include <stdbool.h>
#include <webgpu/webgpu.h>
#include "types/primitives/float3.h"
//#include "shader_manager.h"

// typedef struct {
//     vertex_input_t * items;
//     size_t count;
//     size_t capacity;
// } dynamic_array_vertex_t;

struct vertex {
    float3 position;
    float3 normal;
};
typedef struct mesh {
    struct vertex * vertices;
    uint32_t * indices;
    uint32_t vertex_count;
    uint32_t index_count;
} mesh_t;

WGPUStringView toWGPUStringView(const char* str);

uint64_t fnv1a_hash(char const * str); // assume null-terminated
mesh_t load_obj_file(const char* filename);
bool read_shader_file(char const * filename, char * buffer, size_t buffer_size);

WGPUShaderModule load_shader_module(WGPUDevice device, const char* filename);
void set_default_limits(WGPULimits * limits);
void set_default_bindings_layout(WGPUBindGroupLayoutEntry * binding_layout);
int load_geometry_from_obj(const char* filename, float ** point_data, uint32_t * out_point_count, uint16_t ** index_data, uint32_t * out_index_count);
void free_geometry_data(float * point_data, uint16_t * index_data);
// void dynamic_array_vertex_append(dynamic_array_vertex_t * array, vertex_input_t item);
// void dynamic_array_vertex_free(dynamic_array_vertex_t * array);

#endif // MY_RENDERER_UTILS_H