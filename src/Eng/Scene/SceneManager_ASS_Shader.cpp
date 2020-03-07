#include "SceneManager.h"

#include <fstream>

#include "../Renderer/Renderer_GL_Defines.inl"

void SceneManager::WriteCommonShaderIncludes(const char *in_folder) {
    static const char vs_common[] =
#include "../Renderer/Shaders/_vs_common.glsl"
        ;

    std::string out_file_name = in_folder;
    out_file_name += "/shaders/common_vs.glsl";

    std::ofstream out_file(out_file_name, std::ios::binary);
    out_file.write(vs_common, sizeof(vs_common) - 1);
}

void SceneManager::InlineShaderConstants(assets_context_t &ctx, std::string &line) {
    static bool constants_initialized = false;
    static Ren::HashMap32<std::string, std::string> shader_constants;
    if (!constants_initialized) {
        shader_constants.Insert("$ModifyWarning",
                "/***********************************************/\r\n"
                "/* This file was autogenerated, do not modify! */\r\n"
                "/***********************************************/");

        shader_constants.Insert("$FltEps",          "0.0000001");

        shader_constants.Insert("$ItemGridResX",    AS_STR(REN_GRID_RES_X));
        shader_constants.Insert("$ItemGridResY",    AS_STR(REN_GRID_RES_Y));
        shader_constants.Insert("$ItemGridResZ",    AS_STR(REN_GRID_RES_Z));

        // Vertex attributes
        shader_constants.Insert("$VtxPosLoc",       AS_STR(REN_VTX_POS_LOC));
        shader_constants.Insert("$VtxNorLoc",       AS_STR(REN_VTX_NOR_LOC));
        shader_constants.Insert("$VtxTanLoc",       AS_STR(REN_VTX_TAN_LOC));
        shader_constants.Insert("$VtxUV1Loc",       AS_STR(REN_VTX_UV1_LOC));
        shader_constants.Insert("$VtxAUXLoc",       AS_STR(REN_VTX_AUX_LOC));

        // Texture slots
        shader_constants.Insert("$MatTex0Slot",     AS_STR(REN_MAT_TEX0_SLOT));
        shader_constants.Insert("$MatTex1Slot",     AS_STR(REN_MAT_TEX1_SLOT));
        shader_constants.Insert("$MatTex2Slot",     AS_STR(REN_MAT_TEX2_SLOT));
        shader_constants.Insert("$ShadTexSlot",     AS_STR(REN_SHAD_TEX_SLOT));
        //shader_constants.Insert("$LmapDirSlot",    AS_STR(REN_LMAP_DIR_SLOT));
        //shader_constants.Insert("$LmapIndirSlot",  AS_STR(REN_LMAP_INDIR_SLOT));
        shader_constants.Insert("$LmapSHSlot",      AS_STR(REN_LMAP_SH_SLOT));
        shader_constants.Insert("$DecalTexSlot",    AS_STR(REN_DECAL_TEX_SLOT));
        shader_constants.Insert("$SSAOTexSlot",     AS_STR(REN_SSAO_TEX_SLOT));
        shader_constants.Insert("$BRDFLutTexSlot",  AS_STR(REN_BRDF_TEX_SLOT));
        shader_constants.Insert("$LightBufSlot",    AS_STR(REN_LIGHT_BUF_SLOT));
        shader_constants.Insert("$DecalBufSlot",    AS_STR(REN_DECAL_BUF_SLOT));
        shader_constants.Insert("$CellsBufSlot",    AS_STR(REN_CELLS_BUF_SLOT));
        shader_constants.Insert("$ItemsBufSlot",    AS_STR(REN_ITEMS_BUF_SLOT));
        shader_constants.Insert("$InstanceBufSlot", AS_STR(REN_INST_BUF_SLOT));
        shader_constants.Insert("$EnvTexSlot",      AS_STR(REN_ENV_TEX_SLOT));
        shader_constants.Insert("$Moments0TexSlot", AS_STR(REN_MOMENTS0_TEX_SLOT));
        shader_constants.Insert("$Moments1TexSlot", AS_STR(REN_MOMENTS1_TEX_SLOT));
        shader_constants.Insert("$Moments2TexSlot", AS_STR(REN_MOMENTS2_TEX_SLOT));
        shader_constants.Insert("$Moments0MsTexSlot", AS_STR(REN_MOMENTS0_MS_TEX_SLOT));
        shader_constants.Insert("$Moments1MsTexSlot", AS_STR(REN_MOMENTS1_MS_TEX_SLOT));
        shader_constants.Insert("$Moments2MsTexSlot", AS_STR(REN_MOMENTS2_MS_TEX_SLOT));
        shader_constants.Insert("$NoiseTexSlot", AS_STR(REN_NOISE_TEX_SLOT));

        // Uniform locations
        shader_constants.Insert("$uMMatrixLoc",     AS_STR(REN_U_M_MATRIX_LOC));
        shader_constants.Insert("$uInstancesLoc",   AS_STR(REN_U_INSTANCES_LOC));

        // Uniform block locations
        shader_constants.Insert("$ubSharedDataLoc", AS_STR(REN_UB_SHARED_DATA_LOC));
        shader_constants.Insert("$ubBatchDataLoc",  AS_STR(REN_UB_BATCH_DATA_LOC));

        // Shader output channels
        shader_constants.Insert("$OutColorIndex",   AS_STR(REN_OUT_COLOR_INDEX));
        shader_constants.Insert("$OutNormIndex",    AS_STR(REN_OUT_NORM_INDEX));
        shader_constants.Insert("$OutSpecIndex",    AS_STR(REN_OUT_SPEC_INDEX));

        // Shadow properties
        if (strcmp(ctx.platform, "pc") == 0) {
            shader_constants.Insert("$ShadRes",     AS_STR(REN_SHAD_RES_PC));
        } else if (strcmp(ctx.platform, "android") == 0) {
            shader_constants.Insert("$ShadRes",     AS_STR(REN_SHAD_RES_ANDROID));
        } else {
            ctx.log->Error("Unknown platform %s", ctx.platform);
            return;
        }

        shader_constants.Insert("$ShadCasc0Dist",   AS_STR(REN_SHAD_CASCADE0_DIST));
        shader_constants.Insert("$ShadCasc0Samp",   AS_STR(REN_SHAD_CASCADE0_SAMPLES));
        shader_constants.Insert("$ShadCasc1Dist",   AS_STR(REN_SHAD_CASCADE1_DIST));
        shader_constants.Insert("$ShadCasc1Samp",   AS_STR(REN_SHAD_CASCADE1_SAMPLES));
        shader_constants.Insert("$ShadCasc2Dist",   AS_STR(REN_SHAD_CASCADE2_DIST));
        shader_constants.Insert("$ShadCasc2Samp",   AS_STR(REN_SHAD_CASCADE2_SAMPLES));
        shader_constants.Insert("$ShadCasc3Dist",   AS_STR(REN_SHAD_CASCADE3_DIST));
        shader_constants.Insert("$ShadCasc3Samp",   AS_STR(REN_SHAD_CASCADE3_SAMPLES));
        shader_constants.Insert("$ShadCascSoft",    AS_STR(REN_SHAD_CASCADE_SOFT));

        shader_constants.Insert("$MaxShadowMaps",   AS_STR(REN_MAX_SHADOWMAPS_TOTAL));
        shader_constants.Insert("$MaxProbes",       AS_STR(REN_MAX_PROBES_TOTAL));

        shader_constants.Insert("$MaxBatchSize",    AS_STR(REN_MAX_BATCH_SIZE));

        constants_initialized = true;
    }

    size_t n = 0;
    while ((n = line.find('$', n)) != std::string::npos) {
        size_t l = 1;

        const char punctuation_chars[] = ".,(); $*[]\r\n";
        while (std::find(std::begin(punctuation_chars), std::end(punctuation_chars), line[n + l]) == std::end(punctuation_chars)) {
            l++;
        }

        const std::string var = line.substr(n, l);

        const std::string *it = shader_constants.Find(var);
        if (it) {
            line.replace(n, l, *it);
        } else {
            ctx.log->Error("Unknown variable %s", var.c_str());
            throw std::runtime_error("Unknown variable!");
        }
    }
}

void SceneManager::HPreprocessShader(assets_context_t &ctx, const char *in_file, const char *out_file) {
    ctx.log->Info("[PrepareAssets] Prep %s", out_file);

    {   // resolve includes, inline constants
        std::ifstream src_stream(in_file, std::ios::binary);
        std::ofstream dst_stream(out_file, std::ios::binary);
        std::string line;

        int line_counter = 0;

        while (std::getline(src_stream, line)) {
            if (!line.empty() && line.back() == '\r') {
                line = line.substr(0, line.size() - 1);
            }

            if (line.rfind("#version ") == 0) {
                if (strcmp(ctx.platform, "pc") == 0) {
                    line = "#version 430";
                }
                dst_stream << line << "\r\n";
            } else if (line.rfind("#include ") == 0) {
                size_t n1 = line.find_first_of('\"');
                size_t n2 = line.find_last_of('\"');

                std::string file_name = line.substr(n1 + 1, n2 - n1 - 1);

                auto slash_pos = (size_t)intptr_t(strrchr(in_file, '/') - in_file);
                std::string full_path = std::string(in_file, slash_pos + 1) + file_name;

                dst_stream << "#line 0\r\n";

                std::ifstream incl_stream(full_path, std::ios::binary);
                while (std::getline(incl_stream, line)) {
                    if (!line.empty() && line.back() == '\r') {
                        line = line.substr(0, line.size() - 1);
                    }

                    InlineShaderConstants(ctx, line);

                    dst_stream << line << "\r\n";
                }

                dst_stream << "\r\n#line " << line_counter << "\r\n";
            } else {
                InlineShaderConstants(ctx, line);

                dst_stream << line << "\r\n";
            }

            line_counter++;
        }
    }

    if (strcmp(ctx.platform, "pc") == 0) {
        std::string spv_file = out_file;

        size_t n;
        if ((n = spv_file.find(".glsl")) != std::string::npos) {
            spv_file.replace(n + 1, 4, "spv", 3);
        }

        std::string compile_cmd = "src/libs/spirv/glslangValidator -G ";
        compile_cmd += out_file;
        compile_cmd += " -o ";
        compile_cmd += spv_file;

#ifdef _WIN32
        std::replace(compile_cmd.begin(), compile_cmd.end(), '/', '\\');
#endif
        int res = system(compile_cmd.c_str());
        if (res != 0) {
            ctx.log->Error("[PrepareAssets] Failed to compile %s", spv_file.c_str());
        }

        std::string optimize_cmd = "src/libs/spirv/spirv-opt "
                                   "--eliminate-dead-branches "
                                   "--merge-return "
                                   "--inline-entry-points-exhaustive "
                                   "--loop-unswitch --loop-unroll "
                                   "--eliminate-dead-code-aggressive "
                                   "--private-to-local "
                                   "--eliminate-local-single-block "
                                   "--eliminate-local-single-store "
                                   "--eliminate-dead-code-aggressive "
                                   //"--scalar-replacement=100 "
                                   "--convert-local-access-chains "
                                   "--eliminate-local-single-block "
                                   "--eliminate-local-single-store "
                                   //"--eliminate-dead-code-aggressive "
                                   //"--eliminate-local-multi-store "
                                   //"--eliminate-dead-code-aggressive "
                                   "--ccp "
                                   //"--eliminate-dead-code-aggressive "
                                   "--redundancy-elimination "
                                   "--combine-access-chains "
                                   "--simplify-instructions "
                                   "--vector-dce "
                                   "--eliminate-dead-inserts "
                                   "--eliminate-dead-branches "
                                   "--simplify-instructions "
                                   "--if-conversion "
                                   "--copy-propagate-arrays "
                                   "--reduce-load-size "
                                   //"--eliminate-dead-code-aggressive "
                                   //"--merge-blocks "
                                   "--redundancy-elimination "
                                   "--eliminate-dead-branches "
                                   //"--merge-blocks "
                                   "--simplify-instructions "
                                   "--validate-after-all ";

        optimize_cmd += spv_file;
        optimize_cmd += " -o ";
        optimize_cmd += spv_file;

#ifdef _WIN32
        std::replace(optimize_cmd.begin(), optimize_cmd.end(), '/', '\\');
#endif
        res = system(optimize_cmd.c_str());
        if (res != 0) {
            ctx.log->Error("[PrepareAssets] Failed to optimize %s", spv_file.c_str());
        }

        std::string cross_cmd = "src/libs/spirv/spirv-cross ";
        if (strcmp(ctx.platform, "pc") == 0) {
            cross_cmd += "--version 430 ";
        } else if (strcmp(ctx.platform, "android") == 0) {
            cross_cmd += "--version 310 --es ";
            cross_cmd += "--extension GL_EXT_texture_buffer ";
        }
        cross_cmd += "--no-support-nonzero-baseinstance --glsl-emit-push-constant-as-ubo ";
        cross_cmd += spv_file;
        cross_cmd += " --output ";
        cross_cmd += out_file;

#ifdef _WIN32
        std::replace(cross_cmd.begin(), cross_cmd.end(), '/', '\\');
#endif
        res = system(cross_cmd.c_str());
        if (res != 0) {
            ctx.log->Error("[PrepareAssets] Failed to cross-compile %s", spv_file.c_str());
        }
    }
}