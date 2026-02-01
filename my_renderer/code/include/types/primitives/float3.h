#ifndef MY_RENDERER_FLOAT3_H
#define MY_RENDERER_FLOAT3_H

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

static inline float _clamp(float t, float min, float max) {
    return (t < min) ? min : (t > max) ? max : t;
}

///////////////////////////////////////////////////////////////////////////////////
typedef union {
    struct { float x, y; };
    float data[2];
} float2;

static const float2 FLOAT2_ZERO = {{0.0f, 0.0f}};
static const float2 FLOAT2_ONE  = {{1.0f, 1.0f}};
static const float2 FLOAT2_HALF = {{0.5f, 0.5f}};
static const float2 FLOAT2_RIGHT = {{1.0f, 0.0f}};
static const float2 FLOAT2_UP = {{0.0f, 1.0f}};
static inline float float2_dot(float2 const a, float2 const b) {
    return a.x * b.x + a.y * b.y;
}

static inline float2 float2_lerp(float2 const a, float2 const b, float t) {
    t = _clamp(t, 0.0f, 1.0f);  
    float2 result = {{
        a.x + (b.x - a.x) * t,
        a.y + (b.y - a.y) * t,
    }};
    return result;
}
static inline float2 float2_add(float2 const a, float2 const b) {
    return (float2){{a.x + b.x, a.y + b.y}};
}
static inline float2 float2_sub(float2 const a, float2 const b) {
    return (float2){{a.x - b.x, a.y - b.y}};
}
static inline float2 float2_mul_scalar(float2 const v, float s) {
    return (float2){{v.x * s, v.y * s}};
}
static inline float2 float2_div_scalar(float2 const v, float s) {
    return float2_mul_scalar(v, 1.0f / s);
}
static inline float2 float2_neg(float2 const v) {
    return (float2){{-v.x, -v.y}};
}




// Print
static inline char * float2_to_string(float2 const v, char * buffer, size_t buffer_size) {
    size_t n = snprintf(buffer, buffer_size, "<%f, %f>", v.x, v.y);
    if (n >= buffer_size) {
        fprintf(stderr, "Buffer too small for float2 string representation\n");
        exit(EXIT_FAILURE);
    }
    return buffer;
}
static inline void float2_print(float2 const v) {
    printf("<%f, %f>\n", v.x, v.y);
}

///////////////////////////////////////////////////////////////////////////////////
typedef union {
    struct { float x, y, z; };
    struct { float r, g, b; };
    float data[3];
} float3;

// Constant vectors
static const float3 FLOAT3_ZERO = {{0.0f, 0.0f, 0.0f}};
static const float3 FLOAT3_ONE  = {{1.0f, 1.0f, 1.0f}};
static const float3 FLOAT3_RIGHT = {{1.0f, 0.0f, 0.0f}};
static const float3 FLOAT3_UP    = {{0.0f, 1.0f, 0.0f}};
static const float3 FLOAT3_FORWARD = {{0.0f, 0.0f, 1.0f}};

// Internal helpers (not in header)
static inline float _float3_sqr_magnitude(float3 const v) {
    return v.x * v.x + v.y * v.y + v.z * v.z;
}


// Public functions
static inline float float3_dot(float3 const a, float3 const b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}
static inline float float3_magnitude(float3 const v) {
    return sqrtf(_float3_sqr_magnitude(v));
}
static inline float3 float3_normalize(float3 const v) {
    float mag = float3_magnitude(v);
    if (mag == 0.0f) return FLOAT3_ZERO;
    float3 result = {{v.x / mag, v.y / mag, v.z / mag}};
    return result;
}
static inline float3 float3_cross(float3 const a, float3 const b) {
    float3 result = {{
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    }};
    return result;
}
static inline float3 float3_lerp(float3 const a, float3 const b, float t) {
    t = _clamp(t, 0.0f, 1.0f);  
    float3 result = {{
        a.x + (b.x - a.x) * t,
        a.y + (b.y - a.y) * t,
        a.z + (b.z - a.z) * t
    }};
    return result;
}
static inline float3 float3_add(float3 const a, float3 const b) {
    return (float3){{a.x + b.x, a.y + b.y, a.z + b.z}};
}
static inline float3 float3_sub(float3 const a, float3 const b) {
    return (float3){{a.x - b.x, a.y - b.y, a.z - b.z}};
}
static inline float3 float3_mul_scalar(float3 const v, float s) {
    return (float3){{v.x * s, v.y * s, v.z * s}};
}
static inline float3 float3_div_scalar(float3 const v, float s) {
    return float3_mul_scalar(v, 1.0f / s);
}
static inline float3 float3_neg(float3 const v) {
    return (float3){{-v.x, -v.y, -v.z}};
}

// Print
static inline char * float3_to_string(float3 const v, char * buffer, size_t buffer_size) {
    size_t n = snprintf(buffer, buffer_size, "<%f, %f, %f>", v.x, v.y, v.z);
    if (n >= buffer_size) {
        fprintf(stderr, "Buffer too small for float3 string representation\n");
        exit(EXIT_FAILURE);
    }
    return buffer;
}
static inline void float3_print(float3 const v) {
    printf("<%f, %f, %f>\n", v.x, v.y, v.z);
}

///////////////////////////////////////////////////////////////////////////////////

typedef union {
    struct { float x, y, z, w; };
    struct { float r, g, b, a; };
    float data[4];
} float4;
typedef union {
    struct {
        float4 row0;
        float4 row1;
        float4 row2;
        float4 row3;
    };
    float data[16];
} float4x4;

#endif // MY_RENDERER_FLOAT3_H