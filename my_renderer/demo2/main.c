#include <emscripten.h>
#include <emscripten/html5.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdio.h>

#define CANVAS_WIDTH 1024
#define CANVAS_HEIGHT 1024

typedef union {
    struct { float x, y; };
} float2_t;
static inline float2_t float2(float x, float y) {
    return (float2_t){{x, y}};
}
typedef union {
    struct { float x, y, z; };
    struct { float r, g, b; };
} float3_t;
static inline float3_t float3(float x, float y, float z) {
    return (float3_t){{x, y, z}};
}
typedef union {
    struct { float x, y, z, w; };
    struct { float r, g, b, a; };
} float4_t;
static inline float4_t float4(float x, float y, float z, float w) {
    return (float4_t){{x, y, z, w}};
}
//typedef float4_t float4x4_t[4];
typedef union {
    float4_t rows[4];
    float data[16];
} float4x4_t;
static inline float4x4_t float4x4(
    float m00, float m01, float m02, float m03,
    float m10, float m11, float m12, float m13,
    float m20, float m21, float m22, float m23,
    float m30, float m31, float m32, float m33
) {
    return (float4x4_t){{
        float4(m00, m01, m02, m03),
        float4(m10, m11, m12, m13),
        float4(m20, m21, m22, m23),
        float4(m30, m31, m32, m33)
    }};
}

// Pixel buffer for software rendering
static float3_t * pixel_buffer = NULL;

// static EM_JS(void, init_canvas_image_data, (int width, int height), {
//     const ctx = canvas.getContext('2d');
//     canvas.width = width;
//     canvas.height = height;
//     Module.imageData = ctx.createImageData(width, height);
//     Module.ctx = ctx;
// })
// Include the conversion and javaScript interop functions
static uint8_t * rgba_buffer = NULL;
// Copy buffer to canvas using JavaScript interop
EM_JS(void, copy_buffer_to_canvas, (uint8_t* buffer, int width, int height), {
    const canvas = document.getElementById('canvas');
    const ctx = canvas.getContext('2d');
    
    // Create ImageData from our pixel buffer
    const imageData = ctx.createImageData(width, height);
    const data = imageData.data;
    
    // Copy pixel data from WASM memory to ImageData
    const bufferView = HEAPU8.subarray(buffer, buffer + width * height * 4);
    data.set(bufferView);
    
    // Draw to canvas
    ctx.putImageData(imageData, 0, 0);
});
// Convert float3 RGB buffer to uint8_t RGBA buffer
void convert_to_rgba_buffer(uint32_t width, uint32_t height, float3_t * pixel_buffer  ) {
    for (uint32_t i = 0; i < width * height; i++) {
        uint32_t rgba_index = i * 4;
        // Convert float (0.0-1.0) to uint8_t (0-255) and add alpha
        rgba_buffer[rgba_index + 0] = (uint8_t)(pixel_buffer[i].r * 255.0f); // R
        rgba_buffer[rgba_index + 1] = (uint8_t)(pixel_buffer[i].g * 255.0f); // G
        rgba_buffer[rgba_index + 2] = (uint8_t)(pixel_buffer[i].b * 255.0f); // B
        rgba_buffer[rgba_index + 3] = 255; // A (fully opaque)
    }
}

typedef struct {
    float3_t * vertices;
    uint32_t vertex_count;
    float3_t * triangle_colors;
    uint32_t triangle_count;
} model_t;
model_t new_model(char const * filename);
void free_model(model_t * model);

typedef struct {
    uint32_t count;
    model_t * items;
} model_array_t;

static float utime = 0.0f;
static inline float dot(float2_t a, float2_t b) {
    return a.x * b.x + a.y * b.y;
}
static inline float2_t perpendicular(float2_t v) {
    return float2(v.y, -v.x);
}
static bool maths_point_on_right_side_of_line(float2_t a, float2_t b, float2_t p) {
    float2_t ap = float2(p.x - a.x, p.y - a.y);
    float2_t ab_perp = perpendicular(float2(b.x - a.x, b.y - a.y));
    return dot(ab_perp, ap) >= 0.0f;
}
static bool maths_points_in_triangle(float2_t a, float2_t b, float2_t c, float2_t p) {
    bool side_ab = maths_point_on_right_side_of_line(a, b, p);
    bool side_bc = maths_point_on_right_side_of_line(b, c, p);
    bool side_ca = maths_point_on_right_side_of_line(c, a, p);

    return (side_ab && side_bc && side_ca);
}
float3_t transform_to_world_x(float3_t world_pos) {
    float angle = -0.5f;
    float cos_angle = cosf(angle);
    float sin_angle = sinf(angle);
    float3_t basis_vectors[3] = {
        float3(1.0f, 0.0f, 0.0f), // X axis
        float3(0.0f, cos_angle, -sin_angle),  // Y axis
        float3(0.0f, sin_angle, cos_angle)   // Z axis
    };
    float3_t transformed_pos = float3(
        world_pos.x * basis_vectors[0].x + world_pos.y * basis_vectors[1].x + world_pos.z * basis_vectors[2].x,
        world_pos.x * basis_vectors[0].y + world_pos.y * basis_vectors[1].y + world_pos.z * basis_vectors[2].y,
        world_pos.x * basis_vectors[0].z + world_pos.y * basis_vectors[1].z + world_pos.z * basis_vectors[2].z
    );
    return transformed_pos;
}
float3_t transform_to_world(float3_t world_pos) {
    // rotate around Y axis
    float angle = utime * 2.0f; // fixed angle for demonstration
    float cos_angle = cosf(angle);
    float sin_angle = sinf(angle);
    float3_t basis_vectors[3] = {
        float3(cos_angle, 0.0f, -sin_angle), // X axis
        float3(0.0f, 1.0f, 0.0f),                // Y axis
        float3(sin_angle, 0.0f, cos_angle)   // Z axis
    };
    float3_t transformed_pos = float3(
        world_pos.x * basis_vectors[0].x + world_pos.y * basis_vectors[1].x + world_pos.z * basis_vectors[2].x,
        world_pos.x * basis_vectors[0].y + world_pos.y * basis_vectors[1].y + world_pos.z * basis_vectors[2].y,
        world_pos.x * basis_vectors[0].z + world_pos.y * basis_vectors[1].z + world_pos.z * basis_vectors[2].z
    );
    

    return transformed_pos;
}
float2_t world_to_screen(float3_t world_pos, uint32_t width, uint32_t height) {
    float3_t vertex_world = transform_to_world_x(transform_to_world(world_pos));
    float2_t screen_pos;
    screen_pos.x = (vertex_world.x + 1.0f) * 0.5f * (float)width;
    screen_pos.y = (1.0f - (vertex_world.y + 1.0f) * 0.5f) * (float)height;
    return screen_pos;
}
// Render the scene (called each frame)
void render_frame(model_array_t * models) {
    memset(pixel_buffer, 0, CANVAS_WIDTH * CANVAS_HEIGHT * sizeof(float3_t)); // Clear buffer

    for (uint32_t m = 0; m < models->count; m++) {
        model_t * model = &models->items[m];
        for (uint32_t i = 0; i < model->vertex_count; i += 3) {
            float2_t a = world_to_screen(model->vertices[i + 0], CANVAS_WIDTH, CANVAS_HEIGHT);
            float2_t b = world_to_screen(model->vertices[i + 1], CANVAS_WIDTH, CANVAS_HEIGHT);
            float2_t c = world_to_screen(model->vertices[i + 2], CANVAS_WIDTH, CANVAS_HEIGHT);

            // translate model to center of canvas down
            float offset_y = 0.0f;
            float offset_x = 0.0f;
            if (m == 1) {
                offset_y = 2.0f * CANVAS_HEIGHT;
                offset_x = 2.0f * CANVAS_WIDTH;
            }
            if (m == 2) {
                offset_y = -2.0f * CANVAS_HEIGHT;
                offset_x = -2.0f * CANVAS_WIDTH;
            }
            if (m == 3) {
                offset_y = 2.0f * CANVAS_HEIGHT;
                offset_x = -2.0f * CANVAS_WIDTH;
            }
            if (m == 4) {
                offset_y = -2.0f * CANVAS_HEIGHT;
                offset_x = 2.0f * CANVAS_WIDTH;
            }
            a.y += offset_y;
            b.y += offset_y;
            c.y += offset_y;
            a.x += offset_x;
            b.x += offset_x;
            c.x += offset_x;

            // scale model to fit canvas
            float scale = 0.15f;
            a.x = (a.x - CANVAS_WIDTH / 2) * scale + CANVAS_WIDTH / 2;
            a.y = (a.y - CANVAS_HEIGHT / 2) * scale + CANVAS_HEIGHT / 2;
            b.x = (b.x - CANVAS_WIDTH / 2) * scale + CANVAS_WIDTH / 2;
            b.y = (b.y - CANVAS_HEIGHT / 2) * scale + CANVAS_HEIGHT / 2;
            c.x = (c.x - CANVAS_WIDTH / 2) * scale + CANVAS_WIDTH / 2;
            c.y = (c.y - CANVAS_HEIGHT / 2) * scale + CANVAS_HEIGHT / 2;

            // Rasterize triangle
            // Compute bounding box
            int min_x = (int)fmaxf(0.0f, floorf(fminf(fminf(a.x, b.x), c.x)));
            int max_x = (int)fminf((float)(CANVAS_WIDTH - 1), ceilf(fmaxf(fmaxf(a.x, b.x), c.x)));
            int min_y = (int)fmaxf(0.0f, floorf(fminf(fminf(a.y, b.y), c.y)));
            int max_y = (int)fminf((float)(CANVAS_HEIGHT - 1), ceilf(fmaxf(fmaxf(a.y, b.y), c.y)));

            for (int y = min_y; y <= max_y; y++) {
                for (int x = min_x; x <= max_x; x++) {
                    float2_t p = float2((float)x + 0.5f, (float)y + 0.5f);
                    if (maths_points_in_triangle(a, b, c, p)) {
                        pixel_buffer[y * CANVAS_WIDTH + x] = model->triangle_colors[i / 3];
                    }
                }
            }

        }
    }
    // float2_t a = float2(0.2f * CANVAS_WIDTH, 0.2f * CANVAS_HEIGHT);
    // float2_t b = float2(0.7f * CANVAS_WIDTH, 0.4f * CANVAS_HEIGHT);
    // float2_t c = float2(0.4f * CANVAS_WIDTH, 0.8f * CANVAS_HEIGHT);

    // for (int y = 0; y < CANVAS_HEIGHT; y++) {
    //     for (int x = 0; x < CANVAS_WIDTH; x++) {
    //         float2_t p = float2((float)x, (float)y);
    //         bool inside = maths_points_in_triangle(a, b, c, p);
    //         if (inside) {
    //             pixel_buffer[y * CANVAS_WIDTH + x] = float3(1.0f, 0.0f, 0.0f); // Red color
    //         } else {
    //             pixel_buffer[y * CANVAS_WIDTH + x] = float3(0.0f, 0.0f, 0.0f); // Black color
    //         }
    //     }
    // }
}
// Main loop function called by Emscripten
void main_loop(void * arg) {
    utime += 0.016f; // Simulate time progression (~60 FPS)
    render_frame((model_array_t *)arg);
    convert_to_rgba_buffer(CANVAS_WIDTH, CANVAS_HEIGHT, pixel_buffer);
    copy_buffer_to_canvas(rgba_buffer, CANVAS_WIDTH, CANVAS_HEIGHT);
}

int main(void) {
    float4x4_t identity_matrix = float4x4(
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    );
    // Initialize the pixel buffer
    emscripten_set_canvas_element_size("canvas", CANVAS_WIDTH, CANVAS_HEIGHT);
    pixel_buffer = malloc(CANVAS_WIDTH * CANVAS_HEIGHT * sizeof(float3_t));
    rgba_buffer = malloc(CANVAS_WIDTH * CANVAS_HEIGHT * 4 * sizeof(uint8_t)); // RGBA
    model_t cube = new_model("/demo2/cube.obj");
    model_array_t models = {
        .count = 5,
        .items = (model_t[5]){
            new_model("/demo2/dragon.obj"),
            cube,
            cube,
            cube,
            cube
        }
    };

    emscripten_set_main_loop_arg(main_loop, &models, 60, 1);
    
    return 0;
}

model_t new_model(char const * filename) {
    float3_t * data = NULL;
    uint32_t data_count = 0;
    uint32_t data_capacity = 0;
    float3_t * vertices = NULL;
    uint32_t vertex_count = 0;
    uint32_t vertex_capacity = 0;
    uint32_t triangle_count = 0;
    FILE * file = fopen(filename, "r");
    if (!file) {
        printf("Failed to open model file: %s\n", filename);
        return (model_t){0};
    }
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "v ", 2) == 0) {
            float3_t vertex;
            sscanf(line, "v %f %f %f", &vertex.x, &vertex.y, &vertex.z);
            if (data_count >= data_capacity) {
                data_capacity = data_capacity == 0 ? 4 : data_capacity * 2;
                data = realloc(data, data_capacity * sizeof(float3_t));
            }
            data[data_count++] = vertex;
        } else if (strncmp(line, "f ", 2) == 0) {
            uint32_t v1, v2, v3;
            uint32_t n1, n2, n3;
            sscanf(line, "f %u//%u %u//%u %u//%u", &v1, &n1, &v2, &n2, &v3, &n3);
            if (vertex_count + 3 >= vertex_capacity) {
                vertex_capacity = vertex_capacity == 0 ? 6 : vertex_capacity * 2;
                vertices = realloc(vertices, vertex_capacity * sizeof(float3_t));
            }
            vertices[vertex_count++] = data[v1 - 1];
            vertices[vertex_count++] = data[v2 - 1];
            vertices[vertex_count++] = data[v3 - 1];
            triangle_count++;
        }
    }
    fclose(file);
    free(data);
    float3_t * triangle_colors = malloc(triangle_count * sizeof(float3_t));
    srand(9); // Fixed seed for reproducibility
    for (uint32_t i = 0; i < triangle_count; i++) {
        // random color for each triangle
        triangle_colors[i] = float3((float)(rand() % 256) / 255.0f,
                                    (float)(rand() % 256) / 255.0f,
                                    (float)(rand() % 256) / 255.0f);
    }
    return (model_t){
        .vertices = vertices,
        .vertex_count = vertex_count,
        .triangle_colors = triangle_colors,
        .triangle_count = triangle_count
    };
}
void free_model(model_t * model) {
    if (!model) return;
    free(model->vertices);
    free(model->triangle_colors);
    model->vertices = NULL;
    model->triangle_colors = NULL;
    model->vertex_count = 0;
    model->triangle_count = 0;
}