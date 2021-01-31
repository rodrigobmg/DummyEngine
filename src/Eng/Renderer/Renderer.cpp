#include "Renderer.h"

#include <cstdio>

#include <Ren/Context.h>
#include <Ren/Utils.h>
#include <Sys/ThreadPool.h>
#include <Sys/Time_.h>

#include "Renderer_Names.h"

#ifdef ENABLE_ITT_API
#include <vtune/ittnotify.h>
extern __itt_domain *__g_itt_domain;
#endif

namespace RendererInternal {
bool bbox_test(const float p[3], const float bbox_min[3], const float bbox_max[3]);

extern const uint8_t bbox_indices[];

int upper_power_of_two(int v) {
    int res = 1;
    while (res < v) {
        res *= 2;
    }
    return res;
}

const Ren::Vec2f HaltonSeq23[64] = {
    Ren::Vec2f{0.0000000000f, 0.0000000000f}, Ren::Vec2f{0.5000000000f, 0.3333333430f},
    Ren::Vec2f{0.2500000000f, 0.6666666870f}, Ren::Vec2f{0.7500000000f, 0.1111111190f},
    Ren::Vec2f{0.1250000000f, 0.4444444780f}, Ren::Vec2f{0.6250000000f, 0.7777778510f},
    Ren::Vec2f{0.3750000000f, 0.2222222390f}, Ren::Vec2f{0.8750000000f, 0.5555555820f},
    Ren::Vec2f{0.0625000000f, 0.8888889550f}, Ren::Vec2f{0.5625000000f, 0.0370370410f},
    Ren::Vec2f{0.3125000000f, 0.3703704180f}, Ren::Vec2f{0.8125000000f, 0.7037037610f},
    Ren::Vec2f{0.1875000000f, 0.1481481640f}, Ren::Vec2f{0.6875000000f, 0.4814815220f},
    Ren::Vec2f{0.4375000000f, 0.8148149250f}, Ren::Vec2f{0.9375000000f, 0.2592592840f},
    Ren::Vec2f{0.0312500000f, 0.5925926570f}, Ren::Vec2f{0.5312500000f, 0.9259260300f},
    Ren::Vec2f{0.2812500000f, 0.0740740821f}, Ren::Vec2f{0.7812500000f, 0.4074074630f},
    Ren::Vec2f{0.1562500000f, 0.7407408360f}, Ren::Vec2f{0.6562500000f, 0.1851852090f},
    Ren::Vec2f{0.4062500000f, 0.5185185670f}, Ren::Vec2f{0.9062500000f, 0.8518519400f},
    Ren::Vec2f{0.0937500000f, 0.2962963280f}, Ren::Vec2f{0.5937500000f, 0.6296296720f},
    Ren::Vec2f{0.3437500000f, 0.9629630450f}, Ren::Vec2f{0.8437500000f, 0.0123456810f},
    Ren::Vec2f{0.2187500000f, 0.3456790750f}, Ren::Vec2f{0.7187500000f, 0.6790124770f},
    Ren::Vec2f{0.4687500000f, 0.1234568060f}, Ren::Vec2f{0.9687500000f, 0.4567902090f},
    Ren::Vec2f{0.0156250000f, 0.7901235820f}, Ren::Vec2f{0.5156250000f, 0.2345679400f},
    Ren::Vec2f{0.2656250000f, 0.5679013130f}, Ren::Vec2f{0.7656250000f, 0.9012346860f},
    Ren::Vec2f{0.1406250000f, 0.0493827239f}, Ren::Vec2f{0.6406250000f, 0.3827161190f},
    Ren::Vec2f{0.3906250000f, 0.7160494920f}, Ren::Vec2f{0.8906250000f, 0.1604938510f},
    Ren::Vec2f{0.0781250000f, 0.4938272240f}, Ren::Vec2f{0.5781250000f, 0.8271605970f},
    Ren::Vec2f{0.3281250000f, 0.2716049850f}, Ren::Vec2f{0.8281250000f, 0.6049383880f},
    Ren::Vec2f{0.2031250000f, 0.9382717610f}, Ren::Vec2f{0.7031250000f, 0.0864197686f},
    Ren::Vec2f{0.4531250000f, 0.4197531640f}, Ren::Vec2f{0.9531250000f, 0.7530865670f},
    Ren::Vec2f{0.0468750000f, 0.1975308950f}, Ren::Vec2f{0.5468750000f, 0.5308642980f},
    Ren::Vec2f{0.2968750000f, 0.8641976710f}, Ren::Vec2f{0.7968750000f, 0.3086420300f},
    Ren::Vec2f{0.1718750000f, 0.6419754030f}, Ren::Vec2f{0.6718750000f, 0.9753087760f},
    Ren::Vec2f{0.4218750000f, 0.0246913619f}, Ren::Vec2f{0.9218750000f, 0.3580247460f},
    Ren::Vec2f{0.1093750000f, 0.6913581490f}, Ren::Vec2f{0.6093750000f, 0.1358024920f},
    Ren::Vec2f{0.3593750000f, 0.4691358800f}, Ren::Vec2f{0.8593750000f, 0.8024692540f},
    Ren::Vec2f{0.2343750000f, 0.2469136120f}, Ren::Vec2f{0.7343750000f, 0.5802469850f},
    Ren::Vec2f{0.4843750000f, 0.9135804180f}, Ren::Vec2f{0.9843750000f, 0.0617284030f}};

const int TaaSampleCount = 8;

#include "__brdf_lut.inl"
#include "__cone_rt_lut.inl"
#include "__noise.inl"

#ifdef ENABLE_ITT_API
__itt_string_handle *itt_exec_dr_str = __itt_string_handle_create("ExecuteDrawList");
#endif
} // namespace RendererInternal

// TODO: remove this coupling!!!
namespace SceneManagerInternal {
int WriteImage(const uint8_t *out_data, int w, int h, int channels, bool flip_y,
               bool is_rgbm, const char *name);
}

#define BBOX_POINTS(min, max)                                                            \
    (min)[0], (min)[1], (min)[2], (max)[0], (min)[1], (min)[2], (min)[0], (min)[1],      \
        (max)[2], (max)[0], (min)[1], (max)[2], (min)[0], (max)[1], (min)[2], (max)[0],  \
        (max)[1], (min)[2], (min)[0], (max)[1], (max)[2], (max)[0], (max)[1], (max)[2]

Renderer::Renderer(Ren::Context &ctx, ShaderLoader &sh,
                   std::shared_ptr<Sys::ThreadPool> threads)
    : ctx_(ctx), sh_(sh), threads_(std::move(threads)),
      shadow_splitter_(SHADOWMAP_WIDTH, SHADOWMAP_HEIGHT), rp_builder_(ctx_, sh_) {
    using namespace RendererInternal;

    {
        const float NEAR_CLIP = 0.5f;
        const float FAR_CLIP = 10000.0f;
        SWfloat z = FAR_CLIP / (FAR_CLIP - NEAR_CLIP) +
                    (NEAR_CLIP - (2.0f * NEAR_CLIP)) / (0.15f * (FAR_CLIP - NEAR_CLIP));
        swCullCtxInit(&cull_ctx_, 256, 128, z);
    }

    { // buffer used to sample probes
        FrameBuf::ColorAttachmentDesc desc;
        desc.format = Ren::eTexFormat::RawRGBA32F;
        desc.filter = Ren::eTexFilter::NoFilter;
        desc.repeat = Ren::eTexRepeat::ClampToEdge;
        probe_sample_buf_ =
            FrameBuf("Probe sample", ctx_, 24, 8, &desc, 1, {}, 1, ctx.log());
    }

    static const uint8_t black[] = {0, 0, 0, 0}, white[] = {255, 255, 255, 255};

    { // dummy 1px textures
        Ren::Tex2DParams p;
        p.w = p.h = 1;
        p.format = Ren::eTexFormat::RawRGBA8888;
        p.filter = Ren::eTexFilter::NoFilter;
        p.repeat = Ren::eTexRepeat::ClampToEdge;

        Ren::eTexLoadStatus status;
        dummy_black_ =
            ctx_.LoadTexture2D("dummy_black", black, sizeof(black), p, &status);
        assert(status == Ren::eTexLoadStatus::TexCreatedFromData);

        dummy_white_ =
            ctx_.LoadTexture2D("dummy_white", white, sizeof(white), p, &status);
        assert(status == Ren::eTexLoadStatus::TexCreatedFromData);
    }

    { // random 2d halton 8x8
        Ren::Tex2DParams p;
        p.w = p.h = 8;
        p.format = Ren::eTexFormat::RawRG32F;
        p.filter = Ren::eTexFilter::NoFilter;
        p.repeat = Ren::eTexRepeat::Repeat;

        Ren::eTexLoadStatus status;
        rand2d_8x8_ = ctx_.LoadTexture2D("rand2d_8x8", &HaltonSeq23[0][0],
                                         sizeof(HaltonSeq23), p, &status);
        assert(status == Ren::eTexLoadStatus::TexCreatedFromData);
    }

    { // random 2d directions 8x8
        Ren::Tex2DParams p;
        p.w = p.h = 4;
        p.format = Ren::eTexFormat::RawRG16;
        p.filter = Ren::eTexFilter::NoFilter;
        p.repeat = Ren::eTexRepeat::Repeat;

        Ren::eTexLoadStatus status;
        rand2d_dirs_4x4_ = ctx_.LoadTexture2D("rand2d_dirs_4x4", &__rand_dirs[0],
                                              sizeof(__rand_dirs), p, &status);
        assert(status == Ren::eTexLoadStatus::TexCreatedFromData);
    }

    { // cone/sphere intersection LUT
        /*const int resx = 128, resy = 128;

        const float cone_angles[] = {
            16.0f * Ren::Pi<float>() / 180.0f, 32.0f * Ren::Pi<float>() / 180.0f,
            48.0f * Ren::Pi<float>() / 180.0f, 64.0f * Ren::Pi<float>() / 180.0f};

        std::string str;
        const std::unique_ptr<uint8_t[]> occ_data =
            Generate_ConeTraceLUT(resx, resy, cone_angles, str);

        SceneManagerInternal::WriteImage(&occ_data[0], resx, resy, 4,
                                         false, false, "cone_lut.uncompressed.png");
        //std::exit(0);*/

        Ren::Tex2DParams p;
        p.w = __cone_rt_lut_res;
        p.h = __cone_rt_lut_res;
        p.format = Ren::eTexFormat::RawRGBA8888;
        p.filter = Ren::eTexFilter::BilinearNoMipmap;
        p.repeat = Ren::eTexRepeat::ClampToEdge;

        Ren::eTexLoadStatus status;
        cone_rt_lut_ =
            ctx_.LoadTexture2D("cone_rt_lut", &__cone_rt_lut[0],
                               4 * __cone_rt_lut_res * __cone_rt_lut_res, p, &status);

        // cone_rt_lut_ =
        //    ctx_.LoadTexture2D("cone_rt_lut", &occ_data[0], 4 * resx * resy, p,
        //    &status);
    }

    {
        // std::string c_header;
        // const std::unique_ptr<uint16_t[]> img_data_rg16 = Generate_BRDF_LUT(256,
        // c_header);

        Ren::Tex2DParams p;
        p.w = p.h = RendererInternal::__brdf_lut_res;
        p.format = Ren::eTexFormat::RawRG16U;
        p.filter = Ren::eTexFilter::BilinearNoMipmap;
        p.repeat = Ren::eTexRepeat::ClampToEdge;

        Ren::eTexLoadStatus status;
        brdf_lut_ = ctx_.LoadTexture2D("brdf_lut", &RendererInternal::__brdf_lut[0],
                                       sizeof(__brdf_lut), p, &status);
        assert(status == Ren::eTexLoadStatus::TexCreatedFromData);
    }

    {
        /*const int res = 128;
        std::string c_header;
        const std::unique_ptr<int8_t[]> img_data = Generate_PeriodicPerlin(res, c_header);
        SceneManagerInternal::WriteImage((const uint8_t*)&img_data[0], res, res, 4,
        false, "test1.png");*/

        Ren::Tex2DParams p;
        p.w = p.h = __noise_res;
        p.format = Ren::eTexFormat::RawRGBA8888Snorm;
        p.filter = Ren::eTexFilter::BilinearNoMipmap;
        p.repeat = Ren::eTexRepeat::Repeat;

        Ren::eTexLoadStatus status;
        noise_tex_ = ctx_.LoadTexture2D("noise", &__noise[0],
                                        __noise_res * __noise_res * 4, p, &status);
        assert(status == Ren::eTexLoadStatus::TexCreatedFromData);
    }

    /*{
        const int res = 128;

        // taken from old GPU-Gems article
        const float gauss_variances[] = {
            0.0064f, 0.0484f, 0.187f, 0.567f, 1.99f, 7.41f
        };

        const Ren::Vec3f diffusion_weights[] = {
            Ren::Vec3f{ 0.233f, 0.455f, 0.649f },
            Ren::Vec3f{ 0.100f, 0.336f, 0.344f },
            Ren::Vec3f{ 0.118f, 0.198f, 0.0f },
            Ren::Vec3f{ 0.113f, 0.007f, 0.007f },
            Ren::Vec3f{ 0.358f, 0.004f, 0.0f },
            Ren::Vec3f{ 0.078f, 0.0f, 0.0f }
        };

        const std::unique_ptr<uint8_t[]> img_data = Generate_SSSProfile_LUT(res, 6,
        gauss_variances, diffusion_weights);
        SceneManagerInternal::WriteImage(&img_data[0], res, res, 4, false,
        "assets/textures/skin_diffusion.uncompressed.png");
    }*/

    // Compile built-in shaders etc.
    InitRendererInternal();

    temp_sub_frustums_.realloc(REN_CELLS_COUNT);
    temp_sub_frustums_.count = REN_CELLS_COUNT;

    decals_boxes_.realloc(REN_MAX_DECALS_TOTAL);
    litem_to_lsource_.realloc(REN_MAX_LIGHTS_TOTAL);
    ditem_to_decal_.realloc(REN_MAX_DECALS_TOTAL);
    allocated_shadow_regions_.realloc(REN_MAX_SHADOWMAPS_TOTAL);

    for (int i = 0; i < 2; i++) {
        temp_sort_spans_32_[i].realloc(REN_MAX_SHADOW_BATCHES);
        temp_sort_spans_64_[i].realloc(REN_MAX_MAIN_BATCHES);
    }

    proc_objects_.realloc(REN_MAX_OBJ_COUNT);
}

Renderer::~Renderer() {
    prim_draw_.CleanUp(ctx_);
    DestroyRendererInternal();
    swCullCtxDestroy(&cull_ctx_);
}

void Renderer::PrepareDrawList(const SceneData &scene, const Ren::Camera &cam,
                               DrawList &list) {
    GatherDrawables(scene, cam, list);
}

void Renderer::ExecuteDrawList(const DrawList &list, const FrameBuf *target) {
    using namespace RendererInternal;

#ifdef ENABLE_ITT_API
    __itt_task_begin(__g_itt_domain, __itt_null, __itt_null, itt_exec_dr_str);
#endif

    const int cur_scr_w = ctx_.w(), cur_scr_h = ctx_.h();
    Ren::ILog *log = ctx_.log();

    if (!cur_scr_w || !cur_scr_h) {
        // view is minimized?
        return;
    }

    if (!prim_draw_.LazyInit(ctx_)) {
        log->Error("[Renderer] Failed to initialize primitive drawing!");
    }

    const bool cur_msaa_enabled = (list.render_flags & EnableMsaa) != 0;
    const bool cur_taa_enabled = (list.render_flags & EnableTaa) != 0;
    const bool cur_dof_enabled = (list.render_flags & EnableDOF) != 0;

    uint64_t gpu_draw_start = 0;
    if (list.render_flags & DebugTimings) {
        gpu_draw_start = GetGpuTimeBlockingUs();
    }
    const uint64_t cpu_draw_start_us = Sys::GetTimeUs();

    if (cur_scr_w != view_state_.scr_res[0] || cur_scr_h != view_state_.scr_res[1] ||
        cur_msaa_enabled != view_state_.is_multisampled ||
        cur_taa_enabled != taa_enabled_ || cur_dof_enabled != dof_enabled_) {

        if (cur_taa_enabled) {
            Ren::Tex2DParams params;
            params.w = cur_scr_w;
            params.h = cur_scr_h;
            params.format = Ren::eTexFormat::RawRG11F_B10F;
            params.filter = Ren::eTexFilter::BilinearNoMipmap;
            params.repeat = Ren::eTexRepeat::ClampToEdge;

            Ren::eTexLoadStatus status;
            history_tex_ = ctx_.LoadTexture2D("History tex", params, &status);
            assert(status == Ren::eTexLoadStatus::TexCreatedDefault ||
                   status == Ren::eTexLoadStatus::TexFoundReinitialized);

            log->Info("Setting texture lod bias to -1.0");

            // TODO: Replace this with usage of sampler objects
            int counter = 0;
            ctx_.VisitTextures(Ren::TexUsageScene, [&counter](Ren::Texture2D &tex) {
                Ren::Tex2DParams p = tex.params();
                if (p.lod_bias > -1.0f) {
                    p.lod_bias = -1.0f;
                    tex.SetFilter(p.filter, p.repeat, p.compare, p.lod_bias);
                    ++counter;
                }
            });

            log->Info("Textures processed: %i", counter);
        } else {
            history_tex_ = {};

            log->Info("Setting texture lod bias to 0.0");

            int counter = 0;
            ctx_.VisitTextures(Ren::TexUsageScene, [&counter](Ren::Texture2D &tex) {
                Ren::Tex2DParams p = tex.params();
                if (p.lod_bias < 0.0f) {
                    p.lod_bias = 0.0f;
                    tex.SetFilter(p.filter, p.repeat, p.compare, p.lod_bias);
                    ++counter;
                }
            });

            log->Info("Textures processed: %i", counter);
        }
        { // Texture that holds previous frame (used for SSR)
            Ren::Tex2DParams params;
            params.w = cur_scr_w / 4;
            params.h = cur_scr_h / 4;
            params.format = Ren::eTexFormat::RawRG11F_B10F;
            params.filter = Ren::eTexFilter::BilinearNoMipmap;
            params.repeat = Ren::eTexRepeat::ClampToEdge;

            Ren::eTexLoadStatus status;
            down_tex_4x_ = ctx_.LoadTexture2D("DOWN 4x", params, &status);
            assert(status == Ren::eTexLoadStatus::TexCreatedDefault ||
                   status == Ren::eTexLoadStatus::TexFoundReinitialized);
        }

        view_state_.scr_res = Ren::Vec2i{cur_scr_w, cur_scr_h};
        view_state_.is_multisampled = cur_msaa_enabled;
        taa_enabled_ = cur_taa_enabled;
        dof_enabled_ = cur_dof_enabled;
        log->Info("Successfully initialized framebuffers %ix%i", view_state_.scr_res[0],
                  view_state_.scr_res[1]);
    }

    if (!target) {
        // TODO: Change actual resolution dynamically
#if defined(__ANDROID__)
        view_state_.act_res[0] = int(float(view_state_.scr_res[0]) * 0.4f);
        view_state_.act_res[1] = int(float(view_state_.scr_res[1]) * 0.6f);
#else
        view_state_.act_res[0] = int(float(view_state_.scr_res[0]) * 1.0f);
        view_state_.act_res[1] = int(float(view_state_.scr_res[1]) * 1.0f);
#endif
    } else {
        view_state_.act_res[0] = target->w;
        view_state_.act_res[1] = target->h;
    }
    assert(view_state_.act_res[0] <= view_state_.scr_res[0] &&
           view_state_.act_res[1] <= view_state_.scr_res[1]);

    cur_buf_chunk_ = (cur_buf_chunk_ + 1) % FrameSyncWindow;

    if ((list.render_flags & EnableTaa) != 0) {
        Ren::Vec2f jitter = HaltonSeq23[frame_counter_ % TaaSampleCount];
        jitter = (jitter * 2.0f - Ren::Vec2f{1.0f}) / Ren::Vec2f{view_state_.act_res};

        list.draw_cam.SetPxOffset(jitter);
    } else {
        list.draw_cam.SetPxOffset(Ren::Vec2f{0.0f, 0.0f});
    }

    { // Setup render passes
        rp_builder_.Reset();

        //
        // Update buffers
        //
        rp_update_buffers_.Setup(rp_builder_, list, &view_state_, cur_buf_chunk_,
                                 buf_range_fences_, SKIN_TRANSFORMS_BUF, SHAPE_KEYS_BUF,
                                 INSTANCES_BUF, CELLS_BUF, LIGHTS_BUF, DECALS_BUF,
                                 ITEMS_BUF, SHARED_DATA_BUF);
        RenderPassBase *rp_head = &rp_update_buffers_;
        RenderPassBase *rp_tail = &rp_update_buffers_;

        //
        // Skinning and blend shapes
        //
        rp_skinning_.Setup(rp_builder_, list, cur_buf_chunk_, ctx_.default_vertex_buf1(),
                           ctx_.default_vertex_buf2(), ctx_.default_delta_buf(),
                           ctx_.default_skin_vertex_buf(), SKIN_TRANSFORMS_BUF,
                           SHAPE_KEYS_BUF);
        rp_tail->p_next = &rp_skinning_;
        rp_tail = rp_tail->p_next;

        //
        // Shadow maps
        //
        rp_shadow_maps_.Setup(rp_builder_, list, cur_buf_chunk_, INSTANCES_BUF,
                              SHADOWMAP_TEX);
        rp_tail->p_next = &rp_shadow_maps_;
        rp_tail = rp_tail->p_next;

        //
        // Skydome drawing
        //
        if ((list.render_flags & DebugWireframe) == 0 && list.env.env_map) {
            rp_skydome_.Setup(rp_builder_, list, &view_state_, cur_buf_chunk_,
                              SHARED_DATA_BUF, MAIN_COLOR_TEX, MAIN_SPEC_TEX,
                              MAIN_DEPTH_TEX);
            rp_tail->p_next = &rp_skydome_;
            rp_tail = rp_tail->p_next;
        } else {
            // TODO: ...
        }

        //
        // Depth prepass
        //
        if (list.render_flags & EnableZFill) {
            rp_depth_fill_.Setup(rp_builder_, list, &view_state_, cur_buf_chunk_,
                                 INSTANCES_BUF, SHARED_DATA_BUF, MAIN_DEPTH_TEX,
                                 MAIN_VELOCITY_TEX);
            rp_tail->p_next = &rp_depth_fill_;
            rp_tail = rp_tail->p_next;
        }

        //
        // Downsample depth
        //
        if ((list.render_flags & EnableZFill) &&
            (list.render_flags & (EnableSSAO | EnableSSR)) &&
            ((list.render_flags & DebugWireframe) == 0)) {
            rp_down_depth_.Setup(rp_builder_, &view_state_, cur_buf_chunk_,
                                 SHARED_DATA_BUF, MAIN_DEPTH_TEX, DEPTH_DOWN_2X_TEX);
            rp_tail->p_next = &rp_down_depth_;
            rp_tail = rp_tail->p_next;
        }

        //
        // Ambient occlusion
        //
        const uint32_t use_ssao_mask = (EnableZFill | EnableSSAO | DebugWireframe);
        const uint32_t use_ssao = (EnableZFill | EnableSSAO);
        if ((list.render_flags & use_ssao_mask) == use_ssao) {
            rp_ssao_.Setup(rp_builder_, &view_state_, cur_buf_chunk_,
                           rand2d_dirs_4x4_->handle(), SHARED_DATA_BUF, DEPTH_DOWN_2X_TEX,
                           MAIN_DEPTH_TEX, SSAO_TEX);
            rp_tail->p_next = &rp_ssao_;
            rp_tail = rp_tail->p_next;
        }

        //
        // Opaque pass
        //
        rp_opaque_.Setup(rp_builder_, list, &view_state_, cur_buf_chunk_, brdf_lut_,
                         noise_tex_, cone_rt_lut_, INSTANCES_BUF, SHARED_DATA_BUF,
                         CELLS_BUF, ITEMS_BUF, LIGHTS_BUF, DECALS_BUF, SHADOWMAP_TEX,
                         SSAO_TEX, MAIN_COLOR_TEX, MAIN_NORMAL_TEX, MAIN_SPEC_TEX,
                         MAIN_DEPTH_TEX);
        rp_tail->p_next = &rp_opaque_;
        rp_tail = rp_tail->p_next;

        //
        // Resolve ms buffer
        //
        if ((list.render_flags & EnableOIT) && cur_msaa_enabled) {
            rp_resolve_.Setup(rp_builder_, &view_state_, MAIN_COLOR_TEX,
                              RESOLVED_COLOR_TEX);
        }

        //
        // Transparent pass
        //

        rp_transparent_.Setup(rp_builder_, list, &rp_opaque_.alpha_blend_start_index_,
                              &view_state_, cur_buf_chunk_, brdf_lut_, noise_tex_,
                              cone_rt_lut_, INSTANCES_BUF, SHARED_DATA_BUF, CELLS_BUF,
                              ITEMS_BUF, LIGHTS_BUF, DECALS_BUF, SHADOWMAP_TEX, SSAO_TEX,
                              MAIN_COLOR_TEX, MAIN_NORMAL_TEX, MAIN_SPEC_TEX,
                              MAIN_DEPTH_TEX, RESOLVED_COLOR_TEX);
        rp_tail->p_next = &rp_transparent_;
        rp_tail = rp_tail->p_next;

        //
        // Reflections pass
        //

        const char *refl_out_name =
            view_state_.is_multisampled ? RESOLVED_COLOR_TEX : MAIN_COLOR_TEX;

        rp_reflections_.Setup(rp_builder_, &view_state_, cur_buf_chunk_,
                              list.probe_storage, down_tex_4x_->handle(), brdf_lut_,
                              SHARED_DATA_BUF, CELLS_BUF, ITEMS_BUF, MAIN_DEPTH_TEX,
                              MAIN_NORMAL_TEX, MAIN_SPEC_TEX, DEPTH_DOWN_2X_TEX,
                              refl_out_name);
        rp_tail->p_next = &rp_reflections_;
        rp_tail = rp_tail->p_next;

        //
        // Debug geometry
        //
        if (list.render_flags & DebugProbes) {
            rp_debug_probes_.Setup(rp_builder_, list, &view_state_, cur_buf_chunk_,
                                   SHARED_DATA_BUF, refl_out_name);
            rp_tail->p_next = &rp_debug_probes_;
            rp_tail = rp_tail->p_next;
        }

        if (list.render_flags & DebugEllipsoids) {
            rp_debug_ellipsoids_.Setup(rp_builder_, list, &view_state_, cur_buf_chunk_,
                                       SHARED_DATA_BUF, refl_out_name);
            rp_tail->p_next = &rp_debug_ellipsoids_;
            rp_tail = rp_tail->p_next;
        }

        //
        // Temporal resolve
        //
        if (list.render_flags & EnableTaa) {
            assert(!view_state_.is_multisampled);
            rp_taa_.Setup(rp_builder_, &view_state_, cur_buf_chunk_,
                          history_tex_->handle(), reduced_average_,
                          list.draw_cam.max_exposure, SHARED_DATA_BUF, MAIN_COLOR_TEX,
                          MAIN_DEPTH_TEX, MAIN_VELOCITY_TEX, RESOLVED_COLOR_TEX);
            rp_tail->p_next = &rp_taa_;
            rp_tail = rp_tail->p_next;
        }

        //
        // Color downsampling
        //
        if ((list.render_flags & (EnableSSR | EnableBloom | EnableTonemap | EnableDOF)) &&
            ((list.render_flags & DebugWireframe) == 0)) {
            rp_down_color_.Setup(rp_builder_, &view_state_, cur_buf_chunk_,
                                 SHARED_DATA_BUF, refl_out_name, down_tex_4x_->handle());
            rp_tail->p_next = &rp_down_color_;
            rp_tail = rp_tail->p_next;
        }

        const bool apply_dof = (list.render_flags & EnableDOF) &&
                               list.draw_cam.focus_near_mul > 0.0f &&
                               list.draw_cam.focus_far_mul > 0.0f &&
                               ((list.render_flags & DebugWireframe) == 0);

        if (apply_dof) {
            const int qres_w = cur_scr_w / 4, qres_h = cur_scr_h / 4;

            const char *color_in_name = nullptr;
            const char *dof_out_name = refl_out_name;

            if (view_state_.is_multisampled) {
                // color_tex = resolved_or_transparent_tex_->handle();
                color_in_name = RESOLVED_COLOR_TEX;
            } else {
                if ((list.render_flags & EnableTaa) != 0u) {
                    // color_tex = resolved_or_transparent_tex_->handle();
                    color_in_name = RESOLVED_COLOR_TEX;
                } else {
                    // color_tex = color_tex_->handle();
                    color_in_name = MAIN_COLOR_TEX;
                }
            }

            rp_dof_.Setup(rp_builder_, &list.draw_cam, &view_state_, cur_buf_chunk_,
                          SHARED_DATA_BUF, color_in_name, MAIN_DEPTH_TEX,
                          DEPTH_DOWN_2X_TEX, DEPTH_DOWN_4X_TEX, down_tex_4x_->handle(),
                          dof_out_name);
            rp_tail->p_next = &rp_dof_;
            rp_tail = rp_tail->p_next;
        }

        //
        // Blur
        //
        if ((list.render_flags & (EnableSSR | EnableBloom | EnableTonemap)) &&
            ((list.render_flags & DebugWireframe) == 0)) {
            rp_blur_.Setup(rp_builder_, &view_state_, down_tex_4x_->handle(),
                           BLUR_RES_TEX);
            rp_tail->p_next = &rp_blur_;
            rp_tail = rp_tail->p_next;
        }

        //
        // Fence
        //
        rp_fence_.Setup(rp_builder_, cur_buf_chunk_, buf_range_fences_);
        rp_tail->p_next = &rp_fence_;
        rp_tail = rp_tail->p_next;

        //
        // Sample brightness
        //
        if (list.render_flags & EnableTonemap) {
            rp_sample_brightness_.Setup(rp_builder_, down_tex_4x_->handle(), REDUCED_TEX);
            rp_tail->p_next = &rp_sample_brightness_;
            rp_tail = rp_tail->p_next;
        }

        //
        // Combine with blurred and tonemap
        //
        {
            const char *color_tex = nullptr;
            const char *output_tex = nullptr;
            const char *blur_tex = nullptr;

            if (cur_msaa_enabled || ((list.render_flags & EnableTaa) != 0) || apply_dof) {
                if (apply_dof) {
                    if ((list.render_flags & EnableTaa) != 0) {
                        color_tex = MAIN_COLOR_TEX;
                    } else {
                        color_tex = DOF_COLOR_TEX;
                    }
                } else {
                    color_tex = RESOLVED_COLOR_TEX;
                }
            } else {
                color_tex = MAIN_COLOR_TEX;
            }

            if ((list.render_flags & EnableBloom) &&
                !(list.render_flags & DebugWireframe)) {
                blur_tex = BLUR_RES_TEX;
            }

            if ((list.render_flags & EnableFxaa) &&
                !(list.render_flags & DebugWireframe)) {
                output_tex = MAIN_COMBINED_TEX;
            } else {
                if (!target) {
                    // output to back buffer
                    output_tex = nullptr;
                } else {
                    // TODO: fix this!!!
                    // output_tex = target->attachments[0].tex->handle();
                }
            }

            float gamma = 1.0f;
            if ((list.render_flags & EnableTonemap) &&
                !(list.render_flags & DebugLights)) {
                gamma = 2.2f;
            }

            const bool tonemap = (list.render_flags & EnableTonemap);
            const float reduced_average = rp_sample_brightness_.reduced_average();

            float exposure = reduced_average > std::numeric_limits<float>::epsilon()
                                 ? (1.0f / reduced_average)
                                 : 1.0f;
            exposure = std::min(exposure, list.draw_cam.max_exposure);

            rp_combine_.Setup(rp_builder_, &view_state_, gamma, exposure,
                              list.draw_cam.fade, tonemap, color_tex, blur_tex,
                              output_tex);
            rp_tail->p_next = &rp_combine_;
            rp_tail = rp_tail->p_next;
        }

        //
        // FXAA
        //
        if ((list.render_flags & EnableFxaa) && !(list.render_flags & DebugWireframe)) {
            // Ren::TexHandle output_tex =
            //    target ? target->attachments[0].tex->handle() : Ren::TexHandle{};

            rp_fxaa_.Setup(rp_builder_, &view_state_, cur_buf_chunk_, SHARED_DATA_BUF,
                           MAIN_COMBINED_TEX, nullptr);
            rp_tail->p_next = &rp_fxaa_;
            rp_tail = rp_tail->p_next;
        }

        //
        // Debugging (draw auxiliary surfaces)
        //
        {
            Ren::TexHandle output_tex =
                target ? target->attachments[0].tex->handle() : Ren::TexHandle{};

            rp_debug_textures_.Setup(rp_builder_, &view_state_, list, cur_buf_chunk_,
                                     down_tex_4x_, SHARED_DATA_BUF, CELLS_BUF, ITEMS_BUF,
                                     SHADOWMAP_TEX, MAIN_COLOR_TEX, MAIN_NORMAL_TEX,
                                     MAIN_SPEC_TEX, MAIN_DEPTH_TEX, SSAO_TEX,
                                     BLUR_RES_TEX, REDUCED_TEX, output_tex);
            rp_tail->p_next = &rp_debug_textures_;
            rp_tail = rp_tail->p_next;
        }

        //
        // Compile and execute
        //
        rp_tail->p_next = nullptr;

        rp_builder_.Compile(rp_head);
        rp_builder_.Execute(rp_head);
    }

    { // store matrix to use it in next frame
        view_state_.down_buf_view_from_world = list.draw_cam.view_matrix();
        view_state_.prev_clip_from_world =
            list.draw_cam.proj_matrix() * list.draw_cam.view_matrix();
        view_state_.prev_clip_from_view = list.draw_cam.proj_matrix_offset();
    }

    const uint64_t cpu_draw_end_us = Sys::GetTimeUs();

    // store values for current frame
    backend_cpu_start_ = cpu_draw_start_us;
    backend_cpu_end_ = cpu_draw_end_us;
    backend_time_diff_ = int64_t(gpu_draw_start) - int64_t(backend_cpu_start_);
    frame_counter_++;

#ifdef ENABLE_ITT_API
    __itt_task_end(__g_itt_domain);
#endif
}

#undef BBOX_POINTS
