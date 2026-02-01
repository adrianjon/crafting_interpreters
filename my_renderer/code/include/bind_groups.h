#ifndef MY_RENDERER_BIND_GROUPS_H
#define MY_RENDERER_BIND_GROUPS_H

#include <webgpu/webgpu.h>

typedef enum {
    BGL_GLOBAL_UNIFORMS,   // camera, time, etc
    BGL_OBJECT_UNIFORMS,   // model matrix
    BGL_MATERIAL,          // textures, samplers
    BGL_COUNT
} bind_group_layout_id_t;

typedef struct bg_layout_manager * bg_layout_manager_t;
struct bg_layout_manager {
    WGPUBindGroupLayout (*get_layout)(bind_group_layout_id_t id);
    void (*destroy)(void);
};
bg_layout_manager_t new_layout_manager(void);

#endif // MY_RENDERER_BIND_GROUPS_H