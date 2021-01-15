#pragma once

#include "../Graph/GraphBuilder.h"
#include "../Renderer_DrawList.h"

#if defined(USE_GL_RENDER)
#include <Ren/VaoGL.h>
#endif

class PrimDraw;

class RpDebugEllipsoids : public RenderPassBase {
    PrimDraw &prim_draw_;
    bool initialized = false;

    // temp data (valid only between Setup and Execute calls)
    DynArrayConstRef<EllipsItem> ellipsoids_;
    const ViewState *view_state_ = nullptr;
    int orphan_index_ = -1;
    Ren::TexHandle output_tex_;

    void LazyInit(Ren::Context &ctx, ShaderLoader &sh);
    void DrawProbes(RpBuilder &builder);

    // lazily initialized data
#if defined(USE_GL_RENDER)
    Ren::ProgramRef ellipsoid_prog_;
    Ren::Framebuffer draw_fb_;
#endif
  public:
    RpDebugEllipsoids(PrimDraw &prim_draw) : prim_draw_(prim_draw) {}

    void Setup(RpBuilder &builder, const DrawList &list, const ViewState *view_state,
               int orphan_index, Ren::TexHandle output_tex);
    void Execute(RpBuilder &builder) override;

    // TODO: remove this
    int alpha_blend_start_index_ = -1;

    const char *name() const override { return "DEBUG PROBES"; }
};