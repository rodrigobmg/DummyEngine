#include "Context.h"

#include "VK.h"

#include <Windows.h>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4996)
#endif

struct VkContext {
    VkInstance               instance;
    VkDebugReportCallbackEXT callback;
};

Ren::Context::~Context() {
    ReleaseAll();

    if (vk_ctx_) {
        vkDestroyDebugReportCallbackEXT(vk_ctx_->instance, vk_ctx_->callback, nullptr);
        vkDestroyInstance(vk_ctx_->instance, nullptr);
    }
}

void Ren::Context::Init(int w, int h) {
    HMODULE vulkan_module = LoadLibrary("vulkan-1.dll");
    assert(vulkan_module && "Failed to load vulkan module.");

    vkCreateInstance = (PFN_vkCreateInstance)GetProcAddress(vulkan_module, "vkCreateInstance");
    assert(vkCreateInstance);

    vkDestroyInstance = (PFN_vkDestroyInstance)GetProcAddress(vulkan_module, "vkDestroyInstance");
    assert(vkDestroyInstance);

    vkEnumerateInstanceLayerProperties = (PFN_vkEnumerateInstanceLayerProperties)GetProcAddress(vulkan_module, "vkEnumerateInstanceLayerProperties");
    assert(vkEnumerateInstanceLayerProperties);

    vkEnumerateInstanceExtensionProperties = (PFN_vkEnumerateInstanceExtensionProperties)GetProcAddress(vulkan_module, "vkEnumerateInstanceExtensionProperties");
    assert(vkEnumerateInstanceExtensionProperties);

    vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)GetProcAddress(vulkan_module, "vkGetInstanceProcAddr");
    assert(vkGetInstanceProcAddr);

    vkEnumeratePhysicalDevices = (PFN_vkEnumeratePhysicalDevices)GetProcAddress(vulkan_module, "vkEnumeratePhysicalDevices");
    assert(vkEnumeratePhysicalDevices);

    vkGetPhysicalDeviceProperties = (PFN_vkGetPhysicalDeviceProperties)GetProcAddress(vulkan_module, "vkGetPhysicalDeviceProperties");
    assert(vkGetPhysicalDeviceProperties);

    vkGetPhysicalDeviceQueueFamilyProperties = (PFN_vkGetPhysicalDeviceQueueFamilyProperties)GetProcAddress(vulkan_module, "vkGetPhysicalDeviceQueueFamilyProperties");
    assert(vkGetPhysicalDeviceQueueFamilyProperties);

    vkCreateDevice = (PFN_vkCreateDevice)GetProcAddress(vulkan_module, "vkCreateDevice");
    assert(vkCreateDevice);

    vkDestroyDevice = (PFN_vkDestroyDevice)GetProcAddress(vulkan_module, "vkDestroyDevice");
    assert(vkDestroyDevice);

    vkEnumerateDeviceExtensionProperties = (PFN_vkEnumerateDeviceExtensionProperties)GetProcAddress(vulkan_module, "vkEnumerateDeviceExtensionProperties");
    assert(vkEnumerateDeviceExtensionProperties);

    vk_ctx_.reset(new VkContext);

    const char *layers[] = { "VK_LAYER_LUNARG_standard_validation" };

    {   // Create Vulkan instance
        {   // Find validation layer
            uint32_t layer_count = 0;
            vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

            assert(layer_count != 0 && "Failed to find any layer in your system.");

            auto layers_available = std::unique_ptr<VkLayerProperties[]>{ new VkLayerProperties[layer_count] };
            vkEnumerateInstanceLayerProperties(&layer_count, &layers_available[0]);

            bool found_validation = false;
            for (uint32_t i = 0; i < layer_count; i++) {
                if (strcmp(layers_available[i].layerName, "VK_LAYER_LUNARG_standard_validation") == 0) {
                    found_validation = true;
                }
            }

            assert(found_validation && "Could not find validation layer.");
        }

        const char *extensions[] = { "VK_KHR_surface", "VK_KHR_win32_surface", VK_EXT_DEBUG_REPORT_EXTENSION_NAME };
        uint32_t number_required_extensions = sizeof(extensions) / sizeof(char*);

        {   // Find required extensions
            uint32_t ext_count = 0;
            vkEnumerateInstanceExtensionProperties(NULL, &ext_count, NULL);
            auto extensions_available = std::unique_ptr<VkExtensionProperties[]>{ new VkExtensionProperties[ext_count] };
            vkEnumerateInstanceExtensionProperties(NULL, &ext_count, &extensions_available[0]);

            uint32_t found_extensions = 0;
            for (uint32_t i = 0; i < ext_count; i++) {
                for (uint32_t j = 0; j < number_required_extensions; j++) {
                    if (strcmp(extensions_available[i].extensionName, extensions[j]) == 0) {
                        found_extensions++;
                    }
                }
            }
            assert(found_extensions == number_required_extensions && "Could not find debug extension");
        }

        VkApplicationInfo app_info = {};
        app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        app_info.pApplicationName = "Test1";
        app_info.engineVersion = 1;
        app_info.apiVersion = VK_MAKE_VERSION(1, 0, 0);

        VkInstanceCreateInfo instance_info = {};
        instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instance_info.pApplicationInfo = &app_info;
        instance_info.enabledLayerCount = 1;
        instance_info.ppEnabledLayerNames = layers;
        instance_info.enabledExtensionCount = number_required_extensions;
        instance_info.ppEnabledExtensionNames = extensions;

        VkResult res = vkCreateInstance(&instance_info, nullptr, &vk_ctx_->instance);
        assert(res == VK_SUCCESS && "Failed to create vulkan instance");

        InitVKExtensions(vk_ctx_->instance);

        auto debug_report_callback = [](VkDebugReportFlagsEXT flags,
                                        VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location,
                                        int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void* pUserData) VKAPI_ATTR -> VkBool32 VKAPI_CALL {
            printf("%s: %s\n", pLayerPrefix, pMessage);
            return VK_FALSE;
        };

        VkDebugReportCallbackCreateInfoEXT callback_create_info = {};
        callback_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
        callback_create_info.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
        callback_create_info.pfnCallback = debug_report_callback;
        callback_create_info.pUserData = NULL;

        res = vkCreateDebugReportCallbackEXT(vk_ctx_->instance, &callback_create_info, NULL, &vk_ctx_->callback);
        assert(res == VK_SUCCESS && "Failed to create debug report callback.");
    }

    w_ = w;
    h_ = h;

#if 0
    printf("===========================================\n");
    printf("Device info:\n");

    // print device info
#if !defined(EMSCRIPTEN) && !defined(__ANDROID__)
    GLint gl_major_version;
    glGetIntegerv(GL_MAJOR_VERSION, &gl_major_version);
    GLint gl_minor_version;
    glGetIntegerv(GL_MINOR_VERSION, &gl_minor_version);
    printf("\tOpenGL version\t: %i.%i\n", int(gl_major_version), int(gl_minor_version));
#endif

    printf("\tVendor\t\t: %s\n", glGetString(GL_VENDOR));
    printf("\tRenderer\t: %s\n", glGetString(GL_RENDERER));
    printf("\tGLSL version\t: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

    printf("Capabilities:\n");
    
    // determine if anisotropy supported
    if (IsExtensionSupported("GL_EXT_texture_filter_anisotropic")) {
        GLfloat f;
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &f);
        anisotropy = f;
        printf("\tAnisotropy\t: %f\n", anisotropy);
    }
    
    // how many uniform vec4 vectors can be used
    GLint i = 0;
    glGetIntegerv(/*GL_MAX_VERTEX_UNIFORM_VECTORS*/ GL_MAX_VERTEX_UNIFORM_COMPONENTS, &i);
    i /= 4;
    if (i == 0) i = 256;
    max_uniform_vec4 = i;
    printf("\tMax uniforms\t: %i\n", max_uniform_vec4);

    // how many bones(mat4) can be used at time
    Mesh::max_gpu_bones = max_uniform_vec4 / 8;
    printf("\tBones per pass\t: %i\n", Mesh::max_gpu_bones);
    char buff[16];
    sprintf(buff, "%i", Mesh::max_gpu_bones);
    /*glsl_defines_ += "#define MAX_GPU_BONES ";
    glsl_defines_ += buff;
    glsl_defines_ += "\r\n";*/

    printf("===========================================\n\n");

#ifndef NDEBUG
    if (IsExtensionSupported("GL_KHR_debug") || IsExtensionSupported("ARB_debug_output") ||
        IsExtensionSupported("AMD_debug_output")) {

        auto gl_debug_proc = [](GLenum source,
                                GLenum type,
                                GLuint id,
                                GLenum severity,
                                GLsizei length,
                                const GLchar *message,
                                const void *userParam) {
            if (severity != GL_DEBUG_SEVERITY_NOTIFICATION) {
                printf("%s\n", message);
            }
        };

        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(gl_debug_proc, nullptr);
    }
#endif

    default_vertex_buf1_        = buffers_.Add("default_vtx_buf1", 64 * 1024 * 1024);
    default_vertex_buf2_        = buffers_.Add("default_vtx_buf2", 64 * 1024 * 1024);
    default_skin_vertex_buf_    = buffers_.Add("default_skin_buf", 16 * 1024 * 1024);
    default_indices_buf_        = buffers_.Add("default_ndx_buf2", 64 * 1024 * 1024);
#endif
}

void Ren::Context::Resize(int w, int h) {
    w_ = w;
    h_ = h;
}

#if 0
Ren::ProgramRef Ren::Context::LoadProgramGLSL(const char *name, const char *vs_source, const char *fs_source, eProgLoadStatus *load_status) {
    ProgramRef ref = programs_.FindByName(name);

    std::string vs_source_str, fs_source_str;

    if (vs_source) {
        vs_source_str = glsl_defines_ + vs_source;
        vs_source = vs_source_str.c_str();
    }

    if (fs_source) {
        fs_source_str = glsl_defines_ + fs_source;
        fs_source = fs_source_str.c_str();
    }

    if (!ref) {
        ref = programs_.Add(name, vs_source, fs_source, load_status);
    } else {
        if (ref->ready()) {
            if (load_status) *load_status = ProgFound;
        } else if (!ref->ready() && vs_source && fs_source) {
            ref->Init(vs_source, fs_source, load_status);
        }
    }

    return ref;
}

Ren::ProgramRef Ren::Context::LoadProgramGLSL(const char *name, const char *cs_source, eProgLoadStatus *load_status) {
    ProgramRef ref = programs_.FindByName(name);

    std::string cs_source_str;

    if (cs_source) {
        cs_source_str = glsl_defines_ + cs_source;
        cs_source = cs_source_str.c_str();
    }

    if (!ref) {
        ref = programs_.Add(name, cs_source, load_status);
    } else {
        if (ref->ready()) {
            if (load_status) *load_status = ProgFound;
        } else if (!ref->ready() && cs_source) {
            ref->Init(name, cs_source, load_status);
        }
    }

    return ref;
}
#endif

#ifdef _MSC_VER
#pragma warning(pop)
#endif