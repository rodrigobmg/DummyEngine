#pragma once

#include "Log.h"

typedef struct ALCdevice_struct ALCdevice;
typedef struct ALCcontext_struct ALCcontext;

namespace Snd {
class Context {
    ILog *log_ = nullptr;
#if defined(USE_AL_SOUND)
    ALCdevice *oal_device_ = nullptr;
    ALCcontext *oal_context_ = nullptr;
#endif
  public:
    Context() = default;
    ~Context();

    Context(const Context &rhs) = delete;

    bool Init(ILog *log);

    void ReleaseAll();
};
} // namespace Snd