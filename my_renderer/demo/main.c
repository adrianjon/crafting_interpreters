#include <webgpu/webgpu.h>
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#include <stdio.h>
#include <math.h>
#include "../code/include/renderer.h"
#include "../code/include/gpu.h"
#include "types/primitives/float3.h"
#include "utils.h"

#include "pipeline_manager.h"
#include "buffer_manager.h"
#include "bind_groups.h"

// Example canvas element for web target
static char const CANVAS_ID[] = "#canvas";

static int dragging = 0;
static int last_mouse_x = 0;
static int last_mouse_y = 0;
static float2 offset = {{0.0f, 0.0f}};
static float scale = 1.0f;
EM_BOOL mouse_callback(int eventType, const EmscriptenMouseEvent *e, void *userData) {
    (void)userData; // Unused
    if (eventType == EMSCRIPTEN_EVENT_MOUSEDOWN) {
        dragging = 1;
        last_mouse_x = e->clientX;
        last_mouse_y = e->clientY;
        printf("Mouse button %u down at (%d, %d)\n", e->button, e->clientX, e->clientY);
    }
    else if (eventType == EMSCRIPTEN_EVENT_MOUSEMOVE && dragging) {
        int delta_x = e->clientX - last_mouse_x;
        int delta_y = e->clientY - last_mouse_y;
        float translation_factor_x = 2.0f/(1543.0f * scale); // Adjust sensitivity here
        float translation_factor_y = 2.0f/(1056.0f * scale);  // Adjust sensitivity here
        offset.x += delta_x * translation_factor_x;
        offset.y -= delta_y * translation_factor_y;
        last_mouse_x = e->clientX;
        last_mouse_y = e->clientY;
       
    }
    else if (eventType == EMSCRIPTEN_EVENT_MOUSEUP) {
        dragging = 0;
    }
    return EM_TRUE; // Handled

}
EM_BOOL scroll_callback(int eventType, const EmscriptenWheelEvent *e, void *userData) {
    (void)userData; // Unused
    if (eventType == EMSCRIPTEN_EVENT_WHEEL) {
        float zoom_factor = 1.3f; // Zoom sensitivity
        if (e->deltaY < 0) {
            scale *= zoom_factor; // Zoom in
        } else {
            scale /= zoom_factor; // Zoom out
        }
        // Clamp scale to reasonable range
        if (scale < 0.1f) scale = 0.1f;
        if (scale > 10.0f) scale = 10.0f;
    }
    return EM_TRUE; // Handled
}
// typedef struct triangle {
//     float vertices[6]; // 3 vertices (x,y)
//     float color[4];    // RGBA color
// } triangle_t;

EM_JS(void, gpu_present_frame, (), {
    // Instruct Emscripten to present the current frame
    
    Module.gpuContext.presentFrame();
});

static mesh_t mesh;
void loop(void) {
    double time = emscripten_get_now() / 1000.0;
    (void)time;
    WGPUTextureView current_view = gpu_begin_frame();
    renderer_begin_frame(current_view, time);

    // renderer_draw_triangle((float[]){
    //     (0.0f + offset.x) * scale, (0.5f + offset.y) * scale, 
    //     (-0.5f + offset.x) * scale, (-0.5f + offset.y) * scale, 
    //     (0.5f + offset.x) * scale, (-0.5f + offset.y) * scale
    // }, (float[]){1.0f, 1.0f, 0.0f, 1.0f});
    // renderer_draw_triangle((float[]){
    //     (0.0f + offset.x) * scale, (0.5f + offset.y) * scale, 
    //     (0.5f + offset.x) * scale, (-0.5f + offset.y) * scale, 
    //     (1.0f + offset.x) * scale, (0.5f + offset.y) * scale
    // }, (float[]){0.0f, 0.5f, 1.0f, 1.0f});
    // renderer_draw_triangle((float[]){
    //     (0.0f + offset.x) * scale, (0.5f + offset.y) * scale, 
    //     (-0.5f + offset.x) * scale, (-0.5f + offset.y) * scale, 
    //     (-1.0f + offset.x) * scale, (0.5f + offset.y) * scale
    // }, (float[]){0.0f, 0.5f, 1.0f, 1.0f});
    // renderer_draw_triangle((float[]){
    //     (-0.5f + offset.x) * scale, (0.5f + offset.y) * scale, 
    //     (0.5f + offset.x) * scale, (0.5f + offset.y) * scale, 
    //     (0.0f + offset.x) * scale, (1.0f + offset.y) * scale
    // }, (float[]){1.0f, 0.0f, 0.0f, 1.0f});
    //renderer_draw_mesh(&mesh);
    renderer_end_frame(current_view);
    
    gpu_present_frame();
}

int main(void) {

    // GPU INIT (WGPUDevice, WGPUSurface, etc.)
    // RENDERER INIT (using GPU context from above to create pipelines, buffers, etc.)


    //mesh = load_obj_file("demo/dragon.obj"); // Load any embedded object files for shaders, models, etc.
    printf("=== WebGPU Renderer Demo ===\n");
    // Get canvas dimensions for initialization
    double canvas_width, canvas_height;
    emscripten_get_element_css_size(CANVAS_ID, &canvas_width, &canvas_height);
    
    // Create GPU initialization configuration
    gpu_init_config_t gpu_config = gpu_create_default_config(
        NULL, // Window handle not needed for Emscripten/web target
        (uint32_t)canvas_width, 
        (uint32_t)canvas_height
    );
    
    // Configure for web-specific settings
    gpu_config.preferred_format = WGPUTextureFormat_BGRA8Unorm; // Common web format
    gpu_config.present_mode = WGPUPresentMode_Fifo;             // V-sync for smooth animation
    gpu_config.enable_depth_testing = true;                    // Enable 3D depth testing
    gpu_config.enable_debug_labels = true;                     // Helpful for debugging
    
    // Initialize GPU context
    if (!gpu_init(&gpu_config)) {
        printf("ERROR: Failed to initialize GPU context: %s\n", gpu_get_last_error());
        return -1;
    }
    

    // printf("✓ GPU initialization successful!\n");
    // printf("  Swapchain: %dx%d\n", (int)canvas_width, (int)canvas_height);
    // printf("  Depth testing: %s\n", gpu_config.enable_depth_testing ? "enabled" : "disabled");
    
    // Initialize renderer (which will use the GPU context)
    // renderer_init();
    extra_render_init_all();

    
    //renderer_configure_pipeline("shader.wgsl");


    
        // setup mouse drag to translate model
    emscripten_set_mousedown_callback(CANVAS_ID, NULL, true, mouse_callback);
    emscripten_set_mousemove_callback(CANVAS_ID, NULL, true, mouse_callback);
    emscripten_set_mouseup_callback(CANVAS_ID, NULL, true, mouse_callback);
    
    emscripten_set_wheel_callback(CANVAS_ID, NULL, true, scroll_callback);
    
    emscripten_set_main_loop(loop, 0, true);
    
    return 0;
}