#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>

#include <emscripten/emscripten.h>

#include "../include/gpu.h"
#include "../include/utils.h"

// ============================================================================
// Global State
// ============================================================================

static gpu_context_t g_gpu_context = {0};

// ============================================================================
// Forward Declarations
// ============================================================================

static void on_adapter_request_ended(WGPURequestAdapterStatus status, WGPUAdapter adapter, WGPUStringView message, void * userdata1, void * userdata2);
static void on_device_request_ended(WGPURequestDeviceStatus status, WGPUDevice device, WGPUStringView message, void * userdata1, void * userdata2);
WGPUAdapter requestAdapterSync(WGPUInstance instance, const WGPURequestAdapterOptions * options);
WGPUDevice requestDeviceSync(WGPUAdapter adapter, const WGPUDeviceDescriptor * descriptor);
static void on_device_error(const WGPUDevice * device, WGPUErrorType type, WGPUStringView message, void * userdata1, void * userdata2);
static void on_device_lost(const WGPUDevice * device, WGPUDeviceLostReason reason, WGPUStringView message, void * userdata1, void * userdata2);
static void set_error(const char* format, ...);

WGPUInstance create_instance(WGPU_NULLABLE WGPUInstanceDescriptor const * descriptor);
WGPUSurface create_surface(WGPUInstance instance, char const * canvas_selector);
WGPUAdapter create_adapter(WGPUInstance instance, WGPUSurface surface);
WGPUDevice create_device(WGPUAdapter adapter, WGPULimits * supported_limits, WGPULimits * required_limits);
WGPUTextureFormat configure_surface(WGPUDevice device, WGPUSurface surface, WGPUAdapter adapter, uint32_t width, uint32_t height);
WGPUQueue create_queue(WGPUDevice device);

static void print_limits(const WGPULimits* limits, bool verbose);

// ============================================================================
// Callback Structures for Async Operations
// ============================================================================

typedef struct {
        bool completed;
        bool success;
    WGPUAdapter adapter;
} adapter_request_data_t;

typedef struct {
    bool completed;
    bool success;
    WGPUDevice device;
} device_request_data_t;

// ============================================================================
// Public API Implementation
// ============================================================================

bool gpu_init(const gpu_init_config_t* config) {
    #ifndef __EMSCRIPTEN__
    if (!config || !config->window_handle) {
        set_error("Invalid configuration or missing window handle");
        return false;
    }
    #endif

    // Clear any previous state
    memset(&g_gpu_context, 0, sizeof(gpu_context_t));
    
    printf("[GPU] Initializing WebGPU context...\n");

    WGPUInstance instance = create_instance(NULL);
    g_gpu_context.surface = create_surface(instance, "#canvas");
    WGPUAdapter adapter = create_adapter(instance, g_gpu_context.surface);
    wgpuInstanceRelease(instance); // Release instance after adapter creation
    WGPULimits  supported_limits;
    wgpuAdapterGetLimits(adapter, &supported_limits);
    g_gpu_context.supported_limits = supported_limits;
    print_limits(&supported_limits, false);
    WGPULimits required_limits = supported_limits;
    g_gpu_context.device = create_device(adapter, &supported_limits, &required_limits);
    g_gpu_context.surface_format = configure_surface(g_gpu_context.device, g_gpu_context.surface, adapter, config->initial_width, config->initial_height);
    wgpuAdapterRelease(adapter); // Release adapter after device creation
    g_gpu_context.queue = create_queue(g_gpu_context.device);
    printf("[GPU] ✓ WebGPU initialization completed successfully!\n");
    g_gpu_context.is_initialized = true;
    return true;
}

void gpu_cleanup(void) {
    if (!g_gpu_context.is_initialized) {
        return; // Nothing to cleanup
    }
    
    printf("[GPU] Cleaning up WebGPU context...\n");
    
    // Release resources in reverse order of creation
    
    if (g_gpu_context.queue) {
        wgpuQueueRelease(g_gpu_context.queue);
        g_gpu_context.queue = NULL;
    }
    
    if (g_gpu_context.device) {
        wgpuDeviceRelease(g_gpu_context.device);
        g_gpu_context.device = NULL;
    }
    
    
    if (g_gpu_context.surface) {
        wgpuSurfaceRelease(g_gpu_context.surface);
        g_gpu_context.surface = NULL;
    }
    
    
    memset(&g_gpu_context, 0, sizeof(gpu_context_t));
    printf("[GPU] ✓ Cleanup completed\n");
}

gpu_context_t* gpu_get_context(void) {
    if (!g_gpu_context.is_initialized) {
        fprintf(stderr, "ERROR: GPU context is not initialized!\n");
        return NULL;
    }
    return &g_gpu_context;
}


const char* gpu_get_last_error(void) {
    return g_gpu_context.error_message[0] != '\0' ? g_gpu_context.error_message : NULL;
}

gpu_init_config_t gpu_create_default_config(void* window_handle, uint32_t width, uint32_t height) {
    gpu_init_config_t config = {
        .window_handle = window_handle,
        .initial_width = width,
        .initial_height = height,
        .preferred_format = WGPUTextureFormat_BGRA8Unorm,
        .present_mode = WGPUPresentMode_Fifo, // V-sync enabled
        .enable_depth_testing = true,
        .enable_debug_labels = true,
        .validate_shaders = true,
    };
    return config;
}

WGPUTextureView gpu_begin_frame(void) {
    WGPUSurfaceTexture surface_texture;
    wgpuSurfaceGetCurrentTexture(g_gpu_context.surface, &surface_texture);
    if (surface_texture.status != WGPUSurfaceGetCurrentTextureStatus_SuccessOptimal &&
        surface_texture.status != WGPUSurfaceGetCurrentTextureStatus_SuccessSuboptimal) {
        printf("Failed to get current surface texture: %d\n", surface_texture.status);
        return NULL;
    }
    WGPUTexture texture = surface_texture.texture;
    WGPUTextureViewDescriptor view_desc = {
        // .nextInChain = NULL,
        // .label = toWGPUStringView("Current Swapchain Texture View"),
        .format = wgpuTextureGetFormat(texture),
        .dimension = WGPUTextureViewDimension_2D,
        .baseMipLevel = 0,
        .mipLevelCount = 1,
        .baseArrayLayer = 0,
        .arrayLayerCount = 1,
        .aspect = WGPUTextureAspect_All,
        .usage = WGPUTextureUsage_RenderAttachment,
    };
    WGPUTextureView target_view = wgpuTextureCreateView(texture, &view_desc);
    if (!target_view) {
        printf("Failed to create texture view for current surface texture\n");
        return NULL;
    }
    return target_view;
}
void gpu_present_frame(void) {
    // dawn emscripten backend automatically presents the frame when the texture is released
}
// ============================================================================
// Callback Implementations
// ============================================================================


/**
 * Callback for adapter request completion
 * This is called asynchronously when WebGPU finishes selecting an adapter
 */
static void on_adapter_request_ended( WGPURequestAdapterStatus status, WGPUAdapter adapter, WGPUStringView message, void* userdata1, void* userdata2
) {
    (void)message; // Unused parameter
    bool* requestEnded = (bool*)userdata2;
    if (status == WGPURequestAdapterStatus_Success) {
        *(WGPUAdapter*)userdata1 = adapter;
    } else {
        printf("Failed to get WebGPU adapter\n");
    }
    *requestEnded = true;
}


/**
 * Callback for device request completion
 * This is called asynchronously when WebGPU finishes creating the logical device
 */
static void on_device_request_ended( WGPURequestDeviceStatus status, WGPUDevice device, WGPUStringView message, void* userdata1, void* userdata2
) {
    bool* requestEnded = (bool*)userdata2;
    if (status == WGPURequestDeviceStatus_Success) {
        *(WGPUDevice*)userdata1 = device;
    } else {
        printf("Failed to get WebGPU device: %.*s\n", 
            (int)message.length, message.data ? message.data : "No message");
    }
    *requestEnded = true;
}

/**
 * Callback for uncaptured device errors
 * This helps catch GPU errors that aren't handled elsewhere
 */
static void on_device_error(const WGPUDevice * device, WGPUErrorType type, WGPUStringView message, void * userdata1, void * userdata2) {
    (void)device;
    (void)userdata1;
    (void)userdata2;
    const char* error_type_str = "Unknown";
    
    switch (type) {
        case WGPUErrorType_NoError:      error_type_str = "No Error"; break;
        case WGPUErrorType_Validation:   error_type_str = "Validation"; break;
        case WGPUErrorType_OutOfMemory:  error_type_str = "Out of Memory"; break;
        case WGPUErrorType_Internal:     error_type_str = "Internal"; break;
        case WGPUErrorType_Unknown:      error_type_str = "Unknown"; break;
        default:                        error_type_str = "Unrecognized"; break;
    }
    
    printf("[GPU ERROR] %s: %.*s\n", error_type_str, (int)message.length, message.data ? message.data : "No message");
    set_error("GPU Error (%s): %.*s", error_type_str, (int)message.length, message.data ? message.data : "No message");
}

/**
 * Callback for device lost events
 * This is called when the GPU device becomes unavailable
 */
static void on_device_lost(const WGPUDevice * device, WGPUDeviceLostReason reason, WGPUStringView message, void * userdata1, void * userdata2) {
    (void)device;
    (void)userdata1;
    (void)userdata2;
    const char* reason_str = "Unknown";

    switch (reason) {
        case WGPUDeviceLostReason_Unknown: reason_str = "Unknown"; break;
        case WGPUDeviceLostReason_Destroyed: reason_str = "Destroyed"; break;
        case WGPUDeviceLostReason_CallbackCancelled: reason_str = "Callback Cancelled"; break;
        case WGPUDeviceLostReason_FailedCreation: reason_str = "Failed Creation"; break;
        default: reason_str = "Unrecognized"; break;
    }
    
    printf("[GPU] Device lost (%s): %.*s\n", reason_str, (int)message.length, message.data ? message.data : "No message");
    set_error("Device lost (%s): %.*s", reason_str, (int)message.length, message.data ? message.data : "No message");
    
    // Mark context as no longer initialized
    g_gpu_context.is_initialized = false;
}

// ============================================================================
// Helper Functions
// ============================================================================


/**
 * Set error message with printf-style formatting
 * Provides a centralized way to track the last error that occurred
 */
static void set_error(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vsnprintf(g_gpu_context.error_message, sizeof(g_gpu_context.error_message), format, args);
    va_end(args);
}

// ============================================================================
// Legacy Compatibility
// ============================================================================

/**
 * Legacy initialization function for backward compatibility
 * @deprecated Use gpu_init() with proper configuration instead
 */
void webgpu_init(void) {
    printf("[GPU] Warning: webgpu_init() is deprecated. Use gpu_init() instead.\n");
    
    // Create default configuration (requires a valid window handle in practice)
    gpu_init_config_t config = gpu_create_default_config(NULL, 800, 600);
    
    if (!gpu_init(&config)) {
        printf("[GPU] Failed to initialize with legacy function\n");
    }
}

WGPUInstance create_instance(WGPU_NULLABLE WGPUInstanceDescriptor const * descriptor) {
    WGPUInstance instance = wgpuCreateInstance(descriptor);
    assert(instance != NULL && "Failed to create WGPUInstance");
    printf("WebGPU instance created successfully\n");
    return instance;
}
WGPUSurface create_surface(WGPUInstance instance, char const * canvas_selector) {
    WGPUSurface surface = NULL;
    #ifdef __EMSCRIPTEN__
    WGPUSurfaceDescriptor surfaceDescriptor;
    WGPUChainedStruct chain = {
        .next = NULL,
        .sType = WGPUSType_EmscriptenSurfaceSourceCanvasHTMLSelector,
    };
    WGPUEmscriptenSurfaceSourceCanvasHTMLSelector fromCanvasHTMLSelector = {
        .chain = chain,
        .selector = toWGPUStringView(canvas_selector ? canvas_selector : "#canvas"),
    };
    surfaceDescriptor.nextInChain = &fromCanvasHTMLSelector.chain;
    surface = wgpuInstanceCreateSurface(instance, &surfaceDescriptor);
    assert(surface != NULL && "Failed to create WGPUSurface");
    #else
    // Platform-specific surface creation for non-Emscripten targets goes here
    #endif // __EMSCRIPTEN__
    printf("WebGPU surface created successfully\n");
    return surface;
}
WGPUAdapter create_adapter(WGPUInstance instance, WGPUSurface surface) {
    WGPURequestAdapterOptions options = {};
    options.nextInChain = NULL;
    options.compatibleSurface = surface;
    
    WGPUAdapter adapter = requestAdapterSync(instance, &options);
    assert(adapter != NULL && "Failed to get WGPUAdapter");
    printf("WebGPU adapter obtained successfully\n");
    return adapter;
}
WGPUDevice create_device(WGPUAdapter adapter, WGPULimits * supported_limits, WGPULimits * required_limits) {
    required_limits->minStorageBufferOffsetAlignment = 
        supported_limits->minStorageBufferOffsetAlignment;
    required_limits->minUniformBufferOffsetAlignment =
        supported_limits->minUniformBufferOffsetAlignment;
    WGPUDeviceDescriptor deviceDesc = {
        .nextInChain = NULL,
        .requiredLimits = required_limits,
        .deviceLostCallbackInfo = (WGPUDeviceLostCallbackInfo){
            .nextInChain = NULL,
            .mode = WGPUCallbackMode_AllowSpontaneous,
            .callback = on_device_lost,
            .userdata1 = NULL,
            .userdata2 = NULL,
        },
        .uncapturedErrorCallbackInfo = (WGPUUncapturedErrorCallbackInfo){
            .nextInChain = NULL,
            .callback = on_device_error,
            .userdata1 = NULL,
            .userdata2 = NULL,
        },
    };
    WGPUDevice device = requestDeviceSync(adapter, &deviceDesc);
    assert(device != NULL && "Failed to get WGPUDevice");
    printf("WebGPU device created successfully\n");
    return device;
}
WGPUTextureFormat configure_surface(WGPUDevice device, WGPUSurface surface, WGPUAdapter adapter, uint32_t width, uint32_t height) {
    WGPUSurfaceCapabilities capabilities = {0};
    wgpuSurfaceGetCapabilities(surface, adapter, &capabilities);
    printf("Surface supports %lu formats\n", capabilities.formatCount);
    const char * formats[] = {
        [0x1b] = "WGPUTextureFormat_BGRA8Unorm",
        [0x16] = "WGPUTextureFormat_RGBA8Unorm",
        [0x28] = "WGPUTextureFormat_RGBA16Float",
    };
    for(uint32_t i = 0; i < capabilities.formatCount; i++) {
        printf("  Format %u: %s\n", i, 
            (capabilities.formats[i] < sizeof(formats)/sizeof(formats[0]) && formats[capabilities.formats[i]] != NULL) ? 
            formats[capabilities.formats[i]] : "Unknown Format");
    }
    WGPUTextureFormat surfaceFormat = capabilities.formatCount > 0 ? 
    capabilities.formats[0] : WGPUTextureFormat_BGRA8Unorm;
    printf("Selected surface format: 0x%x %s\n", surfaceFormat,
        (surfaceFormat < sizeof(formats)/sizeof(formats[0]) && formats[surfaceFormat] != NULL) ? 
        formats[surfaceFormat] : "Unknown Format");

    WGPUSurfaceConfiguration config = {
        .device = device,
        .format = surfaceFormat,
        .usage = WGPUTextureUsage_RenderAttachment,
        .width = width,
        .height = height,
        
        .alphaMode = WGPUCompositeAlphaMode_Auto,
        .presentMode = WGPUPresentMode_Fifo,
    };

    wgpuSurfaceConfigure(surface, &config);
    
    
    printf("Surface configured successfully\n");
    return surfaceFormat;
}

WGPUTextureFormat gpu_get_surface_format(void) {
    if (!g_gpu_context.is_initialized) {
        printf("Warning: GPU context not initialized, returning default format\n");
        return WGPUTextureFormat_BGRA8Unorm;
    }
    return g_gpu_context.surface_format;
}
WGPUQueue create_queue(WGPUDevice device) {
    WGPUQueue queue = wgpuDeviceGetQueue(device);
    assert(queue != NULL && "Failed to get WGPUQueue");
    printf("WebGPU device queue obtained successfully\n");
    return queue;
}

WGPUAdapter requestAdapterSync(WGPUInstance instance, const WGPURequestAdapterOptions* options) {
    if (!instance) {
        printf("Error: WGPUInstance is NULL in requestAdapterSync\n");
        return NULL;
    }

    WGPUAdapter adapter = NULL;
    volatile bool requestEnded = false;
    
    wgpuInstanceRequestAdapter(instance, options, (WGPURequestAdapterCallbackInfo){
        .nextInChain = NULL,
        .mode = WGPUCallbackMode_AllowProcessEvents,
        .callback = on_adapter_request_ended,
        .userdata1 = (void*)&adapter,
        .userdata2 = (void*)&requestEnded,
    });

    #ifdef __EMSCRIPTEN__
    int timeout_counter = 0;
    while (!requestEnded && timeout_counter < 100) {
        emscripten_sleep(100);
        wgpuInstanceProcessEvents(instance);
        timeout_counter++;
    }
    if (timeout_counter >= 100) {
        printf("Error: Timeout while waiting for adapter in requestAdapterSync\n");
    }
    #endif // __EMSCRIPTEN__

    assert(requestEnded);
    return adapter;
}


WGPUDevice requestDeviceSync(WGPUAdapter adapter, const WGPUDeviceDescriptor* descriptor) {
    if (!adapter) {
        printf("Error: WGPUAdapter is NULL in requestDeviceSync\n");
        return NULL;
    }

    WGPUDevice device = NULL;
    volatile bool requestEnded = false;
    
    wgpuAdapterRequestDevice(adapter, descriptor, (WGPURequestDeviceCallbackInfo){
        .nextInChain = NULL,
        .mode = WGPUCallbackMode_AllowSpontaneous,
        .callback = on_device_request_ended,
        .userdata1 = (void*)&device,
        .userdata2 = (void*)&requestEnded,
    });

    #ifdef __EMSCRIPTEN__
    int timeout_counter = 0;
    while (!requestEnded && timeout_counter < 100) {
        emscripten_sleep(100);
        timeout_counter++;
    }
    if (timeout_counter >= 100) {
        printf("Error: Timeout while waiting for device in requestDeviceSync\n");
    }
    #endif // __EMSCRIPTEN__

    assert(requestEnded);
    return device;
}

static void print_limits(const WGPULimits* limits, bool verbose) {
    if (!verbose) {
        // not printing detailed limits
        return;
    }
    printf("Adapter limits:\n");
    printf("  minStorageBufferOffsetAlignment: %u\n", limits->minStorageBufferOffsetAlignment);
    printf("  minUniformBufferOffsetAlignment: %u\n", limits->minUniformBufferOffsetAlignment);
    printf("  maxVertexAttributes: %u\n", limits->maxVertexAttributes);
    printf("  maxVertexBuffers: %u\n", limits->maxVertexBuffers);
    printf("  maxBufferSize: %llu\n", limits->maxBufferSize);
    printf("  maxVertexBufferArrayStride: %u\n", limits->maxVertexBufferArrayStride);
    printf("  maxUniformBufferBindingSize: %llu\n", limits->maxUniformBufferBindingSize);
    printf("  maxInterStageShaderVariables: %u\n", limits->maxInterStageShaderVariables);
    printf("  maxBindGroups: %u\n", limits->maxBindGroups);
    printf("  maxUniformBuffersPerShaderStage: %u\n", limits->maxUniformBuffersPerShaderStage);
    printf("  maxUniformBufferBindingSize: %llu\n", limits->maxUniformBufferBindingSize);
    printf("  maxTextureDimension1D: %u\n", limits->maxTextureDimension1D);
    printf("  maxTextureDimension2D: %u\n", limits->maxTextureDimension2D);
    printf("  maxTextureArrayLayers: %u\n", limits->maxTextureArrayLayers);
}