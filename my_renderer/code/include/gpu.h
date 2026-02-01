#ifndef GPU_H
#define GPU_H

#include <webgpu/webgpu.h>
#include <stdbool.h>
#include <stdint.h>

/**
 * GPU context structure containing all WebGPU state
 * This structure encapsulates the entire WebGPU rendering pipeline
 * and provides a clean abstraction for GPU operations.
 */
typedef struct gpu_context {
    // Core WebGPU objects
    WGPUSurface surface;                // Surface to render to (connected to window)
    WGPUDevice device;                  // Logical device for GPU operations
    WGPUQueue queue;                    // Command queue for submitting work
    
    WGPULimits supported_limits;      // Supported device limits

    WGPUTextureFormat surface_format;   // Format of the surface textures

    // Initialization state
    bool is_initialized;                // Track initialization status
    char error_message[512];            // Last error message buffer
} gpu_context_t;

/**
 * WebGPU initialization configuration
 * Allows customization of GPU initialization parameters
 */
typedef struct gpu_init_config {
    // Window/surface parameters
    void* window_handle;                // Platform-specific window handle
    uint32_t initial_width;             // Initial swapchain width
    uint32_t initial_height;            // Initial swapchain height
    
    // Rendering preferences
    WGPUTextureFormat preferred_format; // Preferred swapchain format
    WGPUPresentMode present_mode;       // V-sync mode (immediate, mailbox, fifo)
    bool enable_depth_testing;          // Whether to create depth buffer
    
    // Debug settings
    bool enable_debug_labels;           // Enable debug labels for better debugging
    bool validate_shaders;              // Enable shader validation
} gpu_init_config_t;

// ============================================================================
// Public API Functions
// ============================================================================

/**
 * Initialize the WebGPU context with the specified configuration
 * @param config Initialization configuration parameters
 * @return true on success, false on failure
 */
bool gpu_init(const gpu_init_config_t* config);

/**
 * Cleanup and destroy the GPU context
 * Should be called before application exit
 */
void gpu_cleanup(void);

/**
 * Get the current GPU context
 * @return Pointer to the global GPU context, or NULL if not initialized
 */
gpu_context_t* gpu_get_context(void);

/**
 * Begin a new frame and acquire the next swapchain texture
 * @return The current swapchain texture view, or NULL on failure
 */
WGPUTextureView gpu_begin_frame(void);
/**
 * Present the current frame to the screen
 */
void gpu_present_frame(void);

/**
 * Get the last error message from GPU operations
 * @return Null-terminated error string, or NULL if no error
 */
const char* gpu_get_last_error(void);

/**
 * Create a default initialization configuration
 * @param window_handle Platform-specific window handle
 * @param width Initial window width
 * @param height Initial window height
 * @return Default configuration structure
 */
gpu_init_config_t gpu_create_default_config(void* window_handle, uint32_t width, uint32_t height);

/**
 * Get the surface texture format
 * @return The format of the surface textures
 */
WGPUTextureFormat gpu_get_surface_format(void);

// ============================================================================

WGPUInstance create_instance(WGPU_NULLABLE WGPUInstanceDescriptor const * descriptor);
WGPUSurface create_surface(WGPUInstance instance, char const * canvas_selector);
WGPUAdapter create_adapter(WGPUInstance instance, WGPUSurface surface);
WGPUDevice create_device(WGPUAdapter adapter, WGPULimits * supported_limits, WGPULimits * required_limits);
WGPUTextureFormat configure_surface(WGPUDevice device, WGPUSurface surface, WGPUAdapter adapter, uint32_t width, uint32_t height);

WGPUQueue create_queue(WGPUDevice device);

WGPUAdapter requestAdapterSync(WGPUInstance instance, const WGPURequestAdapterOptions* options);

WGPUDevice requestDeviceSync(WGPUAdapter adapter, const WGPUDeviceDescriptor* descriptor);

#endif // GPU_H