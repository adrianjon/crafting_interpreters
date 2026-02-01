#include <webgpu/webgpu.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <math.h>
#include "gpu.h"
#include "utils.h"
#include "renderer.h"

#include "shader_manager.h"
#include "pipeline_manager.h"
#include "buffer_manager.h"
#include "asset_manager.h"
#include "bind_groups.h"

#define BACKGROUND_COLOR {0.1f, 0.15f, 0.1f, 1.0f}
#define DEFAULT_BUFFER_SIZE 1024  // 1 KB

static vertex_input_t vertex_data[DEFAULT_BUFFER_SIZE] = {0};
static size_t vertex_data_count = 0;
static uint32_t index_data[DEFAULT_BUFFER_SIZE] = {0};
static size_t index_data_count = 0;
static WGPUTextureView depth_texture_view = NULL;
static WGPURenderPipeline g_pipeline = NULL;
static WGPUBindGroup g_bind_group = NULL;
static WGPUCommandEncoder g_encoder = NULL;

static WGPURenderPassEncoder g_render_pass = NULL;

static struct renderer {
    WGPUDevice device;
    WGPUQueue queue;
    
    WGPUCommandEncoder encoder;
    WGPURenderPassEncoder render_pass;

    WGPUBuffer vertex_buffer;
    WGPUBuffer uniforms_buffer;

    // Triangle batching system with indexed drawing
    vertex_input_t* triangle_batch;
    size_t triangle_batch_count;
    size_t triangle_batch_capacity;
    
    // Index batching for indexed drawing
    uint32_t* index_batch;
    size_t index_batch_count;
    size_t index_batch_capacity;
    
    // Bind group management
    WGPUBindGroup uniform_bind_group;
    
    asset_manager_t asset_manager;
    shader_manager_t shader_manager;
    buffer_manager_t buffer_manager;
    bg_layout_manager_t bg_layout_manager;
    pipeline_manager_t pipeline_manager;
    
} g_renderer;

// Helper function to check if two vertices are equal (for deduplication)
static bool vertices_equal(const vertex_input_t* a, const vertex_input_t* b) {
    const float epsilon = 1e-6f;
    return (fabsf(a->position.x - b->position.x) < epsilon &&
            fabsf(a->position.y - b->position.y) < epsilon &&
            fabsf(a->position.z - b->position.z) < epsilon &&
            fabsf(a->normal.x - b->normal.x) < epsilon &&
            fabsf(a->normal.y - b->normal.y) < epsilon &&
            fabsf(a->normal.z - b->normal.z) < epsilon &&
            fabsf(a->color.x - b->color.x) < epsilon &&
            fabsf(a->color.y - b->color.y) < epsilon &&
            fabsf(a->color.z - b->color.z) < epsilon);
}

// Helper function to find existing vertex or add new one, returns index
static uint32_t find_or_add_vertex(const vertex_input_t* vertex) {
    // Search for existing vertex
    for (size_t i = 0; i < g_renderer.triangle_batch_count; i++) {
        if (vertices_equal(&g_renderer.triangle_batch[i], vertex)) {
            return (uint32_t)i;
        }
    }
    
    // Vertex not found, add it
    // Ensure we have capacity for one more vertex
    if (g_renderer.triangle_batch_count >= g_renderer.triangle_batch_capacity) {
        g_renderer.triangle_batch_capacity *= 2;
        g_renderer.triangle_batch = realloc(g_renderer.triangle_batch, 
            g_renderer.triangle_batch_capacity * sizeof(vertex_input_t));
        if (!g_renderer.triangle_batch) {
            printf("ERROR: Failed to reallocate vertex batch memory\n");
            return 0;
        }
    }
    
    // Add the new vertex
    g_renderer.triangle_batch[g_renderer.triangle_batch_count] = *vertex;
    return (uint32_t)g_renderer.triangle_batch_count++;
}

void renderer_init(void) {

    gpu_context_t * gpu_context = gpu_get_context();
    if (!gpu_context) {
        printf("ERROR: GPU context is not initialized!\n");
        return;
    }
    g_renderer.device = gpu_context->device;
    g_renderer.queue = gpu_context->queue;

    // Initialize triangle batching system with indexed drawing
    g_renderer.triangle_batch_capacity = 1024; // Start with capacity for 1024 vertices
    g_renderer.triangle_batch = malloc(g_renderer.triangle_batch_capacity * sizeof(vertex_input_t));
    g_renderer.triangle_batch_count = 0;
    
    // Initialize index batching
    g_renderer.index_batch_capacity = 3072; // Start with capacity for 3072 indices (1024 triangles * 3)
    g_renderer.index_batch = malloc(g_renderer.index_batch_capacity * sizeof(uint32_t));
    g_renderer.index_batch_count = 0;
    
    g_renderer.asset_manager = new_asset_manager();
    g_renderer.shader_manager = new_shader_manager();
    g_renderer.buffer_manager = new_buffer_manager();
    g_renderer.bg_layout_manager = new_layout_manager();
    g_renderer.pipeline_manager = new_pipeline_manager();

    
    //g_renderer.shader_manager->init_shaders(gpu_context);
    g_renderer.pipeline_manager->init_pipelines(gpu_context);
    g_renderer.buffer_manager->initialize_buffers(gpu_context);
    
    // Create bind group for uniforms this should be moved to bind_groups.c
    WGPUBindGroupLayout uniform_layout = g_renderer.bg_layout_manager->get_layout(BGL_GLOBAL_UNIFORMS);
    if (uniform_layout) {
        g_renderer.uniform_bind_group = wgpuDeviceCreateBindGroup(g_renderer.device, &(WGPUBindGroupDescriptor){
            .layout = uniform_layout,
            .entryCount = 1,
            .entries = &(WGPUBindGroupEntry){
                .binding = 0,
                .buffer = g_renderer.buffer_manager->get_buffer(BUFFER_UNIFORM),
                .offset = 0,
                .size = sizeof(my_uniforms_t),
            },
        });
    }
    
    printf("Renderer initialized.\n");
    
}
void renderer_begin_frame(WGPUTextureView current_view, double time) {
    (void)time; // Unused for now
    // Reset triangle and index batches for new frame

    // vertex_data_count = 0;
    // index_data_count = 0;

    g_encoder = wgpuDeviceCreateCommandEncoder(g_renderer.device, NULL);
    WGPURenderPassDescriptor render_pass_desc = {0};
    WGPURenderPassColorAttachment color_attachment = {0};
    color_attachment.view = current_view;
    color_attachment.loadOp = WGPULoadOp_Clear;
    color_attachment.storeOp = WGPUStoreOp_Store;
    color_attachment.clearValue = (WGPUColor)BACKGROUND_COLOR;
    color_attachment.depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;

    render_pass_desc.colorAttachmentCount = 1;
    render_pass_desc.colorAttachments = &color_attachment;

    WGPURenderPassDepthStencilAttachment depth_stencil_attachment = {0};
    depth_stencil_attachment.view = depth_texture_view; // TODO make global
    depth_stencil_attachment.depthClearValue = 1.0f;
    depth_stencil_attachment.depthLoadOp = WGPULoadOp_Clear;
    depth_stencil_attachment.depthStoreOp = WGPUStoreOp_Store;
    depth_stencil_attachment.depthReadOnly = false;
    depth_stencil_attachment.stencilLoadOp = WGPULoadOp_Undefined;
    depth_stencil_attachment.stencilStoreOp = WGPUStoreOp_Undefined;
    depth_stencil_attachment.stencilReadOnly = true;

    render_pass_desc.depthStencilAttachment = &depth_stencil_attachment;
    render_pass_desc.timestampWrites = NULL;
    
    g_render_pass = wgpuCommandEncoderBeginRenderPass(g_encoder, &render_pass_desc);


    // g_renderer.triangle_batch_count = 0;
    // g_renderer.index_batch_count = 0;
    
    // g_renderer.pipeline_manager->set_active_pipeline(PIPELINE_3D_PRIMARY); 

    // g_renderer.encoder = wgpuDeviceCreateCommandEncoder(g_renderer.device, NULL);
    // // Setup rendering commands here using the encoder and current_view
    // WGPURenderPassDescriptor render_pass_desc = {
    //     .colorAttachmentCount = 1,
    //     .colorAttachments = &(WGPURenderPassColorAttachment){
    //         .view = current_view,
    //         .loadOp = WGPULoadOp_Clear,
    //         .storeOp = WGPUStoreOp_Store,
    //         .clearValue = (WGPUColor)BACKGROUND_COLOR,
    //         .depthSlice = WGPU_DEPTH_SLICE_UNDEFINED,
    //     },
    //     .depthStencilAttachment = NULL,
    // };
    // g_renderer.render_pass = wgpuCommandEncoderBeginRenderPass(g_renderer.encoder, &render_pass_desc);
}

// to be done somewhere:
    // set pipeline
    // set bind groups
    // set buffers
    // draw calls
void renderer_draw_triangle(float const vertices[6], float const color[4]) {

    // Create the 3 vertices of this triangle
    vertex_input_t triangle_vertices[3] = {
        { 
            .position = {vertices[0], vertices[1], 1.0f}, 
            .normal = {0.0f, 0.0f, 1.0f}, // Default normal pointing towards camera
            .color = {color[0], color[1], color[2]},
        },
        { 
            .position = {vertices[2], vertices[3], 1.0f}, 
            .normal = {0.0f, 0.0f, 1.0f},
            .color = {color[0], color[1], color[2]},
        },
        { 
            .position = {vertices[4], vertices[5], 1.0f}, 
            .normal = {0.0f, 0.0f, 1.0f},
            .color = {color[0], color[1], color[2]},
        },
    };

    // Add vertices to batch and get their indices
    for (int i = 0; i < 3; i++) {
        if (vertex_data_count < DEFAULT_BUFFER_SIZE) {
            vertex_data[vertex_data_count++] = triangle_vertices[i];
            index_data[index_data_count++] = (uint32_t)(vertex_data_count - 1);
        } else {
            printf("ERROR: Vertex buffer overflow in renderer_draw_triangle\n");
            return;
        }
    }
    

    // // Ensure we have capacity for 3 more indices
    // if (g_renderer.index_batch_count + 3 > g_renderer.index_batch_capacity) {
    //     g_renderer.index_batch_capacity *= 2;
    //     g_renderer.index_batch = realloc(g_renderer.index_batch, 
    //         g_renderer.index_batch_capacity * sizeof(uint32_t));
    //     if (!g_renderer.index_batch) {
    //         printf("ERROR: Failed to reallocate index batch memory\n");
    //         return;
    //     }
    // }

    // // Find or add each vertex and store the indices
    // for (int i = 0; i < 3; i++) {
    //     uint32_t vertex_index = find_or_add_vertex(&triangle_vertices[i]);
    //     g_renderer.index_batch[g_renderer.index_batch_count + i] = vertex_index;
    // }
    // g_renderer.index_batch_count += 3;
}
void renderer_draw_mesh(mesh_t * mesh) {
    (void)mesh;
    // for (uint32_t i = 0; i < mesh->triangle_count; i++) {
    // }
    // wgpuRenderPassEncoderDrawIndexed()
}
void renderer_end_frame(WGPUTextureView current_view) {
    

    printf("g_pipeline: %p\n", (void*)g_pipeline);
    printf("g_render_pass: %p\n", (void*)g_render_pass);
    wgpuRenderPassEncoderSetPipeline(g_render_pass, g_pipeline);
    wgpuRenderPassEncoderSetVertexBuffer(g_render_pass, 0, g_renderer.vertex_buffer, 0, vertex_data_count * sizeof(vertex_input_t));
    wgpuRenderPassEncoderSetBindGroup(g_render_pass, 0, g_bind_group, 0, NULL);
    wgpuRenderPassEncoderDraw(g_render_pass, vertex_data_count, 1, 0, 0);
    wgpuRenderPassEncoderEnd(g_render_pass);
    wgpuRenderPassEncoderRelease(g_render_pass);

    wgpuTextureViewRelease(current_view);

    WGPUCommandBuffer command = wgpuCommandEncoderFinish(g_encoder, NULL);
    wgpuCommandEncoderRelease(g_encoder);
    wgpuQueueSubmit(g_renderer.queue, 1, &command);
    wgpuCommandBufferRelease(command);




    // // Debug output
    // printf("renderer_end_frame: triangle_batch_count = %zu, index_batch_count = %zu\n", 
    //        g_renderer.triangle_batch_count, g_renderer.index_batch_count);
    
    // // Only draw if we have indices to render
    // if (g_renderer.index_batch_count > 0 && g_renderer.triangle_batch_count > 0) {
    //     // Upload all accumulated vertices to vertex buffer
    //     size_t vertex_data_size = g_renderer.triangle_batch_count * sizeof(vertex_input_t);
    //     g_renderer.buffer_manager->write_buffer(
    //         g_renderer.queue, 
    //         BUFFER_VERTEX, 
    //         0, 
    //         g_renderer.triangle_batch, 
    //         vertex_data_size
    //     );
    //     printf("triangle_batch contents:\n");
    //     for (size_t i = 0; i < g_renderer.triangle_batch_count; i++) {
    //         printf(" Vertex %zu: Pos(%.2f, %.2f, %.2f) Normal(%.2f, %.2f, %.2f) Color(%.2f, %.2f, %.2f)\n",
    //                i,
    //                g_renderer.triangle_batch[i].position.x,
    //                g_renderer.triangle_batch[i].position.y,
    //                g_renderer.triangle_batch[i].position.z,
    //                g_renderer.triangle_batch[i].normal.x,
    //                g_renderer.triangle_batch[i].normal.y,
    //                g_renderer.triangle_batch[i].normal.z,
    //                g_renderer.triangle_batch[i].color.x,
    //                g_renderer.triangle_batch[i].color.y,
    //                g_renderer.triangle_batch[i].color.z);
    //     }
    //     printf("Uploaded %zu vertices to buffer\n", g_renderer.triangle_batch_count);
        
    //     // Upload all accumulated indices to index buffer
    //     size_t index_data_size = g_renderer.index_batch_count * sizeof(uint32_t);
    //     g_renderer.buffer_manager->write_buffer(
    //         g_renderer.queue, 
    //         BUFFER_INDEX, 
    //         0, 
    //         g_renderer.index_batch, 
    //         index_data_size
    //     );
    //     printf("index_batch contents:\n");
    //     for (size_t i = 0; i < g_renderer.index_batch_count; i++) {
    //         printf(" Index %zu: %u\n", i, g_renderer.index_batch[i]);
    //     }
    //     printf("Uploaded %zu indices to buffer\n", g_renderer.index_batch_count);
        
    //     // Create proper 2D projection matrix (orthographic)
    //     // Assuming normalized device coordinates from -1 to 1
    //     my_uniforms_t uniforms = {
    //         .projection = {
    //             {1.0f, 0.0f, 0.0f, 0.0f},  // Scale X by 1
    //             {0.0f, 1.0f, 0.0f, 0.0f},  // Scale Y by 1  
    //             {0.0f, 0.0f, 1.0f, 0.0f},  // Scale Z by 1
    //             {0.0f, 0.0f, 0.0f, 1.0f}   // No translation
    //         },
    //         .view = {
    //             {1.0f, 0.0f, 0.0f, 0.0f},  // Identity view matrix
    //             {0.0f, 1.0f, 0.0f, 0.0f},
    //             {0.0f, 0.0f, 1.0f, 0.0f},
    //             {0.0f, 0.0f, 0.0f, 1.0f}
    //         },
    //         .model = {
    //             {1.0f, 0.0f, 0.0f, 0.0f},  // Identity model matrix
    //             {0.0f, 1.0f, 0.0f, 0.0f},
    //             {0.0f, 0.0f, 1.0f, 0.0f},
    //             {0.0f, 0.0f, 0.0f, 1.0f}
    //         },
    //         .color = {1.0f, 1.0f, 1.0f, 1.0f},  // White tint
    //         .time = 0.0f
    //     };
        
    //     // Upload uniform data
    //     g_renderer.buffer_manager->write_buffer(
    //         g_renderer.queue, 
    //         BUFFER_UNIFORM, 
    //         0, 
    //         &uniforms, 
    //         sizeof(my_uniforms_t)
    //     );
    //     printf("Uploaded uniforms\n");
        
    //     // Set the pipeline
    //     WGPURenderPipeline pipeline = g_renderer.pipeline_manager->get_active_pipeline();
    //     printf("Active pipeline: %p\n", (void*)pipeline);
        
    //     if (pipeline) {
    //         wgpuRenderPassEncoderSetPipeline(g_renderer.render_pass, pipeline);
    //         printf("Set pipeline\n");
            
    //         // Set vertex buffer
    //         WGPUBuffer vertex_buffer = g_renderer.buffer_manager->get_buffer(BUFFER_VERTEX);
    //         printf("Vertex buffer: %p\n", (void*)vertex_buffer);
            
    //         wgpuRenderPassEncoderSetVertexBuffer(
    //             g_renderer.render_pass, 
    //             0, 
    //             vertex_buffer, 
    //             0, 
    //             WGPU_WHOLE_SIZE
    //         );
    //         printf("Set vertex buffer\n");
            
    //         // Set index buffer
    //         WGPUBuffer index_buffer = g_renderer.buffer_manager->get_buffer(BUFFER_INDEX);
    //         printf("Index buffer: %p\n", (void*)index_buffer);
            
            
    //         wgpuRenderPassEncoderSetIndexBuffer(
    //             g_renderer.render_pass,
    //             index_buffer,
    //             WGPUIndexFormat_Uint32,
    //             0,
    //             WGPU_WHOLE_SIZE
    //         );
    //         printf("Set index buffer\n");
            
    //         // Set bind group for uniforms
    //         if (g_renderer.uniform_bind_group) {
    //             wgpuRenderPassEncoderSetBindGroup(
    //                 g_renderer.render_pass, 
    //                 0, 
    //                 g_renderer.uniform_bind_group, 
    //                 0, 
    //                 NULL
    //             );
    //             printf("Set bind group\n");
    //         } else {
    //             printf("WARNING: No uniform bind group!\n");
    //         }
            
    //         // Draw all triangles using indexed drawing
    //         wgpuRenderPassEncoderDrawIndexed(
    //             g_renderer.render_pass, 
    //             g_renderer.index_batch_count,  // indexCount 
    //             1,                             // instanceCount
    //             0,                             // firstIndex
    //             0,                             // baseVertex
    //             0                              // firstInstance
    //         );
    //         printf("Drew %zu indices (%zu triangles) using indexed drawing\n", 
    //                g_renderer.index_batch_count, g_renderer.index_batch_count / 3);
    //     } else {
    //         printf("WARNING: No active pipeline set, skipping draw\n");
    //     }
    // } else {
    //     printf("No triangles to render (vertices: %zu, indices: %zu)\n", 
    //            g_renderer.triangle_batch_count, g_renderer.index_batch_count);
    // }
    // // End render pass and submit command buffer
    // wgpuRenderPassEncoderEnd(g_renderer.render_pass);
    // wgpuRenderPassEncoderRelease(g_renderer.render_pass);

    // WGPUCommandBuffer command_buffer = wgpuCommandEncoderFinish(g_renderer.encoder, NULL);
    // wgpuCommandEncoderRelease(g_renderer.encoder);

    // wgpuQueueSubmit(g_renderer.queue, 1, &command_buffer);
    // wgpuCommandBufferRelease(command_buffer);

    // wgpuTextureViewRelease(current_view);
}

// render_context_t * t(render_context_t * ctx, float const vertices[6], float const color[4]) {
//     renderer_draw_triangle(vertices, color);
//     return ctx;
// }
// render_context_t * e(render_context_t * ctx, WGPUTextureView current_view) {
//     renderer_end_frame(current_view);
//     return ctx;
// }
// render_context_t * renderer_begin_frame2(render_context_t * ctx, WGPUTextureView current_view) {
//     renderer_begin_frame(current_view, 0.0);

//     ctx->renderer_draw_triangle = t;
//     ctx->renderer_end_frame = e;
//     return ctx;
// }

renderer_t * renderer_get_renderer(void) {
    return &g_renderer;
}

void extra_render_init_all(void) {

    new_shader_manager();
    
    gpu_context_t * gpu_context = gpu_get_context();
    g_renderer.device = gpu_context->device;
    g_renderer.queue = gpu_context->queue;

    WGPURenderPipelineDescriptor pipeline_desc = {0};
    WGPUVertexAttribute attributes[3] = {0};
    // Position
    attributes[0].shaderLocation = 0;
    attributes[0].format = WGPUVertexFormat_Float32x3;
    attributes[0].offset = offsetof(vertex_input_t, position);
    // Normal
    attributes[1].shaderLocation = 1;
    attributes[1].format = WGPUVertexFormat_Float32x3;
    attributes[1].offset = offsetof(vertex_input_t, normal);
    // Color
    attributes[2].shaderLocation = 2;
    attributes[2].format = WGPUVertexFormat_Float32x3;
    attributes[2].offset = offsetof(vertex_input_t, color);

    WGPUVertexBufferLayout vertex_layout = {0};
    vertex_layout.arrayStride = sizeof(vertex_input_t);
    vertex_layout.stepMode = WGPUVertexStepMode_Vertex;
    vertex_layout.attributeCount = 3;
    vertex_layout.attributes = attributes;

    pipeline_desc.vertex.bufferCount = 1;
    pipeline_desc.vertex.buffers = &vertex_layout;

    shader_t * vertex_shader = shader_file_to_module("/demo/shader.wgsl", g_renderer.device);
    pipeline_desc.vertex.module = shader_get_module(vertex_shader);
    pipeline_desc.vertex.entryPoint = toWGPUStringView("vs_main");
    
    pipeline_desc.primitive.topology = WGPUPrimitiveTopology_TriangleList;
    pipeline_desc.primitive.stripIndexFormat = WGPUIndexFormat_Undefined;
    pipeline_desc.primitive.frontFace = WGPUFrontFace_CCW;
    pipeline_desc.primitive.cullMode = WGPUCullMode_None;

    WGPUFragmentState fragment_state = {0};
    pipeline_desc.fragment = &fragment_state;
    fragment_state.module = shader_get_module(vertex_shader);
    fragment_state.entryPoint = toWGPUStringView("fs_main");

    

    WGPUBlendState blend_state = {0};
    blend_state.color.srcFactor = WGPUBlendFactor_SrcAlpha;
    blend_state.color.dstFactor = WGPUBlendFactor_OneMinusSrcAlpha;
    blend_state.color.operation = WGPUBlendOperation_Add;
    blend_state.alpha.srcFactor = WGPUBlendFactor_Zero;
    blend_state.alpha.dstFactor = WGPUBlendFactor_One;
    blend_state.alpha.operation = WGPUBlendOperation_Add;

    WGPUColorTargetState color_target = {0};
    color_target.format = gpu_context->surface_format;
    color_target.blend = &blend_state;
    color_target.writeMask = WGPUColorWriteMask_All;

    fragment_state.targetCount = 1;
    fragment_state.targets = &color_target;

    WGPUDepthStencilState depth_stencil = {0};
    depth_stencil.depthCompare = WGPUCompareFunction_Less;
    depth_stencil.depthWriteEnabled = true;
    WGPUTextureFormat depthTextureFormat = WGPUTextureFormat_Depth24Plus;
    depth_stencil.format = depthTextureFormat;
    depth_stencil.stencilReadMask = 0;
    depth_stencil.stencilWriteMask = 0;

    pipeline_desc.depthStencil = &depth_stencil;
    pipeline_desc.multisample.count = 1;
    pipeline_desc.multisample.mask = ~0;

    WGPUBindGroupLayoutEntry binding_layout = {0};
    binding_layout.binding = 0;
    binding_layout.visibility = WGPUShaderStage_Vertex | WGPUShaderStage_Fragment;
    binding_layout.buffer.type = WGPUBufferBindingType_Uniform;
    binding_layout.buffer.minBindingSize = sizeof(my_uniforms_t);

    WGPUBindGroupLayoutDescriptor bg_layout_desc = {0};
    bg_layout_desc.entryCount = 1;
    bg_layout_desc.entries = &binding_layout;
    WGPUBindGroupLayout bind_group_layout = wgpuDeviceCreateBindGroupLayout(g_renderer.device, &bg_layout_desc);

    WGPUPipelineLayoutDescriptor layout_desc = {0};
    layout_desc.bindGroupLayoutCount = 1;
    layout_desc.bindGroupLayouts = &bind_group_layout;
    WGPUPipelineLayout layout = wgpuDeviceCreatePipelineLayout(g_renderer.device, &layout_desc);
    pipeline_desc.layout = layout;

    g_pipeline = wgpuDeviceCreateRenderPipeline(g_renderer.device, &pipeline_desc);
    printf("Pipeline created: %p\n", (void*)g_pipeline);

    

    WGPUTextureDescriptor depth_texture_desc = {0};
    depth_texture_desc.dimension = WGPUTextureDimension_2D;
    depth_texture_desc.format = depthTextureFormat;
    depth_texture_desc.mipLevelCount = 1;
    depth_texture_desc.sampleCount = 1;
    depth_texture_desc.size.height = 1025;
    depth_texture_desc.size.width = 1512;
    depth_texture_desc.size.depthOrArrayLayers = 1;
    depth_texture_desc.usage = WGPUTextureUsage_RenderAttachment;
    depth_texture_desc.viewFormatCount = 1;
    depth_texture_desc.viewFormats = (WGPUTextureFormat*)&depthTextureFormat;
    WGPUTexture depth_texture = wgpuDeviceCreateTexture(g_renderer.device, &depth_texture_desc);
    printf("Depth texture created: %p\n", (void*)depth_texture);

    WGPUTextureViewDescriptor depth_view_desc = {0};
    depth_view_desc.aspect = WGPUTextureAspect_DepthOnly;
    depth_view_desc.baseArrayLayer = 0;
    depth_view_desc.arrayLayerCount = 1;
    depth_view_desc.baseMipLevel = 0;
    depth_view_desc.mipLevelCount = 1;
    depth_view_desc.dimension = WGPUTextureViewDimension_2D;
    depth_view_desc.format = depthTextureFormat;
    depth_texture_view = wgpuTextureCreateView(depth_texture, &depth_view_desc);
    printf("Depth texture view created: %p\n", (void*)depth_texture_view);


    // load data into CPU-side buffers, done in renderer_draw_triangle

    WGPUBufferDescriptor buffer_desc = {0};
    buffer_desc.size = DEFAULT_BUFFER_SIZE * sizeof(vertex_input_t);
    buffer_desc.usage = WGPUBufferUsage_Vertex | WGPUBufferUsage_CopyDst;
    buffer_desc.mappedAtCreation = false;
    g_renderer.vertex_buffer = wgpuDeviceCreateBuffer(g_renderer.device, &buffer_desc);
    printf("Vertex buffer created: %p\n", (void*)g_renderer.vertex_buffer);


    renderer_draw_triangle((float[]){
        (0.0f), (0.5f),
        (-0.5f), (-0.5f),
        (0.5f), (-0.5f)
    }, (float[]){1.0f, 1.0f, 0.0f, 1.0f});
    wgpuQueueWriteBuffer(g_renderer.queue, g_renderer.vertex_buffer, 0, vertex_data, vertex_data_count * sizeof(vertex_input_t));
    printf("Wrote %zu vertices to vertex buffer\n", vertex_data_count);

    buffer_desc.size = sizeof(my_uniforms_t);
    buffer_desc.usage = WGPUBufferUsage_Uniform | WGPUBufferUsage_CopyDst;
    g_renderer.uniforms_buffer = wgpuDeviceCreateBuffer(g_renderer.device, &buffer_desc);
    printf("Uniforms buffer created: %p\n", (void*)g_renderer.uniforms_buffer);

    my_uniforms_t initial_uniforms = {
        .projection = {
            {1.0f, 0.0f, 0.0f, 0.0f},
            {0.0f, 1.0f, 0.0f, 0.0f},
            {0.0f, 0.0f, 1.0f, 0.0f},
            {0.0f, 0.0f, 0.0f, 1.0f}
        },
        .view = {
            {1.0f, 0.0f, 0.0f, 0.0f},
            {0.0f, 1.0f, 0.0f, 0.0f},
            {0.0f, 0.0f, 1.0f, 0.0f},
            {0.0f, 0.0f, 0.0f, 1.0f}
        },
        .model = {
            {1.0f, 0.0f, 0.0f, 0.0f},
            {0.0f, 1.0f, 0.0f, 0.0f},
            {0.0f, 0.0f, 1.0f, 0.0f},
            {0.0f, 0.0f, 0.0f, 1.0f}
        },
        .color = {1.0f, 1.0f, 1.0f, 1.0f},
        .time = 0.0f,
        ._padding = {0},
    };
    wgpuQueueWriteBuffer(g_renderer.queue, g_renderer.uniforms_buffer, 0, &initial_uniforms, sizeof(my_uniforms_t));

    WGPUBindGroupEntry bg_entry = {0};
    bg_entry.binding = 0;
    bg_entry.buffer = g_renderer.uniforms_buffer;
    bg_entry.offset = 0;
    bg_entry.size = sizeof(my_uniforms_t);

    WGPUBindGroupDescriptor bg_desc = {0};
    bg_desc.layout = bind_group_layout;
    bg_desc.entryCount = bg_layout_desc.entryCount;
    bg_desc.entries = &bg_entry;
    g_bind_group = wgpuDeviceCreateBindGroup(g_renderer.device, &bg_desc);
    printf("Bind group created: %p\n", (void*)g_bind_group);
}