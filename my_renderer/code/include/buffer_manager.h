#ifndef MY_RENDERER_BUFFER_MANAGER_H
#define MY_RENDERER_BUFFER_MANAGER_H

#include <webgpu/webgpu.h>
#include "gpu.h"

typedef enum {
    BUFFER_VERTEX,
    BUFFER_INDEX,
    BUFFER_UNIFORM,
    BUFFER_COUNT
} buffer_id_t;

typedef struct buffer_manager * buffer_manager_t;
struct buffer_manager {
    WGPUBuffer (*create_buffer)(gpu_context_t * context, WGPUBufferUsage usage, size_t size);
    void (*destroy_buffer)(WGPUBuffer buffer);
    void (*destroy)(void);
    void (*write_buffer)(WGPUQueue queue, buffer_id_t id, size_t offset, const void * data, size_t size);
    WGPUBuffer (*get_buffer)(buffer_id_t id);
    void (*initialize_buffers)(gpu_context_t * context);
};



buffer_manager_t new_buffer_manager(void);

#endif // MY_RENDERER_BUFFER_MANAGER_H