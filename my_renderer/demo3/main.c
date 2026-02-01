#include <emscripten.h>
#include <emscripten/html5.h>
#include <webgpu/webgpu.h>
#include "gpu.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TINYOBJ_LOADER_C_IMPLEMENTATION
#include "tinyobj_loader_c.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#ifdef __cplusplus
}
#endif

// #define HEIGHT 480
// #define WIDTH 640
#define HEIGHT 1000
#define WIDTH 1000

static char const canvas_id[] = "#canvas";
static char const shader_file[] = "demo3/shader.wgsl";
static char const * obj_files[] = {
    
    //"demo3/dragon.obj",
    //   "demo3/floor.obj",
    // "demo3/xyz_axis.obj",
    //  "demo3/fourareen/fourareen.obj",
    "demo3/43-obj/obj/Wolf_One_obj.obj",
    // "demo3/cube.obj",
};

static WGPUColor const bg_color = {0.75f, 0.75f, 0.75f, 1.0f};
static WGPUColor const colors[] = {
    {0.8f, 0.2f, 0.1f, 1.0f}, // red
    {0.2f, 0.8f, 0.1f, 1.0f}, // green
    {0.2f, 0.2f, 0.8f, 1.0f}, // blue
};
static uint8_t const obj_file_count = sizeof(obj_files) / sizeof(obj_files[0]);

struct my_uniforms {
    // float color[4];        // 16 bytes (0-15)
    // float time;            // 4 bytes  (16-19)
    // float padding[3];      // 12 bytes (20-31) - align transform to 16-byte boundary
    float transform[16];   // 64 bytes (32-95) - 4x4 matrix, 16-byte aligned
};

struct mesh_range {
    uint32_t vertex_start;
    uint32_t vertex_count;

    uint32_t index_offset;
    uint32_t index_count;
};
struct mesh_batch {
    float * vertex_data;
    uint32_t vertex_count;
    uint8_t vertex_stride; // 8 (x, y, z, tu, tv, nx, ny, nz) for now

    uint32_t * index_data;
    uint32_t index_count;

    struct mesh_range * meshes;
    uint32_t mesh_count;
};

static struct {
    WGPUDevice device;
    WGPUQueue queue;
    WGPUSurface surface;
    WGPUTextureFormat surface_format;
    
    WGPURenderPipeline pipeline;
    WGPUBuffer point_buffer;
    WGPUBuffer index_buffer;
    uint32_t index_count;
    WGPUBuffer uniform_buffer;
    WGPUPipelineLayout pipeline_layout;
    WGPUBindGroupLayout bind_group_layout;
    WGPUBindGroup bind_group;

    // NEW!
    // Depth testing
    WGPUTexture depth_texture;
    WGPUTextureView depth_view;


    // NEW!
    // Texture
    WGPUTextureView texture_view;

    // NEW!
    // sampler 
    WGPUSampler sampler;

    struct mesh_batch mesh_data;

    // Geometry data on cpu side for debugging
    float * point_buffer_data;
    uint32_t point_count;

    int first;

    struct my_uniforms camera;
    WGPUBuffer camera_buffer;

    // Slider parameters for transform control
    float rotation_speed;
    float scale;
    float base_rotation;

    float rotate_x;
    float rotate_y;
    float rotate_z;

    float translate_x;
    float translate_y;
    float translate_z;
} g_app;

WGPUTexture load_texture(char const * filename, WGPUDevice device, WGPUTextureView * out_texture_view);

bool wheel_callback(int eventType, const EmscriptenWheelEvent *wheelEvent, void *userData) {
    (void)eventType;
    (void)userData;
    //printf("Wheel deltaY: %f\n", wheelEvent->deltaY);
    g_app.translate_z -= wheelEvent->deltaY * 0.001f;
    if (g_app.translate_z < -10.0f) {
        g_app.translate_z = -10.0f;
    }
    if (g_app.translate_z > 0.0f) {
        g_app.translate_z = 0.0f;
    }
    return true;
}

bool mouse_callback(int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData) {
    (void)userData;
    static bool is_dragging = false;
    static bool right_dragging = false;
    static long last_x = 0;
    static long last_y = 0;
    //printf("Button: %ld\n", mouseEvent->button);
    if (eventType == EMSCRIPTEN_EVENT_MOUSEDOWN) {
        switch (mouseEvent->button) {
            case 0: // left button
                is_dragging = true;
                break;
            case 2: // right button
                right_dragging = true;
                break;
            default:
                break;
        }
        last_x = mouseEvent->clientX;
        last_y = mouseEvent->clientY;
        //printf("Mouse down at: (%ld, %ld)\n", last_x, last_y);
    }
    else if (eventType == EMSCRIPTEN_EVENT_MOUSEUP) {
        is_dragging = false;
        right_dragging = false;
    }
    else if (eventType == EMSCRIPTEN_EVENT_MOUSEMOVE) {
        if (is_dragging) { // handle left button drag to translate view
            //printf("Current pos: (%ld, %ld), Last pos: (%ld, %ld)\n", mouseEvent->clientX, mouseEvent->clientY, last_x, last_y);
            long delta_x = mouseEvent->clientX - last_x;
            long delta_y = mouseEvent->clientY - last_y;
            //printf("Mouse move delta: (%ld, %ld)\n", delta_x, delta_y);

            g_app.translate_x += (float)(delta_x * 0.01f);
            g_app.translate_y -= (float)(delta_y * 0.01f);

            last_x = mouseEvent->clientX;
            last_y = mouseEvent->clientY;
        }
        if (right_dragging) { // handle right button drag to rotate view
            long delta_x = mouseEvent->clientX - last_x;
            long delta_y = mouseEvent->clientY - last_y;

            g_app.rotate_y += (float)(delta_x * 0.01f);
            g_app.rotate_x += (float)(delta_y * 0.01f);

            last_x = mouseEvent->clientX;
            last_y = mouseEvent->clientY;
        }
    }
    return true;
}

int app_init(void);
void callback(void);

// File reader callback for tinyobj with Emscripten
void emscripten_file_reader(void *ctx, const char *filename, int is_mtl, const char *obj_filename, char **buf, size_t *len) {
    (void)ctx; // unused
    (void)is_mtl; // unused
    (void)obj_filename; // unused
    FILE *file = fopen(filename, "rb");
    if (!file) {
        printf("Failed to open file: %s\n", filename);
        *buf = NULL;
        *len = 0;
        return;
    }
    // Get file size
    fseek(file, 0, SEEK_END);
    *len = (size_t)ftell(file);
    fseek(file, 0, SEEK_SET);
    // Allocate buffer and read file
    *buf = (char*)malloc(*len + 1);
    if (*buf) {
        size_t read_size = fread(*buf, 1, *len, file);
        (*buf)[read_size] = '\0'; // Null terminate
        *len = read_size;
    } else {
        *len = 0;
    }
    fclose(file);
}

int main(void) {

    if (app_init() != 0) {
        return -1;
    }
    g_app.first = 1;
    
    // Initialize slider parameters with default values
    g_app.rotation_speed = 1.0f;
    g_app.scale = 1.0f;
    g_app.base_rotation = 0.785f; // 45 degrees
    g_app.translate_x = 0.0f;
    g_app.translate_y = -0.1f;
    g_app.translate_z = -1.0f;
    g_app.rotate_x = 0.0f;
    g_app.rotate_y = 0.785f;
    g_app.rotate_z = 0.0f;

    // set up emscripten html5 callbacks
    emscripten_set_wheel_callback(canvas_id, NULL, true, wheel_callback);
    
            // setup mouse drag to translate model
    emscripten_set_mousedown_callback(canvas_id, NULL, true, mouse_callback);
    emscripten_set_mousemove_callback(canvas_id, NULL, true, mouse_callback);
    emscripten_set_mouseup_callback(canvas_id, NULL, true, mouse_callback);

    emscripten_set_main_loop(callback, 0, true);
    
}
void init_pipeline(void);
void init_buffers2(void);
void init_bind_groups(void);
int app_init(void) {
    g_app.surface_format = WGPUTextureFormat_Undefined;
    WGPUInstance instance = create_instance(NULL);
    g_app.surface = create_surface(instance, canvas_id);
    WGPUAdapter adapter = create_adapter(instance, g_app.surface);
    wgpuInstanceRelease(instance); // Release instance after adapter creation
    WGPULimits supported_limits = {0};
    wgpuAdapterGetLimits(adapter, &supported_limits);
    WGPULimits required_limits = {0};
    g_app.device = create_device(adapter, &supported_limits, &required_limits);
   
    g_app.queue = create_queue(g_app.device);
    g_app.surface_format = configure_surface(g_app.device, g_app.surface, adapter, WIDTH, HEIGHT);
    
    // NEW!
    // Create depth texture
    WGPUTextureDescriptor depth_desc = {0};
    depth_desc.usage = WGPUTextureUsage_RenderAttachment;
    depth_desc.dimension = WGPUTextureDimension_2D;
    depth_desc.size = (WGPUExtent3D){WIDTH, HEIGHT, 1};
    depth_desc.format = WGPUTextureFormat_Depth24Plus;
    depth_desc.mipLevelCount = 1;
    depth_desc.sampleCount = 1;
    g_app.depth_texture = wgpuDeviceCreateTexture(g_app.device, &depth_desc);
    g_app.depth_view = wgpuTextureCreateView(g_app.depth_texture, NULL);
    
    wgpuAdapterRelease(adapter); // Release adapter after device creation

    init_pipeline();
    printf("Pipeline initialized\n");
    init_buffers2();
    printf("Buffers initialized\n");
    

    // NEW! Add texture, color texture
    WGPUTextureDescriptor tex_desc = {0};
    tex_desc.dimension = WGPUTextureDimension_2D;
    tex_desc.size = (WGPUExtent3D){256, 256, 1};
    tex_desc.mipLevelCount = 8; // NEW! Mipmaps
    tex_desc.sampleCount = 1;
    tex_desc.format = WGPUTextureFormat_RGBA8Unorm;
    tex_desc.usage = WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst;
    tex_desc.viewFormatCount = 0;
    tex_desc.viewFormats = NULL;
    WGPUTexture texture = wgpuDeviceCreateTexture(g_app.device, &tex_desc);

    WGPUTextureViewDescriptor tex_view_desc = {0};
    tex_view_desc.aspect = WGPUTextureAspect_All;
    tex_view_desc.baseArrayLayer = 0;
    tex_view_desc.arrayLayerCount = 1;
    tex_view_desc.baseMipLevel = 0;
    tex_view_desc.mipLevelCount = 8; // NEW! Mipmaps
    tex_view_desc.dimension = WGPUTextureViewDimension_2D;
    tex_view_desc.format = tex_desc.format;
    g_app.texture_view = wgpuTextureCreateView(texture, &tex_view_desc);

    uint8_t * pixels = malloc(4 * tex_desc.size.width * tex_desc.size.height);
    for (uint32_t i = 0; i < tex_desc.size.width; ++i) {
        for (uint32_t j = 0; j < tex_desc.size.height; ++j) {
            uint8_t * pixel = &pixels[4 * (j * tex_desc.size.width + i)];
            // pixel[0] = (uint8_t)(i % 256); // R
            // pixel[1] = (uint8_t)(j % 256); // G
            // pixel[2] = 128;               // B
            pixel[0] = (i / 16) % 2 == (j / 16) % 2 ? 255 : 0; // R
            pixel[1] = ((i - j) / 16) % 2 == 0 ? 255 : 0; // G
            pixel[2] = ((i + j) / 16) % 2 == 0 ? 255 : 0; // B
            pixel[3] = 255;               // A
        }
    }
    
    WGPUTexelCopyTextureInfo destination = {0};
    destination.texture = texture;
    destination.mipLevel = 0;
    destination.origin = (WGPUOrigin3D){0, 0, 0};
    destination.aspect = WGPUTextureAspect_All;

    WGPUTexelCopyBufferLayout source = {0};
    source.offset = 0;
    source.bytesPerRow = 4 * tex_desc.size.width;
    source.rowsPerImage = tex_desc.size.height;

    // wgpuQueueWriteTexture(
    //     g_app.queue,
    //     &destination,
    //     pixels,
    //     4 * tex_desc.size.width * tex_desc.size.height,
    //     &source,
    //     &tex_desc.size
    // );
    free(pixels);

    // NEW! Generate mipmaps
    WGPUTexelCopyTextureInfo destination_mip = {0};
    destination_mip.texture = texture;
    destination_mip.origin = (WGPUOrigin3D){0, 0, 0};
    destination_mip.aspect = WGPUTextureAspect_All;
    WGPUTexelCopyBufferLayout source_mip = {0};
    source_mip.offset = 0;

    WGPUExtent3D mip_level_size = tex_desc.size;
    uint8_t * previous_level_pixels = NULL;

    for (uint32_t level = 0; level < tex_desc.mipLevelCount; ++level) {
        uint8_t * pixels_mip = malloc(4 * mip_level_size.width * mip_level_size.height);
        for (uint32_t i = 0; i < mip_level_size.width; ++i) {
            for (uint32_t j = 0; j < mip_level_size.height; ++j) {
                uint8_t* p = &pixels_mip[4 * (j * mip_level_size.width + i)];
                if (level == 0) {
                    // Our initial texture formula
                    p[0] = (i / 16) % 2 == (j / 16) % 2 ? 255 : 0; // r
                    p[1] = ((i - j) / 16) % 2 == 0 ? 255 : 0; // g
                    p[2] = ((i + j) / 16) % 2 == 0 ? 255 : 0; // b
                } else {
                    // // Some debug value for visualizing mip levels
                    // p[0] = level % 2 == 0 ? 255 : 0;
                    // p[1] = (level / 2) % 2 == 0 ? 255 : 0;
                    // p[2] = (level / 4) % 2 == 0 ? 255 : 0;
                    uint8_t * p1 = &previous_level_pixels[4 * ((2 * j) * (mip_level_size.width * 2) + (2 * i))];
                    uint8_t * p2 = &previous_level_pixels[4 * ((2 * j) * (mip_level_size.width * 2) + (2 * i + 1))];
                    uint8_t * p3 = &previous_level_pixels[4 * ((2 * j + 1) * (mip_level_size.width * 2) + (2 * i))];
                    uint8_t * p4 = &previous_level_pixels[4 * ((2 * j + 1) * (mip_level_size.width * 2) + (2 * i + 1))];

                    p[0] = ((p1[0] + p2[0] + p3[0] + p4[0]) / 4); // r
                    p[1] = ((p1[1] + p2[1] + p3[1] + p4[1]) / 4); // g
                    p[2] = ((p1[2] + p2[2] + p3[2] + p4[2]) / 4); // b
                }
                p[3] = 255; // a
            }
        }

        destination_mip.mipLevel = level;
        source_mip.bytesPerRow = 4 * mip_level_size.width;
        source_mip.rowsPerImage = mip_level_size.height;

        wgpuQueueWriteTexture(
            g_app.queue,
            &destination_mip,
            pixels_mip,
            4 * mip_level_size.width * mip_level_size.height,
            &source_mip,
            &mip_level_size
        );
        //free(pixels_mip);
        // Halve dimensions for next mip level
        mip_level_size.width /= 2;
        mip_level_size.height /= 2;

        // Keep previous level pixels for next iteration
        if (previous_level_pixels) {
            free(previous_level_pixels);
        }
        previous_level_pixels = pixels_mip;
    }
    free(previous_level_pixels);



    // NEW! Sampler
    WGPUSamplerDescriptor sampler_desc = {0};
    sampler_desc.addressModeU = WGPUAddressMode_Repeat;
    sampler_desc.addressModeV = WGPUAddressMode_Repeat;
    sampler_desc.addressModeW = WGPUAddressMode_ClampToEdge;
    sampler_desc.magFilter = WGPUFilterMode_Linear;
    sampler_desc.minFilter = WGPUFilterMode_Linear;
    sampler_desc.mipmapFilter = WGPUMipmapFilterMode_Linear;
    sampler_desc.lodMinClamp = 0.0f;
    sampler_desc.lodMaxClamp = 8.0f; // match mipLevelCount
    sampler_desc.compare = WGPUCompareFunction_Undefined;
    sampler_desc.maxAnisotropy = 1;
    g_app.sampler = wgpuDeviceCreateSampler(g_app.device, &sampler_desc);

    // // should use these 
    // wgpuTextureDestroy(texture);
    // wgpuTextureRelease(texture);

    // WGPUTexture texture_boat = load_texture("demo3/fourareen/fourareen2K_albedo.jpg", g_app.device, &g_app.texture_view);
    WGPUTexture texture_boat = load_texture("demo3/43-obj/obj/textures/Wolf_Body.jpg", g_app.device, &g_app.texture_view);

    init_bind_groups();
    printf("Bind groups initialized\n");

    return 0;
}

// 256-byte aligned uniform buffer
static const uint32_t uniform_stride = 256; 
WGPUTextureView get_next_surface_texture_view(void);
void mat4_mul(const float* a, const float* b, float* out) {
    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 4; col++) {
            out[row * 4 + col] =
                a[row * 4 + 0] * b[0 * 4 + col] +
                a[row * 4 + 1] * b[1 * 4 + col] +
                a[row * 4 + 2] * b[2 * 4 + col] +
                a[row * 4 + 3] * b[3 * 4 + col];
        }
    }
}
void mat4_add(const float* a, const float* b, float* out) {
    for (int i = 0; i < 16; i++) {
        out[i] = a[i] + b[i];
    }
}

// static float const focal_length = 1.0f;
// static float const ratio = 1.0f; // WIDTH / HEIGHT
// static float const far = 100.0f;
// static float const near = 0.01f;
// static float const divider = 1 / (focal_length * (far - near));
// static float g_projection[16] = {
//     1.0f,   0.0f,       0.0f,                   0.0f,
//     0.0f,   ratio,      0.0f,                   0.0f,
//     0.0f,   0.0f,       far * divider,          -far * near * divider,
//     0.0f,   0.0f,       1.0f / focal_length,    0.0f

// };
void callback(void) {
    //float time = emscripten_get_now() / 1000.0f;

    float r[3] = {g_app.rotate_x, g_app.rotate_y, g_app.rotate_z};
    float s[3] = {sinf(r[0]), sinf(r[1]), sinf(r[2])};
    float c[3] = {cosf(r[0]), cosf(r[1]), cosf(r[2])};

    float model_matrices[obj_file_count][16] = {
                {
            0.2f, 0.0f, 0.0f, 0.0f,
            0.0f, 0.2f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.2f, 0.0f,
            0.0f,  0.0f, 0.0f, 1.0f  // Dragon
        },
        {
            0.2f, 0.0f, 0.0f, 0.0f,
            0.0f, 0.2f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.2f, 0.0f,
            0.0f, -1.0f, 0.0f, 1.0f  // Floor
        },
        {
            0.2f, 0.0f, 0.0f, 0.0f,
            0.0f, 0.2f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.2f, 0.0f,
            -0.7f, -0.3f, 0.0f, 1.0f  // XYZ Axis
        },
        {
            0.2f, 0.0f, 0.0f, 0.0f,
            0.0f, 0.2f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.2f, 0.0f,
            1.0f, 0.0f, 0.0f, 1.0f  // Cube
        }
    };

    for (uint8_t i = 0; i < obj_file_count; i++) {
        wgpuQueueWriteBuffer(g_app.queue, g_app.uniform_buffer, i * uniform_stride + offsetof(struct my_uniforms, transform), model_matrices[i], sizeof(model_matrices[i]));
    }


    // write camera matrix uniform
    g_app.camera = (struct my_uniforms){
        .transform = {
            (c[1]*c[2]), (-c[1]*s[2]), s[1], 0.0f,
            (s[0]*s[1]*c[2] + c[0]*s[2]), (-s[0]*s[1]*s[2] + c[0]*c[2]), (-s[0]*c[1]), 0.0f,
            (-c[0]*s[1]*c[2] + s[0]*s[2]), (c[0]*s[1]*s[2] + s[0]*c[2]), (c[0]*c[1]), 0.0f,
            g_app.translate_x, g_app.translate_y, -g_app.translate_z, 1.0f // move back along z
        }
    };

    wgpuQueueWriteBuffer(g_app.queue, g_app.camera_buffer, 0, &g_app.camera, sizeof(g_app.camera));

    //wgpuQueueWriteBuffer(g_app.queue, g_app.uniform_buffer, offsetof(struct my_uniforms, transform), transposed, sizeof(transposed));

    WGPUTextureView target_view = get_next_surface_texture_view();
    if (!target_view) return;

    WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(g_app.device, NULL);
    WGPURenderPassDescriptor render_pass_desc = {0};
    WGPURenderPassColorAttachment color_attachment = {0};
    color_attachment.view = target_view;
    color_attachment.depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;
    color_attachment.loadOp = WGPULoadOp_Clear;
    color_attachment.storeOp = WGPUStoreOp_Store;
    color_attachment.clearValue = bg_color;
    render_pass_desc.colorAttachmentCount = 1;
    render_pass_desc.colorAttachments = &color_attachment;

    // NEW!
    // Add depth attachment
    WGPURenderPassDepthStencilAttachment depth_attachment = {0};
    depth_attachment.view = g_app.depth_view;
    depth_attachment.depthLoadOp = WGPULoadOp_Clear;
    depth_attachment.depthStoreOp = WGPUStoreOp_Store;
    depth_attachment.depthClearValue = 1.0f;
    render_pass_desc.depthStencilAttachment = &depth_attachment;

    WGPURenderPassEncoder render_pass = wgpuCommandEncoderBeginRenderPass(encoder, &render_pass_desc);
    wgpuRenderPassEncoderSetPipeline(render_pass, g_app.pipeline);
    wgpuRenderPassEncoderSetVertexBuffer(render_pass, 0, g_app.point_buffer, 0, WGPU_WHOLE_SIZE);
    wgpuRenderPassEncoderSetIndexBuffer(render_pass, g_app.index_buffer, WGPUIndexFormat_Uint32, 0, WGPU_WHOLE_SIZE);

    uint32_t dynamic_offset = 0;
// TODO: change draw calls and keep track of object index count and offsets

    for (uint32_t i = 0; i < g_app.mesh_data.mesh_count; i++) {
        dynamic_offset = i * uniform_stride;
        wgpuRenderPassEncoderSetBindGroup(render_pass, 0, g_app.bind_group, 1, &dynamic_offset);
        wgpuRenderPassEncoderDrawIndexed(render_pass, g_app.mesh_data.meshes[i].index_count, 1, g_app.mesh_data.meshes[i].index_offset, 0, 0);
    }

    // wgpuRenderPassEncoderSetBindGroup(render_pass, 0, g_app.bind_group, 1, &dynamic_offset);
    // //wgpuRenderPassEncoderDrawIndexed(render_pass, g_app.index_count, 1, 0, 0, 0);
    // wgpuRenderPassEncoderDrawIndexed(render_pass, g_app.mesh_data.meshes[0].index_count, 1, g_app.mesh_data.meshes[0].index_offset, 0, 0);
    // // Draw second object with different transform
    // dynamic_offset = uniform_stride;
    // wgpuRenderPassEncoderSetBindGroup(render_pass, 0, g_app.bind_group, 1, &dynamic_offset);
    // //wgpuRenderPassEncoderDrawIndexed(render_pass, g_app.index_count, 1, 0, 0, 0);
    // wgpuRenderPassEncoderDrawIndexed(render_pass, g_app.mesh_data.meshes[1].index_count, 1, g_app.mesh_data.meshes[1].index_offset, 0, 0);

    wgpuRenderPassEncoderEnd(render_pass);
    wgpuRenderPassEncoderRelease(render_pass);

    WGPUCommandBuffer command_buffer = wgpuCommandEncoderFinish(encoder, NULL);
    wgpuQueueSubmit(g_app.queue, 1, &command_buffer);
    wgpuCommandBufferRelease(command_buffer);
}
WGPUTextureView get_next_surface_texture_view(void) {
    WGPUSurfaceTexture surface_texture;
    wgpuSurfaceGetCurrentTexture(g_app.surface, &surface_texture);
    if (surface_texture.status != WGPUSurfaceGetCurrentTextureStatus_SuccessOptimal &&
        surface_texture.status != WGPUSurfaceGetCurrentTextureStatus_SuccessSuboptimal) {
        printf("Failed to get current surface texture\n");
        return NULL;
    }

    WGPUTextureViewDescriptor view_desc = {0};
    view_desc.format = wgpuTextureGetFormat(surface_texture.texture);
    view_desc.dimension = WGPUTextureViewDimension_2D;
    view_desc.baseMipLevel = 0;
    view_desc.mipLevelCount = 1;
    view_desc.baseArrayLayer = 0;
    view_desc.arrayLayerCount = 1;
    view_desc.aspect = WGPUTextureAspect_All;
    WGPUTextureView texture_view = wgpuTextureCreateView(surface_texture.texture, &view_desc);
    return texture_view;
}

struct vertex_input {
    float position[3];
    float texcoord[2];
    float normal[3];
    float color[3];
};
void init_pipeline(void) {
    WGPUShaderModule shader_module = load_shader_module(g_app.device, shader_file);
    WGPURenderPipelineDescriptor pipeline_desc = {0};

    WGPUVertexBufferLayout vertex_buffer_layout = {0};
    WGPUVertexAttribute vertex_attributes[4] = {0};
    vertex_attributes[0].format = WGPUVertexFormat_Float32x3;
    vertex_attributes[0].offset = offsetof(struct vertex_input, position);
    vertex_attributes[0].shaderLocation = 0; // position
    vertex_attributes[1].format = WGPUVertexFormat_Float32x2;
    vertex_attributes[1].offset = offsetof(struct vertex_input, texcoord);
    vertex_attributes[1].shaderLocation = 1; // texcoord
    vertex_attributes[2].format = WGPUVertexFormat_Float32x3;
    vertex_attributes[2].offset = offsetof(struct vertex_input, normal);
    vertex_attributes[2].shaderLocation = 2; // normal
    vertex_attributes[3].format = WGPUVertexFormat_Float32x3;
    vertex_attributes[3].offset = offsetof(struct vertex_input, color);
    vertex_attributes[3].shaderLocation = 3; // color
    vertex_buffer_layout.arrayStride = sizeof(struct vertex_input);
    vertex_buffer_layout.attributeCount = 4;
    vertex_buffer_layout.attributes = vertex_attributes;

    pipeline_desc.vertex.bufferCount = 1;
    pipeline_desc.vertex.buffers = &vertex_buffer_layout;
    pipeline_desc.vertex.module = shader_module;
    pipeline_desc.vertex.entryPoint = toWGPUStringView("vs_main");

    pipeline_desc.primitive.topology = WGPUPrimitiveTopology_TriangleList;
    pipeline_desc.primitive.stripIndexFormat = WGPUIndexFormat_Undefined;
    pipeline_desc.primitive.frontFace = WGPUFrontFace_CW;
    pipeline_desc.primitive.cullMode = WGPUCullMode_Back; // NEW!
    //pipeline_desc.primitive.cullMode = WGPUCullMode_None; // NEW!

    WGPUFragmentState fragment_state = {0};
    fragment_state.module = shader_module;
    fragment_state.entryPoint = toWGPUStringView("fs_main");

    WGPUBlendState blend_state = {0};
    blend_state.color.srcFactor = WGPUBlendFactor_SrcAlpha;
    blend_state.color.dstFactor = WGPUBlendFactor_OneMinusSrcAlpha;
    blend_state.color.operation = WGPUBlendOperation_Add;
    blend_state.alpha.srcFactor = WGPUBlendFactor_Zero;
    blend_state.alpha.dstFactor = WGPUBlendFactor_One;
    blend_state.alpha.operation = WGPUBlendOperation_Add;

    WGPUColorTargetState color_target = {0};
    color_target.format = g_app.surface_format;
    color_target.blend = &blend_state;
    color_target.writeMask = WGPUColorWriteMask_All;

    fragment_state.targetCount = 1;
    fragment_state.targets = &color_target;
    pipeline_desc.fragment = &fragment_state;

    // NEW!
    // Enable depth testing
    WGPUDepthStencilState depth_stencil_state = {0};
    depth_stencil_state.format = WGPUTextureFormat_Depth24Plus;
    depth_stencil_state.depthWriteEnabled = true;
    depth_stencil_state.depthCompare = WGPUCompareFunction_Less;
    pipeline_desc.depthStencil = &depth_stencil_state;
    
    pipeline_desc.multisample.count = 1;
    pipeline_desc.multisample.mask = ~0u;
    pipeline_desc.multisample.alphaToCoverageEnabled = false;

    WGPUBindGroupLayoutEntry bgl_entry = {0};
    //set_default_bindings_layout(&bgl_entry);
    bgl_entry.binding = 0;
    bgl_entry.visibility = WGPUShaderStage_Vertex | WGPUShaderStage_Fragment;
    bgl_entry.buffer.type = WGPUBufferBindingType_Uniform;
    bgl_entry.buffer.minBindingSize = sizeof(struct my_uniforms);
    bgl_entry.buffer.hasDynamicOffset = true;
    
    WGPUBindGroupLayoutEntry bgl_entry2 = {0}; // camera uniform
    bgl_entry2.binding = 1;
    bgl_entry2.visibility = WGPUShaderStage_Vertex;
    bgl_entry2.buffer.type = WGPUBufferBindingType_Uniform;
    bgl_entry2.buffer.minBindingSize = sizeof(struct my_uniforms);
    bgl_entry2.buffer.hasDynamicOffset = false;

    WGPUBindGroupLayoutEntry bgl_entry3 = {0}; // texture sampler
    bgl_entry3.binding = 2;
    bgl_entry3.visibility = WGPUShaderStage_Fragment;
    bgl_entry3.texture.sampleType = WGPUTextureSampleType_Float;
    bgl_entry3.texture.viewDimension = WGPUTextureViewDimension_2D;

    WGPUBindGroupLayoutEntry bgl_entry4 = {0}; // sampler
    bgl_entry4.binding = 3;
    bgl_entry4.visibility = WGPUShaderStage_Fragment;
    bgl_entry4.sampler.type = WGPUSamplerBindingType_Filtering;

    // bgl_entry.storageTexture = (WGPUStorageTextureBindingLayout){0};

    WGPUBindGroupLayoutDescriptor bgl_desc = {0};
    bgl_desc.entryCount = 4;
    bgl_desc.entries = (WGPUBindGroupLayoutEntry[]){bgl_entry, bgl_entry2, bgl_entry3, bgl_entry4};
    g_app.bind_group_layout = wgpuDeviceCreateBindGroupLayout(g_app.device, &bgl_desc);

    WGPUPipelineLayoutDescriptor pipeline_layout_desc = {0};
    pipeline_layout_desc.bindGroupLayoutCount = 1;
    pipeline_layout_desc.bindGroupLayouts = &g_app.bind_group_layout;
    g_app.pipeline_layout = wgpuDeviceCreatePipelineLayout(g_app.device, &pipeline_layout_desc);

    pipeline_desc.layout = g_app.pipeline_layout;
    g_app.pipeline = wgpuDeviceCreateRenderPipeline(g_app.device, &pipeline_desc);
    wgpuShaderModuleRelease(shader_module); // Release shader module after pipeline creation
}

int add_color_to_mesh_batch(struct mesh_batch * batch) {
    uint32_t new_vertex_stride = batch->vertex_stride + 3; // add r, g, b
    float * new_vertex_data = malloc(batch->vertex_count * new_vertex_stride * sizeof(float));
    srand(44); // fixed seed for reproducibility
    // create a random base color for each mesh

    for (uint8_t m = 0; m < batch->mesh_count; m++) {
        int random_index = rand() % (sizeof(colors) / sizeof(colors[0]));
        float r = colors[random_index].r;
        float g = colors[random_index].g;
        float b = colors[random_index].b;
        uint32_t start = batch->meshes[m].vertex_start;
        uint32_t count = batch->meshes[m].vertex_count;
        for (uint32_t i = 0; i < count; i++) {
            uint32_t vi = start + i;
            // copy existing vertex data
            memcpy(&new_vertex_data[vi * new_vertex_stride], &batch->vertex_data[vi * batch->vertex_stride], batch->vertex_stride * sizeof(float));
            // add some random color variation around base color

            // full random color
            float r_var = rand() / (float)RAND_MAX;
            float g_var = rand() / (float)RAND_MAX;
            float b_var = rand() / (float)RAND_MAX;
            new_vertex_data[vi * new_vertex_stride + batch->vertex_stride + 0] = fminf(fmaxf(r + r_var, 0.0f), 1.0f);
            new_vertex_data[vi * new_vertex_stride + batch->vertex_stride + 1] = fminf(fmaxf(g + g_var, 0.0f), 1.0f);
            new_vertex_data[vi * new_vertex_stride + batch->vertex_stride + 2] = fminf(fmaxf(b + b_var, 0.0f), 1.0f);
        }
    }
    // for (uint32_t i = 0; i < batch->vertex_count; i++) {
    //     // copy existing vertex data
    //     memcpy(&new_vertex_data[i * new_vertex_stride], &batch->vertex_data[i * batch->vertex_stride], batch->vertex_stride * sizeof(float));
    //     // add random color
    //     float r = rand() / (float)RAND_MAX;
    //     float g = rand() / (float)RAND_MAX;
    //     float b = rand() / (float)RAND_MAX;
    //     new_vertex_data[i * new_vertex_stride + batch->vertex_stride + 0] = r;
    //     new_vertex_data[i * new_vertex_stride + batch->vertex_stride + 1] = g;
    //     new_vertex_data[i * new_vertex_stride + batch->vertex_stride + 2] = b;
    // }
    free(batch->vertex_data);
    batch->vertex_data = new_vertex_data;
    batch->vertex_stride = new_vertex_stride;
    return 0;
}
// OLD FUNCTION DEPRECATED
// int load_objects(char const * filenames[], uint32_t file_count, struct mesh_batch * out) {
//     uint8_t const VERTEX_STRIDE = 8; // x, y, z, tu, tv, nx, ny, nz (add color later)
//     memset(out, 0, sizeof(struct mesh_batch));
//     out->vertex_stride = VERTEX_STRIDE;
//     out->meshes = malloc(file_count * sizeof(struct mesh_range));
//     out->mesh_count = file_count;

//     for (uint32_t i = 0; i < file_count; i++) {
//         float * v = NULL;
//         uint32_t * idx = NULL;
//         uint32_t vc = 0, ic = 0;
//         if (load_geometry_from_obj(filenames[i], &v, &vc, &idx, &ic) != 0) {
//             printf("Failed to load geometry from OBJ file: %s\n", filenames[i]);
//             return -1;
//         }
//         struct mesh_range * m = &out->meshes[i];

//         m->vertex_start = out->vertex_count;
//         m->vertex_count = vc;
//         m->index_offset = out->index_count;
//         m->index_count = ic;

//         // grow vertex buffer
//         out->vertex_data = realloc(out->vertex_data, (out->vertex_count + vc) * VERTEX_STRIDE * sizeof(float));
//         memcpy(&out->vertex_data[out->vertex_count * VERTEX_STRIDE], v, vc * VERTEX_STRIDE * sizeof(float));

//         // grow index buffer
//         out->index_data = realloc(out->index_data, (out->index_count + ic) * sizeof(uint32_t));

//         // copy and rebase indices
//         for (uint32_t j = 0; j < ic; j++) {
//             out->index_data[out->index_count + j] = idx[j] + m->vertex_start;
//         }
//         out->vertex_count += vc;
//         out->index_count += ic;

//         free(v);
//         free(idx);
//     }

//     printf("Loaded %u objects: total %u vertices, %u indices\n", file_count, out->vertex_count, out->index_count);
//     return 0;
// }
void color_arrows(struct mesh_batch * batch, struct mesh_range range, WGPUColor color_x, WGPUColor color_y, WGPUColor color_z);
// this uses tinyobjloader and is faster than my own load_objects function that uses my utils.c load_geometry_from_obj function
void load_objects2(char const * filenames[], uint32_t file_count, struct mesh_batch * out) {
    
    tinyobj_attrib_t attrib = {0};
    tinyobj_shape_t * shapes = NULL;
    tinyobj_material_t * materials = NULL;
    size_t num_shapes = 0;
    size_t num_materials = 0;
    
    out->mesh_count = file_count;
    out->meshes = malloc(file_count * sizeof(struct mesh_range));
    out->vertex_stride = 8; // x, y, z, tu, tv, nx, ny, nz (add color later)
    
    printf("Number of OBJ files to load: %u\n", file_count);

    for (uint32_t i = 0; i < file_count; i++) {
        printf("Loading OBJ file: %s\n", filenames[i]);
        
        
        int ret = tinyobj_parse_obj(&attrib, &shapes, &num_shapes, &materials, &num_materials, filenames[i], emscripten_file_reader, NULL, TINYOBJ_FLAG_TRIANGULATE);
        if (ret != TINYOBJ_SUCCESS) {
            printf("Failed to load OBJ file: %s\n", filenames[i]);
            continue;
        }

        out->meshes[i].vertex_start = out->vertex_count;
        out->meshes[i].index_offset = out->index_count;
        out->meshes[i].vertex_count = attrib.num_faces;
        out->meshes[i].index_count = attrib.num_faces;
        
        uint32_t new_vertex_count = out->vertex_count + attrib.num_faces;
        uint32_t new_index_count = out->index_count + attrib.num_faces;

        out->vertex_data = realloc(out->vertex_data, new_vertex_count * out->vertex_stride * sizeof(float));

        out->index_data = realloc(out->index_data, new_index_count * sizeof(uint32_t));

        // iterate over vertices (every third index is a new face)
        for (uint32_t i = 0; i < attrib.num_faces; i++) {

            // add new vertex to out->vertex_data
            if (attrib.faces[i].v_idx != (int)TINYOBJ_INVALID_INDEX) {
                // 3 floats per vertex
                out->vertex_data[(out->vertex_count + i) * out->vertex_stride + 0] = attrib.vertices[attrib.faces[i].v_idx * 3 + 0];
                out->vertex_data[(out->vertex_count + i) * out->vertex_stride + 1] = attrib.vertices[attrib.faces[i].v_idx * 3 + 1];
                out->vertex_data[(out->vertex_count + i) * out->vertex_stride + 2] = attrib.vertices[attrib.faces[i].v_idx * 3 + 2];
            } else {
                out->vertex_data[(out->vertex_count + i) * out->vertex_stride + 0] = 0.0f;
                out->vertex_data[(out->vertex_count + i) * out->vertex_stride + 1] = 0.0f;
                out->vertex_data[(out->vertex_count + i) * out->vertex_stride + 2] = 0.0f;
            }
            if (attrib.faces[i].vt_idx != (int)TINYOBJ_INVALID_INDEX) {
                // 2 texcoord per vertex
                out->vertex_data[(out->vertex_count + i) * out->vertex_stride + 3] = attrib.texcoords[attrib.faces[i].vt_idx * 2 + 0];
                out->vertex_data[(out->vertex_count + i) * out->vertex_stride + 4] = attrib.texcoords[attrib.faces[i].vt_idx * 2 + 1];
            } else {
                out->vertex_data[(out->vertex_count + i) * out->vertex_stride + 3] = 0.0f;
                out->vertex_data[(out->vertex_count + i) * out->vertex_stride + 4] = 0.0f;
            }
            if (attrib.faces[i].vn_idx != (int)TINYOBJ_INVALID_INDEX) {
                // 3 normal per vertex
                out->vertex_data[(out->vertex_count + i) * out->vertex_stride + 5] = attrib.normals[attrib.faces[i].vn_idx * 3 + 0];
                out->vertex_data[(out->vertex_count + i) * out->vertex_stride + 6] = attrib.normals[attrib.faces[i].vn_idx * 3 + 1];
                out->vertex_data[(out->vertex_count + i) * out->vertex_stride + 7] = attrib.normals[attrib.faces[i].vn_idx * 3 + 2];
            } else {
                out->vertex_data[(out->vertex_count + i) * out->vertex_stride + 5] = 0.0f;
                out->vertex_data[(out->vertex_count + i) * out->vertex_stride + 6] = 0.0f;
                out->vertex_data[(out->vertex_count + i) * out->vertex_stride + 7] = 0.0f;
            }

            // add new index to out->index_data
            out->index_data[out->index_count + i] = out->vertex_count + i;
        
        }
        out->vertex_count += attrib.num_faces;
        out->index_count += attrib.num_faces;
    }
    tinyobj_attrib_free(&attrib);
    tinyobj_shapes_free(shapes, num_shapes);
    tinyobj_materials_free(materials, num_materials);
}
void load_objects3(char const * filenames[], uint32_t file_count, struct mesh_batch * out);
void init_buffers2(void) {
    // Create point buffer
    struct mesh_batch batch = {0};
    load_objects3(obj_files, obj_file_count, &batch);
    //exit(0);
    add_color_to_mesh_batch(&batch);


    



    // // color arrows
    // if (strcmp(obj_files[2], "demo3/xyz_axis.obj") == 0) {
    //     color_arrows(&batch, batch.meshes[2], (WGPUColor){1.0f, 0.0f, 0.0f, 1.0f}, (WGPUColor){0.0f, 1.0f, 0.0f, 1.0f}, (WGPUColor){0.0f, 0.0f, 1.0f, 1.0f});
    // }

    WGPUBufferDescriptor point_buffer_desc = {0};
    point_buffer_desc.size = batch.vertex_count * batch.vertex_stride * sizeof(float);
    point_buffer_desc.usage = WGPUBufferUsage_Vertex | WGPUBufferUsage_CopyDst;
    g_app.point_buffer = wgpuDeviceCreateBuffer(g_app.device, &point_buffer_desc);
    wgpuQueueWriteBuffer(g_app.queue, g_app.point_buffer, 0, batch.vertex_data, batch.vertex_count * batch.vertex_stride * sizeof(float));

    // Create index buffer
    WGPUBufferDescriptor index_buffer_desc = {0};
    index_buffer_desc.size = batch.index_count * sizeof(uint32_t);
    index_buffer_desc.size = (index_buffer_desc.size + 3) & ~3; // Align to 4 bytes
    index_buffer_desc.usage = WGPUBufferUsage_Index | WGPUBufferUsage_CopyDst;
    g_app.index_buffer = wgpuDeviceCreateBuffer(g_app.device, &index_buffer_desc);
    wgpuQueueWriteBuffer(g_app.queue, g_app.index_buffer, 0, batch.index_data, index_buffer_desc.size);
    printf("Loaded geometry: %u points, %u indices\n", batch.vertex_count, batch.index_count);
    printf("HELLO?\n");
    printf("First 10 vertices with UV coordinates:\n");
    
    for(uint32_t i = 0; i < 10 && i < batch.vertex_count; i++) {
        printf("Vertex %u: pos(%.3f, %.3f, %.3f) uv(%.3f, %.3f) normal(%.3f, %.3f, %.3f)\n", 
               i, 
               batch.vertex_data[i * 11 + 0], batch.vertex_data[i * 11 + 1], batch.vertex_data[i * 11 + 2],
               batch.vertex_data[i * 11 + 3], batch.vertex_data[i * 11 + 4],
               batch.vertex_data[i * 11 + 5], batch.vertex_data[i * 11 + 6], batch.vertex_data[i * 11 + 7]);
    }
    // Free loaded geometry data
    //free_geometry_data(merged_point_data, merged_index_data);
    // keep point_data for debugging
    g_app.point_buffer_data = batch.vertex_data;
    g_app.point_count = batch.vertex_count;
    // free index data
    free(batch.index_data);

    // Create uniform buffer
    WGPUBufferDescriptor uniform_buffer_desc = {0};
    uniform_buffer_desc.size = obj_file_count * uniform_stride; // space for multiple objects
    uniform_buffer_desc.usage = WGPUBufferUsage_Uniform | WGPUBufferUsage_CopyDst;
    g_app.uniform_buffer = wgpuDeviceCreateBuffer(g_app.device, &uniform_buffer_desc);

    // Upload initial uniform data (identity matrices, white color, time = 0)
    struct my_uniforms initial_uniforms = {
        // .color = {0.0f, 1.0f, 0.4f, 1.0f},
        // .time = 0.0f,
        .transform = {
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        }
    };
    for (uint8_t i = 0; i < obj_file_count; i++) {
        wgpuQueueWriteBuffer(g_app.queue, g_app.uniform_buffer, i * uniform_stride, &initial_uniforms, sizeof(initial_uniforms));
    }

    g_app.mesh_data = batch; // store mesh data for rendering

    WGPUBufferDescriptor camera_buffer_desc = {0};
    camera_buffer_desc.size = sizeof(struct my_uniforms);
    camera_buffer_desc.usage = WGPUBufferUsage_Uniform | WGPUBufferUsage_CopyDst;
    g_app.camera_buffer = wgpuDeviceCreateBuffer(g_app.device, &camera_buffer_desc);
    
    // Upload initial camera data (identity matrix)
    g_app.camera = (struct my_uniforms){
        .transform = {
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 3.0f, 1.0f
        }
    };
    wgpuQueueWriteBuffer(g_app.queue, g_app.camera_buffer, 0, &g_app.camera, sizeof(g_app.camera));
}

void init_bind_groups(void) {
    WGPUBindGroupEntry binding = {0};
    binding.binding = 0;
    binding.buffer = g_app.uniform_buffer;
    binding.offset = 0;
    binding.size = sizeof(struct my_uniforms);

    WGPUBindGroupEntry binding2 = {0};
    binding2.binding = 1;
    binding2.offset = 0;
    binding2.buffer = g_app.camera_buffer;
    binding2.size = sizeof(struct my_uniforms);

    WGPUBindGroupEntry binding3 = {0}; // texture
    binding3.binding = 2;
    binding3.textureView = g_app.texture_view;

    WGPUBindGroupEntry binding4 = {0}; // sampler
    binding4.binding = 3;
    binding4.sampler = g_app.sampler;

    WGPUBindGroupDescriptor bind_group_desc = {0};
    bind_group_desc.layout = g_app.bind_group_layout;
    bind_group_desc.entryCount = 4;
    bind_group_desc.entries = (WGPUBindGroupEntry[]){binding, binding2, binding3, binding4};
    g_app.bind_group = wgpuDeviceCreateBindGroup(g_app.device, &bind_group_desc);
}

// void append_mesh_to_buffers(struct mesh_data * mesh, WGPUBuffer * out_point_buffer, WGPUBuffer * out_index_buffer) {
//     // Create point buffer
//     WGPUBufferDescriptor point_buffer_desc = {0};
//     point_buffer_desc.size = mesh->point_count * sizeof(struct vertex_input);
//     point_buffer_desc.usage = WGPUBufferUsage_Vertex | WGPUBufferUsage_CopyDst;
//     *out_point_buffer = wgpuDeviceCreateBuffer(g_app.device, &point_buffer_desc);
//     wgpuQueueWriteBuffer(g_app.queue, *out_point_buffer, 0, mesh->points, mesh->point_count * sizeof(struct vertex_input));

//     // Create index buffer
//     WGPUBufferDescriptor index_buffer_desc = {0};
//     index_buffer_desc.size = mesh->index_count * sizeof(uint16_t);
//     index_buffer_desc.size = (index_buffer_desc.size + 3) & ~3; // Align to 4 bytes
//     index_buffer_desc.usage = WGPUBufferUsage_Index | WGPUBufferUsage_CopyDst;
//     *out_index_buffer = wgpuDeviceCreateBuffer(g_app.device, &index_buffer_desc);
//     wgpuQueueWriteBuffer(g_app.queue, *out_index_buffer, 0, mesh->indices, index_buffer_desc.size);

    
// JavaScript interface function to set parameters from sliders
#include <string.h>
EMSCRIPTEN_KEEPALIVE
void set_parameter(const char* param_name, float value) {
    if (strcmp(param_name, "rotate_x") == 0) {
        g_app.rotate_x = value;
    } else if (strcmp(param_name, "rotate_y") == 0) {
        g_app.rotate_y = value;
    } else if (strcmp(param_name, "rotate_z") == 0) {
        g_app.rotate_z = value;
    } else if (strcmp(param_name, "scale") == 0) {
        g_app.scale = value;
    } else if (strcmp(param_name, "translate_x") == 0) {
        g_app.translate_x = value;
    } else if (strcmp(param_name, "translate_y") == 0) {
        g_app.translate_y = value;
    } else if (strcmp(param_name, "translate_z") == 0) {
        g_app.translate_z = value;
    }
}
// }

void color_arrows(struct mesh_batch * batch, struct mesh_range range, WGPUColor color_x, WGPUColor color_y, WGPUColor color_z) {
    // assume each vertex already has color attribute at the end
    if (batch->vertex_stride != 11) {
        printf("Error: vertex stride too small to hold color attribute\n");
        return;
    }
    // the range represents the arrows mesh, color each axis arrow differently
    // the offset is the vertex offset for each axis arrow there should be 13 vertices per arrow
    // each face counts as 3 vertices, each arrow has 18 faces = 54 vertices
    if (range.vertex_count != 162) { // testing with single arrow mesh
        printf("Error: arrow mesh should have 162 vertices (54 per axis)\n");
        printf("Got %u vertices\n", range.vertex_count);
        return;
    }
    for (uint32_t i = 0; i < range.vertex_count; i++) {
        uint32_t vi = range.vertex_start + i;
        // first 13 vertices should be colored with color_x
        if (i < 54) {
            batch->vertex_data[vi * batch->vertex_stride + 8] = color_x.r;
            batch->vertex_data[vi * batch->vertex_stride + 9] = color_x.g;
            batch->vertex_data[vi * batch->vertex_stride + 10] = color_x.b;
        }
        // next 54 vertices should be colored with color_y
        else if (i < 108) {
            batch->vertex_data[vi * batch->vertex_stride + 8] = color_y.r;
            batch->vertex_data[vi * batch->vertex_stride + 9] = color_y.g;
            batch->vertex_data[vi * batch->vertex_stride + 10] = color_y.b;
        }
        // last 54 vertices should be colored with color_z
        else {
            batch->vertex_data[vi * batch->vertex_stride + 8] = color_z.r;
            batch->vertex_data[vi * batch->vertex_stride + 9] = color_z.g;
            batch->vertex_data[vi * batch->vertex_stride + 10] = color_z.b;
        }

    }

}

void write_mip_maps(WGPUDevice device, WGPUTexture texture, WGPUExtent3D size, uint32_t mip_level_count, unsigned char * pixel_data) {

    WGPUTexelCopyTextureInfo destination = {0};
    destination.texture = texture;
    destination.mipLevel = 0;
    destination.origin = (WGPUOrigin3D){0, 0, 0};
    destination.aspect = WGPUTextureAspect_All;

    WGPUTexelCopyBufferLayout source = {0};
    source.offset = 0;
    source.bytesPerRow = 4 * size.width;
    source.rowsPerImage = size.height;

    WGPUQueue queue = wgpuDeviceGetQueue(device);
    wgpuQueueWriteTexture(queue, &destination, pixel_data, 4 * size.width * size.height, &source, &size);
    wgpuQueueRelease(queue);
}
WGPUTexture load_texture(char const * filename, WGPUDevice device, WGPUTextureView * texture_view) {
    int width, height, channels;
    unsigned char * pixel_data = stbi_load(filename, &width, &height, &channels, STBI_rgb_alpha);
    if (!pixel_data) {
        printf("Failed to load texture image: %s\n", filename);
        return NULL;
    }

    WGPUTextureDescriptor texture_desc = {0};
    texture_desc.dimension = WGPUTextureDimension_2D;
    texture_desc.format = WGPUTextureFormat_RGBA8Unorm; // by convention for bmp, png and jpg file. Be careful with other formats.
    texture_desc.mipLevelCount = 1;
    texture_desc.sampleCount = 1;
    texture_desc.size = (WGPUExtent3D){ (unsigned int)width, (unsigned int)height, 1 };
    texture_desc.usage = WGPUTextureUsage_TextureBinding | WGPUTextureUsage_CopyDst;
    texture_desc.viewFormatCount = 0;
    texture_desc.viewFormats = NULL;
    WGPUTexture texture = wgpuDeviceCreateTexture(device, &texture_desc);

    write_mip_maps(device, texture, texture_desc.size, texture_desc.mipLevelCount, pixel_data);

    stbi_image_free(pixel_data);

    if (texture_view) {
        WGPUTextureViewDescriptor view_desc = {0};
        view_desc.aspect = WGPUTextureAspect_All;
        view_desc.baseArrayLayer = 0;
        view_desc.arrayLayerCount = 1;
        view_desc.baseMipLevel = 0;
        view_desc.mipLevelCount = texture_desc.mipLevelCount;
        view_desc.dimension = WGPUTextureViewDimension_2D;
        view_desc.format = texture_desc.format;
        *texture_view = wgpuTextureCreateView(texture, &view_desc);
    }

    return texture;
}


struct VertexAttributes{
    float position[3];
    float texcoord[2];
    float normal[3];
};

typedef struct {
    int v_idx;
    int vt_idx;
    int vn_idx;
} VertexKey;

static size_t hash_vertex(VertexKey const * key) {
    size_t h = 14695981039346656037ULL; // FNV offset
    h ^= (size_t)key->v_idx;  h *= 1099511628211ULL;
    h ^= (size_t)key->vn_idx; h *= 1099511628211ULL;
    h ^= (size_t)key->vt_idx; h *= 1099511628211ULL;
    return h;
}
typedef struct VertexEntry {
    VertexKey key;
    uint32_t index;
    struct VertexEntry * next;
}  VertexEntry;
// #define HASH_SIZE 131071 // a large prime number
#define HASH_SIZE 59 // a prime number
typedef struct {
    VertexEntry * table[HASH_SIZE];
} VertexHash;
static void vertex_hash_init(VertexHash * hash) {
    memset(hash->table, 0, sizeof(hash->table));
}
static uint32_t vertex_hash_get_or_add(VertexHash * hash, VertexKey key, struct VertexAttributes * vertex_array, uint32_t * vertex_count) {
    size_t bucket = hash_vertex(&key) % HASH_SIZE;
    //printf("Hashed key (v_idx=%d, vt_idx=%d, vn_idx=%d) to bucket %zu\n", key.v_idx, key.vt_idx, key.vn_idx, bucket);
    VertexEntry * entry = hash->table[bucket];

    // search for existing entry
    while (entry) {
        if (entry->key.v_idx == key.v_idx &&
            entry->key.vn_idx == key.vn_idx &&
            entry->key.vt_idx == key.vt_idx) {
            return entry->index;
        }
        entry = entry->next;
    }
    // not found, add new entry
    uint32_t new_index = (uint32_t)(*vertex_count);

    VertexEntry * new_entry = (VertexEntry *)malloc(sizeof(VertexEntry));
    new_entry->key = key;
    new_entry->index = new_index;
    new_entry->next = hash->table[bucket];
    hash->table[bucket] = new_entry;

    (*vertex_count)++;

    return new_index;
}
static void vertex_hash_free(VertexHash * hash) {
    for (size_t i = 0; i < HASH_SIZE; i++) {
        VertexEntry * entry = hash->table[i];
        while (entry) {
            VertexEntry * next = entry->next;
            free(entry);
            entry = next;
        }
        hash->table[i] = NULL;
    }
}

struct MeshRange{
    uint32_t index_offset;
    uint32_t index_count;
    uint32_t vertex_start;
    uint32_t vertex_count;
};
struct MeshBatch{
    struct VertexAttributes * vertex_data;
    uint32_t * index_data;

    struct MeshRange * meshes;

    uint32_t vertex_count;
    uint32_t index_count;
    uint32_t mesh_count;
};
struct Mesh {
    struct VertexAttributes * vertices;
    uint32_t vertex_count;
    uint32_t * indices;
    uint32_t index_count;
};



// File reader callback for tinyobj with Emscripten
void emscripten_file_reader2(void *ctx, const char *filename, int is_mtl, const char *obj_filename, char **buf, size_t *len) {
    (void)ctx; // unused
    (void)is_mtl; // unused
    (void)obj_filename; // unused
    FILE *file = fopen(filename, "rb");
    if (!file) {
        printf("Failed to open file: %s\n", filename);
        *buf = NULL;
        *len = 0;
        return;
    }
    // Get file size
    fseek(file, 0, SEEK_END);
    *len = (size_t)ftell(file);
    fseek(file, 0, SEEK_SET);
    // Allocate buffer and read file
    printf("Allocating %zu bytes for file: %s\n", *len, filename);

    *buf = (char*)malloc(*len + 1);
    if (*buf) {
        size_t read_size = fread(*buf, 1, *len, file);
        (*buf)[read_size] = '\0'; // Null terminate
        *len = read_size;
    } else {
        *len = 0;
    }
    fclose(file);
}

struct Mesh loadGeometryFromObj(char const * filename) {
    struct Mesh mesh = {0};
    VertexHash hash = {0};
    vertex_hash_init(&hash);

    tinyobj_attrib_t attrib;
    tinyobj_shape_t * shapes = NULL;
    size_t num_shapes = 0;
    tinyobj_material_t * materials = NULL;
    size_t num_materials = 0;
    int ret = tinyobj_parse_obj(&attrib, &shapes, &num_shapes, &materials, &num_materials, filename, emscripten_file_reader2, NULL, TINYOBJ_FLAG_TRIANGULATE);
    if (ret != TINYOBJ_SUCCESS) {
        printf("Failed to load OBJ file: %s\n", filename);
        return mesh;
    }

    // print info
    printf("Loaded OBJ file: %s\n", filename);
    printf("  Vertices: %u\n", attrib.num_vertices);
    printf("  Normals: %u\n", attrib.num_normals);
    printf("  Texcoords: %u\n", attrib.num_texcoords);
    printf("  Faces: %u\n", attrib.num_faces / 3); // attrib.num_faces is actually number of indices where each face has 3 indices after triangulation

    // assume one shape
    if (num_shapes != 1) {
        printf("Warning: Expected one shape in OBJ file, found %zu\n", num_shapes);
    }

    uint32_t mesh_vertex_capacity = 64;
    mesh.vertices = (struct VertexAttributes*)malloc(sizeof(struct VertexAttributes) * mesh_vertex_capacity);
    
    mesh.vertex_count = 0;

    mesh.indices = (uint32_t*)malloc(sizeof(uint32_t) * attrib.num_faces);
    mesh.index_count = attrib.num_faces;

    // copy data to vertex buffer
    for (size_t i = 0; i < attrib.num_faces; i++) {
        tinyobj_vertex_index_t idx = attrib.faces[i];
        struct VertexAttributes vertex = {0};
        if (idx.v_idx != (int)TINYOBJ_INVALID_INDEX) {
            vertex.position[0] = attrib.vertices[idx.v_idx * 3 + 0];
            vertex.position[1] = attrib.vertices[idx.v_idx * 3 + 1];
            vertex.position[2] = attrib.vertices[idx.v_idx * 3 + 2];
        }
        if (idx.vt_idx != (int)TINYOBJ_INVALID_INDEX) {
            vertex.texcoord[0] = attrib.texcoords[idx.vt_idx * 2 + 0];
            vertex.texcoord[1] = attrib.texcoords[idx.vt_idx * 2 + 1];
        }
        if (idx.vn_idx != (int)TINYOBJ_INVALID_INDEX) {
            vertex.normal[0] = attrib.normals[idx.vn_idx * 3 + 0];
            vertex.normal[1] = attrib.normals[idx.vn_idx * 3 + 1];
            vertex.normal[2] = attrib.normals[idx.vn_idx * 3 + 2];
        }
        VertexKey key = {idx.v_idx, idx.vt_idx, idx.vn_idx};
        uint32_t index = vertex_hash_get_or_add(&hash, key, mesh.vertices, &mesh.vertex_count);
        if (index == mesh.vertex_count - 1) {
            // new vertex added
            if (mesh.vertex_count >= mesh_vertex_capacity) {
                mesh_vertex_capacity *= 2;
                mesh.vertices = (struct VertexAttributes*)realloc(mesh.vertices, sizeof(struct VertexAttributes) * mesh_vertex_capacity);
            }
            if (index >= mesh_vertex_capacity) {
                printf("Error: Vertex index %u exceeds capacity %u\n", index, mesh_vertex_capacity);
            }
            mesh.vertices[index] = vertex;
        }
        mesh.indices[i] = (uint32_t)index;
    }
    vertex_hash_free(&hash);



    tinyobj_attrib_free(&attrib);
    tinyobj_shapes_free(shapes, num_shapes);
    tinyobj_materials_free(materials, num_materials);
    return mesh;
}
void add_mesh_to_batch(struct MeshBatch * batch, struct Mesh * mesh) {
    
    batch->vertex_data = realloc(batch->vertex_data, sizeof(struct VertexAttributes) * (batch->vertex_count + mesh->vertex_count));
    if (!batch->vertex_data) {
        printf("Error: Failed to allocate memory for vertex data in batch\n");
        return;
    }
    memcpy(&batch->vertex_data[batch->vertex_count], mesh->vertices, sizeof(struct VertexAttributes) * mesh->vertex_count);
    batch->index_data = realloc(batch->index_data, sizeof(uint32_t) * (batch->index_count + mesh->index_count));
    if (!batch->index_data) {
        printf("Error: Failed to allocate memory for index data in batch\n");
        return;
    }
    for (uint32_t i = 0; i < mesh->index_count; i++) {
        batch->index_data[batch->index_count + i] = mesh->indices[i] + batch->vertex_count;
    }
    
    struct MeshRange range = {
        .index_offset = batch->index_count,
        .index_count = mesh->index_count,
        .vertex_start = batch->vertex_count,
        .vertex_count = mesh->vertex_count
    };

    batch->meshes = realloc(batch->meshes, sizeof(struct MeshRange) * (batch->mesh_count + 1));
    if (!batch->meshes) {
        printf("Error: Failed to allocate memory for mesh ranges in batch\n");
        return;
    }
    batch->meshes[batch->mesh_count] = range;
    
    batch->vertex_count += mesh->vertex_count;
    batch->index_count += mesh->index_count;
    batch->mesh_count += 1;

    printf("Added mesh to batch: vertices=%u, indices=%u\n", mesh->vertex_count, mesh->index_count);
}

void load_objects3(char const * filenames[], uint32_t file_count, struct mesh_batch * out) {
    struct MeshBatch batch = {0};
    for (uint32_t i = 0; i < file_count; i++) {
        struct Mesh mesh = loadGeometryFromObj(filenames[i]);
        add_mesh_to_batch(&batch, &mesh);

        struct mesh_range range = {0};
        range.index_count = batch.meshes[batch.mesh_count - 1].index_count;
        range.index_offset = batch.meshes[batch.mesh_count - 1].index_offset;
        range.vertex_start = batch.meshes[batch.mesh_count - 1].vertex_start;
        range.vertex_count = batch.meshes[batch.mesh_count - 1].vertex_count;

        // add new mesh range to out
        out->meshes = realloc(out->meshes, sizeof(struct mesh_range) * (out->mesh_count + 1));
        out->meshes[out->mesh_count] = range;
        out->mesh_count++;

        free(mesh.vertices);
        free(mesh.indices);
    }
    out->index_count = batch.index_count;
    out->vertex_count = batch.vertex_count;
    out->index_data = batch.index_data;
    out->vertex_data = (float *)batch.vertex_data;
    out->vertex_stride = sizeof(struct VertexAttributes) / sizeof(float);

}