
#include "../include/utils.h"
#include "types/primitives/float3.h"
#include <string.h>
#include <stdio.h>

WGPUStringView toWGPUStringView(const char* str) {
    WGPUStringView view;
    view.data = str;
    view.length = str ? strlen(str) : 0;
    return view;
}

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
EMSCRIPTEN_KEEPALIVE
#endif
void set_canvas_size(uint32_t width, uint32_t height) {
    // printf("Setting canvas size to %ux%u\n", width, height);
    // printf("Not implemented: resizing canvas in WebGPU context\n");   
    (void)width;
    (void)height; 
}

uint64_t fnv1a_hash(char const * str) { // assume null-terminated
    uint64_t hash = 14695981039346656037ULL; // FNV offset basis
    uint64_t const fnv_prime = 1099511628211ULL; // FNV prime
    for (size_t i = 0; str[i]; ++i) {
        hash ^= (uint64_t)(unsigned char)str[i];
        hash *= fnv_prime;
    }
    return hash;
}
struct dynamic_array_float3 {
    float3 * items;
    size_t count;
    size_t capacity;
};
struct dynamic_array_float3 dynamic_array_append(struct dynamic_array_float3 * array, float3 item) {
    if (array->count >= array->capacity) {
        size_t new_capacity = array->capacity == 0 ? 4 : array->capacity * 2;
        array->items = realloc(array->items, new_capacity * sizeof(float3));
        array->capacity = new_capacity;
    }
        array->items[array->count++] = item;
        return *array;
}
void dynamic_array_free(struct dynamic_array_float3 * array) {
    if (!array) return;

    free(array->items);
    array->items = NULL;
    array->count = 0;
    array->capacity = 0;
}
struct dynamic_array_uint32 {
    uint32_t * items;
    size_t count;
    size_t capacity;
};
struct dynamic_array_uint32 dynamic_array_append_uint32(struct dynamic_array_uint32 * array, uint32_t item) {
    if (array->count >= array->capacity) {
        size_t new_capacity = array->capacity == 0 ? 4 : array->capacity * 2;
        array->items = realloc(array->items, new_capacity * sizeof(uint32_t));
        array->capacity = new_capacity;
    }
        array->items[array->count++] = item;
        return *array;
}
void dynamic_array_free_uint32(struct dynamic_array_uint32 * array) {
    if (!array) return;

    free(array->items);
    array->items = NULL;
    array->count = 0;
    array->capacity = 0;
}
struct dynamic_array_vertex {
    struct vertex * items;
    size_t count;
    size_t capacity;
};
struct dynamic_array_vertex dynamic_array_append_vertex(struct dynamic_array_vertex * array, struct vertex item) {
    if (array->count >= array->capacity) {
        size_t new_capacity = array->capacity == 0 ? 4 : array->capacity * 2;
        array->items = realloc(array->items, new_capacity * sizeof(struct vertex));
        array->capacity = new_capacity;
    }
        array->items[array->count++] = item;
        return *array;
}
void dynamic_array_free_vertex(struct dynamic_array_vertex * array) {
    if (!array) return;

    free(array->items);
    array->items = NULL;
    array->count = 0;
    array->capacity = 0;
}
mesh_t load_obj_file(const char* filename) {
    FILE * file = fopen(filename, "r");
    if (!file) {
        printf("Failed to open OBJ file: %s\n", filename);
        return (mesh_t){0};
    }
    struct dynamic_array_float3 obj_vertices = {0};
    struct dynamic_array_float3 obj_normals = {0};
    //struct dynamic_array_float2 texcoords = {0};

    // GPU mesh data
    struct dynamic_array_vertex unique_vertices = {0};
    struct dynamic_array_uint32 vertex_indices = {0};

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "v ", 2) == 0) {
            float3 vertex;
            sscanf(line, "v %f %f %f", &vertex.x, &vertex.y, &vertex.z);
            dynamic_array_append(&obj_vertices, vertex);
        }
        else if (strncmp(line, "vn ", 3) == 0) {
            float3 normal;
            sscanf(line, "vn %f %f %f", &normal.x, &normal.y, &normal.z);
            dynamic_array_append(&obj_normals, normal);
        }
        else if (strncmp(line, "f ", 2) == 0) {
            // Handle face definitions
            uint32_t v1, v2, v3;
            uint32_t n1, n2, n3;
            int result = sscanf(line, "f %u//%u %u//%u %u//%u",
                   &v1, &n1,
                   &v2, &n2,
                   &v3, &n3);
            
            if (result != 6) {
                printf("Unsupported face format in OBJ file: %s\n", line);
                continue;
            }

            struct vertex vertices[3] = {
                { obj_vertices.items[v1 - 1], obj_normals.items[n1 - 1] },
                { obj_vertices.items[v2 - 1], obj_normals.items[n2 - 1] },
                { obj_vertices.items[v3 - 1], obj_normals.items[n3 - 1] },
            };

            for (int i = 0; i < 3; i++) {
                uint32_t vertex_index = UINT32_MAX;
                for (size_t j = 0; j < unique_vertices.count; j++) {
                   struct vertex * existing = &unique_vertices.items[j];
                   if (memcmp(existing, &vertices[i], sizeof(struct vertex)) == 0) {
                       vertex_index = (uint32_t)j;
                       break;
                   }
                }
                if (vertex_index == UINT32_MAX) {
                    dynamic_array_append_vertex(&unique_vertices, vertices[i]);
                    vertex_index = (uint32_t)(unique_vertices.count - 1);
                }
                dynamic_array_append_uint32(&vertex_indices, vertex_index);
            }
        }

    }
    

    dynamic_array_free(&obj_vertices);
    dynamic_array_free(&obj_normals);

    printf("Loaded OBJ file: %s\n", filename);
    printf("  Unique vertices: %zu\n", unique_vertices.count);
    printf("  Indices: %zu\n", vertex_indices.count);

    printf("First 5 vertices:\n");
    for (size_t i = 0; i < unique_vertices.count && i < 5; i++) {
       // char buffer[64];
        printf("Vertex %zu:\n", i);
        float3_print(unique_vertices.items[i].position);
        float3_print(unique_vertices.items[i].normal);

    }

    fclose(file);
    return (mesh_t){
        .vertices = unique_vertices.items,
        .indices = vertex_indices.items,
        .vertex_count = (uint32_t)unique_vertices.count,
        .index_count = (uint32_t)vertex_indices.count
    };
}

bool read_shader_file(char const * filename, char * buffer, size_t buffer_size) {
    if (!filename || !buffer || buffer_size == 0) {
        fprintf(stderr, "Invalid arguments to read_shader_file\n");
        return false;
    }
    FILE * file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Failed to open shader file: %s\n", filename);
        return false;
    }
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    if (file_size >= (long)buffer_size) {
        fprintf(stderr, "Shader file too large for buffer: %s\n", filename);
        fclose(file);
        return false;
    }
    fread(buffer, 1, file_size, file);
    buffer[file_size] = '\0';
    fclose(file);
    return true;
}


WGPUShaderModule load_shader_module(WGPUDevice device, const char* filename) {
    if (!device || !filename) {
        fprintf(stderr, "Invalid arguments to load_shader_module\n");
        return NULL;
    }

    // Read shader source code from file
    // get file size
    FILE * file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Failed to open shader file: %s\n", filename);
        return NULL;
    }
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    fclose(file);
    if (file_size <= 0 || file_size > 65536) {
        fprintf(stderr, "Shader file size invalid or too large: %s\n", filename);
        return NULL;
    }
    char * shader_code = (char *)malloc(file_size + 1);

    if (!read_shader_file(filename, shader_code, file_size + 1)) {
        fprintf(stderr, "Failed to read shader file: %s\n", filename);
        free(shader_code);
        return NULL;
    }

    WGPUShaderModuleDescriptor shader_desc = {
        .label = toWGPUStringView(filename),
        .nextInChain = (WGPUChainedStruct*)&(WGPUShaderSourceWGSL){
            .chain = { .sType = WGPUSType_ShaderSourceWGSL, },
            .code = toWGPUStringView(shader_code),
        },
    };

    WGPUShaderModule shader_module = wgpuDeviceCreateShaderModule(device, &shader_desc);
    if (!shader_module) {
        fprintf(stderr, "Failed to create shader module from file: %s\n", filename);
        return NULL;
    }

    printf("Loaded shader module: %s\n", filename);

    free(shader_code);
    return shader_module;
}
void set_default_limits(WGPULimits * limits){
    limits->nextInChain = NULL;
    limits->maxTextureDimension1D = WGPU_LIMIT_U32_UNDEFINED;
	limits->maxTextureDimension2D = WGPU_LIMIT_U32_UNDEFINED;
	limits->maxTextureDimension3D = WGPU_LIMIT_U32_UNDEFINED;
	limits->maxTextureArrayLayers = WGPU_LIMIT_U32_UNDEFINED;
	limits->maxBindGroups = WGPU_LIMIT_U32_UNDEFINED;
	limits->maxBindGroupsPlusVertexBuffers = WGPU_LIMIT_U32_UNDEFINED;
	limits->maxBindingsPerBindGroup = WGPU_LIMIT_U32_UNDEFINED;
	limits->maxDynamicUniformBuffersPerPipelineLayout = WGPU_LIMIT_U32_UNDEFINED;
	limits->maxDynamicStorageBuffersPerPipelineLayout = WGPU_LIMIT_U32_UNDEFINED;
	limits->maxSampledTexturesPerShaderStage = WGPU_LIMIT_U32_UNDEFINED;
	limits->maxSamplersPerShaderStage = WGPU_LIMIT_U32_UNDEFINED;
	limits->maxStorageBuffersPerShaderStage = WGPU_LIMIT_U32_UNDEFINED;
	limits->maxStorageTexturesPerShaderStage = WGPU_LIMIT_U32_UNDEFINED;
	limits->maxUniformBuffersPerShaderStage = WGPU_LIMIT_U32_UNDEFINED;
	limits->maxUniformBufferBindingSize = WGPU_LIMIT_U64_UNDEFINED;
	limits->maxStorageBufferBindingSize = WGPU_LIMIT_U64_UNDEFINED;
	limits->minUniformBufferOffsetAlignment = WGPU_LIMIT_U32_UNDEFINED;
	limits->minStorageBufferOffsetAlignment = WGPU_LIMIT_U32_UNDEFINED;
	limits->maxVertexBuffers = WGPU_LIMIT_U32_UNDEFINED;
	limits->maxBufferSize = WGPU_LIMIT_U64_UNDEFINED;
	limits->maxVertexAttributes = WGPU_LIMIT_U32_UNDEFINED;
	limits->maxVertexBufferArrayStride = WGPU_LIMIT_U32_UNDEFINED;
	limits->maxInterStageShaderVariables = WGPU_LIMIT_U32_UNDEFINED;
	limits->maxColorAttachments = WGPU_LIMIT_U32_UNDEFINED;
	limits->maxColorAttachmentBytesPerSample = WGPU_LIMIT_U32_UNDEFINED;
	limits->maxComputeWorkgroupStorageSize = WGPU_LIMIT_U32_UNDEFINED;
	limits->maxComputeInvocationsPerWorkgroup = WGPU_LIMIT_U32_UNDEFINED;
	limits->maxComputeWorkgroupSizeX = WGPU_LIMIT_U32_UNDEFINED;
	limits->maxComputeWorkgroupSizeY = WGPU_LIMIT_U32_UNDEFINED;
	limits->maxComputeWorkgroupSizeZ = WGPU_LIMIT_U32_UNDEFINED;
	limits->maxComputeWorkgroupsPerDimension = WGPU_LIMIT_U32_UNDEFINED;
    limits->maxImmediateSize = WGPU_LIMIT_U32_UNDEFINED;
}

void set_default_bindings_layout(WGPUBindGroupLayoutEntry * binding_layout) {
    binding_layout->buffer.nextInChain = NULL;
	binding_layout->buffer.type = WGPUBufferBindingType_Undefined;
	binding_layout->buffer.hasDynamicOffset = false;

	binding_layout->sampler.nextInChain = NULL;
	binding_layout->sampler.type = WGPUSamplerBindingType_Undefined;

	binding_layout->storageTexture.nextInChain = NULL;
	binding_layout->storageTexture.access = WGPUStorageTextureAccess_Undefined;
	binding_layout->storageTexture.format = WGPUTextureFormat_Undefined;
	binding_layout->storageTexture.viewDimension = WGPUTextureViewDimension_Undefined;

	binding_layout->texture.nextInChain = NULL;
	binding_layout->texture.multisampled = false;
	binding_layout->texture.sampleType = WGPUTextureSampleType_Undefined;
	binding_layout->texture.viewDimension = WGPUTextureViewDimension_Undefined;
}
int load_geometry_from_obj(const char* filename, float ** point_data, uint32_t * out_point_count, uint16_t ** index_data, uint32_t * out_index_count) {
    FILE * file = fopen(filename, "r");
    if (!file) {
        printf("Failed to open OBJ file: %s\n", filename);
        return -1;
    }

    struct vertex_input {
        float position[3];
        float texcoord[2];
        float normal[3];
    };

    struct vertex_input * vertices = NULL;
    uint32_t vertex_capacity = 0;
    uint32_t vertex_count = 0;


    float * positions = NULL;
    uint32_t point_capacity = 0;
    uint32_t point_count = 0;
    
    float * texcoords = NULL;
    uint32_t texcoord_capacity = 0;
    uint32_t texcoord_count = 0;

    float * normals = NULL;
    uint32_t normal_capacity = 0;
    uint32_t normal_count = 0;

    uint16_t * indices = NULL;
    uint32_t index_capacity = 0;
    uint32_t index_count = 0;
    
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "v ", 2) == 0) {
            if (point_count >= point_capacity) {
                point_capacity = point_capacity == 0 ? 4 : point_capacity * 2;
                positions = realloc(positions, point_capacity * 3 * sizeof(float));
            }
            float x, y, z;
            sscanf(line, "v %f %f %f", &x, &y, &z);
            positions[point_count * 3 + 0] = x;
            positions[point_count * 3 + 1] = y;
            positions[point_count * 3 + 2] = z;
            point_count++;
            continue;
        } else if (strncmp(line, "vt ", 3) == 0) {
            if (texcoord_count >= texcoord_capacity) {
                texcoord_capacity = texcoord_capacity == 0 ? 4 : texcoord_capacity * 2;
                texcoords = realloc(texcoords, texcoord_capacity * 2 * sizeof(float));
            }
            float u, v;
            sscanf(line, "vt %f %f", &u, &v);
            texcoords[texcoord_count * 2 + 0] = u;
            texcoords[texcoord_count * 2 + 1] = v;
            texcoord_count++;
            continue;
        } else if (strncmp(line, "vn ", 3) == 0) {
            if (normal_count >= normal_capacity) {
                normal_capacity = normal_capacity == 0 ? 4 : normal_capacity * 2;
                normals = realloc(normals, normal_capacity * 3 * sizeof(float));
            }
            float nx, ny, nz;
            sscanf(line, "vn %f %f %f", &nx, &ny, &nz);
            normals[normal_count * 3 + 0] = nx;
            normals[normal_count * 3 + 1] = ny;
            normals[normal_count * 3 + 2] = nz;
            normal_count++;
            continue;
        } else if (strncmp(line, "f ", 2) == 0) {
            if (vertex_count + 3 > vertex_capacity) {
                vertex_capacity = vertex_capacity == 0 ? 6 : vertex_capacity * 2;
                vertices = realloc(vertices, vertex_capacity * sizeof(struct vertex_input));
            }
            uint16_t v1, v2, v3;
            uint16_t n1, n2, n3;
            uint16_t t1, t2, t3;
            // try vertex/texcoord/normal first
            int count = sscanf(line, "f %hu/%hu/%hu %hu/%hu/%hu %hu/%hu/%hu", &v1, &t1, &n1, &v2, &t2, &n2, &v3, &t3, &n3);
            //printf("Parsed face with %d elements\n", count);
            if (count == 9) {
                for (int i = 0; i < 3; i++) {
                    uint16_t v_idx = (i == 0) ? v1 : (i == 1) ? v2 : v3;
                    uint16_t t_idx = (i == 0) ? t1 : (i == 1) ? t2 : t3;
                    uint16_t n_idx = (i == 0) ? n1 : (i == 1) ? n2 : n3;

                    vertices[vertex_count + i].position[0] = positions[(v_idx - 1) * 3 + 0];
                    vertices[vertex_count + i].position[1] = positions[(v_idx - 1) * 3 + 1];
                    vertices[vertex_count + i].position[2] = positions[(v_idx - 1) * 3 + 2];
                    vertices[vertex_count + i].texcoord[0] = texcoords[(t_idx - 1) * 2 + 0];
                    vertices[vertex_count + i].texcoord[1] = texcoords[(t_idx - 1) * 2 + 1];
                    vertices[vertex_count + i].normal[0] = normals[(n_idx - 1) * 3 + 0];
                    vertices[vertex_count + i].normal[1] = normals[(n_idx - 1) * 3 + 1];
                    vertices[vertex_count + i].normal[2] = normals[(n_idx - 1) * 3 + 2];
                }
                
                goto append_indices;
            }
            count = sscanf(line, "f %hu//%hu %hu//%hu %hu//%hu", &v1, &n1, &v2, &n2, &v3, &n3);
            //printf("Parsed face with %d elements (no texcoords)\n", count);
            if (count == 6) {
                for (int i = 0; i < 3; i++) {
                    uint16_t v_idx = (i == 0) ? v1 : (i == 1) ? v2 : v3;
                    uint16_t n_idx = (i == 0) ? n1 : (i == 1) ? n2 : n3;

                    if (v_idx < 1 || v_idx > point_count || n_idx < 1 || n_idx > normal_count) {
                        printf("Invalid vertex or normal index in face definition: %s\n", line);
                        continue;
                    }

                    vertices[vertex_count + i].position[0] = positions[(v_idx - 1) * 3 + 0];
                    vertices[vertex_count + i].position[1] = positions[(v_idx - 1) * 3 + 1];
                    vertices[vertex_count + i].position[2] = positions[(v_idx - 1) * 3 + 2];
                    vertices[vertex_count + i].texcoord[0] = 0.0f;
                    vertices[vertex_count + i].texcoord[1] = 0.0f;
                    vertices[vertex_count + i].normal[0] = normals[(n_idx - 1) * 3 + 0];
                    vertices[vertex_count + i].normal[1] = normals[(n_idx - 1) * 3 + 1];
                    vertices[vertex_count + i].normal[2] = normals[(n_idx - 1) * 3 + 2];
                }
                
                goto append_indices;
            }
            count = sscanf(line, "f %hu// %hu// %hu//", &v1, &v2, &v3);
            //printf("Parsed face with %d elements (vertex only)\n", count);
            if (count == 3) {
                for (int i = 0; i < 3; i++) {
                    uint16_t v_idx = (i == 0) ? v1 : (i == 1) ? v2 : v3;

                    vertices[vertex_count + i].position[0] = positions[(v_idx - 1) * 3 + 0];
                    vertices[vertex_count + i].position[1] = positions[(v_idx - 1) * 3 + 1];
                    vertices[vertex_count + i].position[2] = positions[(v_idx - 1) * 3 + 2];
                    vertices[vertex_count + i].texcoord[0] = 0.0f;
                    vertices[vertex_count + i].texcoord[1] = 0.0f;
                    vertices[vertex_count + i].normal[0] = 0.0f;
                    vertices[vertex_count + i].normal[1] = 0.0f;
                    vertices[vertex_count + i].normal[2] = 0.0f;
                }
                goto append_indices;
            }
        } else {
            printf("Ignoring line: %s", line);
            continue; // ignore other lines
        }
        printf("Failed to parse face line: %s\n", line);
        continue;
append_indices:
        if (index_count + 3 > index_capacity) {
            index_capacity = index_capacity == 0 ? 6 : index_capacity * 2;
            indices = realloc(indices, index_capacity * sizeof(uint16_t));
        }
        indices[index_count++] = vertex_count++;
        indices[index_count++] = vertex_count++;
        indices[index_count++] = vertex_count++;           
    }
    fclose(file);


    free(positions);
    free(texcoords);
    free(normals);
    *point_data = (float *)vertices; // cast to float pointer for output (8 floats per vertex) TODO: fix this, change function signature to return struct vertex_input **
    printf("Loaded OBJ file: %s\n", filename);
    printf("  Total points: %u\n", point_count);
    printf("  Total indices: %u\n", index_count);
    *out_point_count = vertex_count;
    *index_data = indices;
    *out_index_count = index_count;
    return 0;
}
void free_geometry_data(float * point_data, uint16_t * index_data) {
    free(point_data);
    free(index_data);
}