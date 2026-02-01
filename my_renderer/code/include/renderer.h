#ifndef MY_RENDERER_RENDERER_H
#define MY_RENDERER_RENDERER_H

#include "utils.h"

typedef struct renderer renderer_t;

typedef struct render_context {
    struct render_context * (*renderer_draw_triangle)(struct render_context * ctx, float const vertices[6], float const color[4]);
    struct render_context * (*renderer_end_frame)(struct render_context * ctx, WGPUTextureView current_view);
} render_context_t;

void renderer_init(void);
void renderer_begin_frame(WGPUTextureView current_view, double time);
void renderer_draw_triangle(float const vertices[6], float const color[4]);
void renderer_draw_mesh(mesh_t * mesh);
void renderer_end_frame(WGPUTextureView current_view);

render_context_t * renderer_begin_frame2(render_context_t * ctx, WGPUTextureView current_view);

void renderer_configure_pipeline(void);

renderer_t * renderer_get_renderer(void);

void extra_render_init_all(void) ;
#endif // MY_RENDERER_RENDERER_H