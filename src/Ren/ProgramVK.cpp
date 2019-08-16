#include "ProgramVK.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4996)
#endif

Ren::Program::Program(const char *name, ShaderCode vs_code, ShaderCode fs_code, VkDevice device, eProgLoadStatus *status) {
    name_ = String{ name };
    Init(vs_code, fs_code, device, status);
}

Ren::Program::Program(const char *name, ShaderCode cs_code, VkDevice device, eProgLoadStatus *status) {
    name_ = String{ name };
    Init(cs_code, device, status);
}

Ren::Program::~Program() {
    if (vs_module_) {
        vkDestroyShaderModule(device_, vs_module_, nullptr);
    }
    if (fs_module_) {
        vkDestroyShaderModule(device_, fs_module_, nullptr);
    }
    if (cs_module_) {
        vkDestroyShaderModule(device_, cs_module_, nullptr);
    }
}

Ren::Program &Ren::Program::operator=(Program &&rhs) {
    RefCounter::operator=(std::move(rhs));

    if (vs_module_) {
        vkDestroyShaderModule(device_, vs_module_, nullptr);
    }
    if (fs_module_) {
        vkDestroyShaderModule(device_, fs_module_, nullptr);
    }
    if (cs_module_) {
        vkDestroyShaderModule(device_, cs_module_, nullptr);
    }

    device_ = rhs.device_;
    rhs.device_ = VK_NULL_HANDLE;
    vs_module_ = rhs.vs_module_;
    rhs.vs_module_ = VK_NULL_HANDLE;
    fs_module_ = rhs.fs_module_;
    rhs.fs_module_ = VK_NULL_HANDLE;
    cs_module_ = rhs.cs_module_;
    rhs.cs_module_ = VK_NULL_HANDLE;

    attributes_ = std::move(rhs.attributes_);
    uniforms_ = std::move(rhs.uniforms_);
    uniform_blocks_ = std::move(rhs.uniform_blocks_);
    ready_ = rhs.ready_;
    rhs.ready_ = false;
    name_ = std::move(rhs.name_);

    return *this;
}

void Ren::Program::Init(ShaderCode vs_code, ShaderCode fs_code, VkDevice device, eProgLoadStatus *status) {
    InitFromSPIRV({ vs_code, fs_code, {} }, device, status);
}

void Ren::Program::Init(ShaderCode cs_code, VkDevice device, eProgLoadStatus *status) {
    InitFromSPIRV({ {}, {}, cs_code }, device, status);
}

void Ren::Program::InitFromSPIRV(const Shaders &shaders, VkDevice device, eProgLoadStatus *status) {
    if ((!shaders.vs_code.code || !shaders.fs_code.code) && !shaders.cs_code.code) {
        if (status) *status = ProgSetToDefault;
        return;
    }

    assert(!ready_);

    if (shaders.vs_code.code && shaders.fs_code.code) {
        VkShaderModuleCreateInfo vs_create_info = {};
        vs_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        vs_create_info.codeSize = static_cast<size_t>(shaders.vs_code.code_size);
        vs_create_info.pCode = reinterpret_cast<const uint32_t *>(shaders.vs_code.code);

        VkResult res = vkCreateShaderModule(device_, &vs_create_info, nullptr, &vs_module_);
        assert(res == VK_SUCCESS && "Failed to create shader module!");

        VkShaderModuleCreateInfo fs_create_info = {};
        fs_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        fs_create_info.codeSize = static_cast<size_t>(shaders.fs_code.code_size);
        fs_create_info.pCode = reinterpret_cast<const uint32_t *>(shaders.fs_code.code);

        res = vkCreateShaderModule(device_, &fs_create_info, nullptr, &fs_module_);
        assert(res == VK_SUCCESS && "Failed to create shader module!");
    } else if (shaders.cs_code.code) {
        // TODO: !!!!!!!!!
    }
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif