#pragma once

#include <cstdint>
#include <cstring>

#include <array>
#include <string>

#include "Storage.h"
#include "String.h"

#include "VK.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4996)
#endif

namespace Ren {
const int MAX_NUM_ATTRIBUTES = 32;
const int MAX_NUM_UNIFORMS = 32;
const int MAX_NUM_UNIFORM_BLOCKS = 16;
struct Descr {
    std::string name;
    int loc = -1;
};
typedef Descr Attribute;
typedef Descr Uniform;
typedef Descr UniformBlock;

enum eProgLoadStatus { ProgFound, ProgSetToDefault, ProgCreatedFromData };

struct ShaderCode {
    const uint32_t *code = nullptr;
    uint32_t        code_size = 0;
};

class Program : public RefCounter {
    VkDevice                                    device_ = VK_NULL_HANDLE;
    VkShaderModule                              vs_module_ = VK_NULL_HANDLE,
                                                fs_module_ = VK_NULL_HANDLE,
                                                cs_module_ = VK_NULL_HANDLE;
    std::array<Attribute, MAX_NUM_ATTRIBUTES>   attributes_;
    std::array<Uniform, MAX_NUM_UNIFORMS>       uniforms_;
    std::array<UniformBlock, MAX_NUM_UNIFORM_BLOCKS> uniform_blocks_;
    bool        ready_ = false;
    String      name_;

    struct Shaders {
        ShaderCode vs_code, fs_code, cs_code;
    };

    void InitFromSPIRV(const Shaders &shaders, VkDevice device, eProgLoadStatus *status);
public:
    Program() {}
    Program(const char *name, ShaderCode vs_code, ShaderCode fs_code, VkDevice device, eProgLoadStatus *status = nullptr);
    Program(const char *name, ShaderCode cs_code, VkDevice device, eProgLoadStatus *status = nullptr);
    Program(const Program &rhs) = delete;
    Program(Program &&rhs) {
        *this = std::move(rhs);
    }
    ~Program();

    Program &operator=(const Program &rhs) = delete;
    Program &operator=(Program &&rhs);

    bool ready() const {
        return ready_;
    }
    const String &name() const {
        return name_;
    }

    const Attribute &attribute(int i) const {
        return attributes_[i];
    }

    const Attribute &attribute(const char *name) const {
        for (int i = 0; i < MAX_NUM_ATTRIBUTES; i++) {
            if (attributes_[i].name == name) {
                return attributes_[i];
            }
        }
        return attributes_[0];
    }

    const Uniform &uniform(int i) const {
        return uniforms_[i];
    }

    const Uniform &uniform(const char *name) const {
        for (int i = 0; i < MAX_NUM_UNIFORMS; i++) {
            if (uniforms_[i].name == name) {
                return uniforms_[i];
            }
        }
        return uniforms_[0];
    }

    const UniformBlock &uniform_block(int i) const {
        return uniform_blocks_[i];
    }

    const UniformBlock &uniform_block(const char *name) const {
        for (int i = 0; i < MAX_NUM_UNIFORM_BLOCKS; i++) {
            if (uniform_blocks_[i].name == name) {
                return uniform_blocks_[i];
            }
        }
        return uniform_blocks_[0];
    }

    void Init(ShaderCode vs_code, ShaderCode fs_code, VkDevice device, eProgLoadStatus *status);
    void Init(ShaderCode cs_code, VkDevice device, eProgLoadStatus *status);
};

typedef StorageRef<Program> ProgramRef;
typedef Storage<Program> ProgramStorage;
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif