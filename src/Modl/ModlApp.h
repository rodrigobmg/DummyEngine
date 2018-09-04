#pragma once

#include <string>
#include <vector>

#include <Ren/Camera.h>
#include <Ren/Context.h>
#include <Ren/Mesh.h>

struct SDL_Window;

class ModlApp {
public:
    ModlApp();

    int Run(const std::vector<std::string> &args);

    int Init(int w, int h);
    void Frame();
    void PollEvents();
    void Destroy();

    bool terminated() const {
        return quit_;
    }
private:
    enum eCompileResult { RES_SUCCESS = 0, RES_PARSE_ERROR, RES_FILE_NOT_FOUND };
    int CompileModel(const std::string &in_file_name, const std::string &out_file_name);
    int CompileAnim(const std::string &in_file_name, const std::string &out_file_name);

    bool quit_;
    SDL_Window *window_;
#if defined(USE_GL_RENDER)
    void *gl_ctx_;
#endif

    Ren::MeshRef view_mesh_;
    Ren::AnimSeqRef anim_seq_;
    Ren::Camera cam_;
    Ren::Context ctx_;

    Ren::ProgramRef diag_prog_;
    Ren::Texture2DRef checker_tex_;

    float angle_x_ = 0.0f, angle_y_ = 0.0f;
    bool mouse_grabbed_ = false;

    enum eViewMode { Material, DiagNormals1, DiagNormals2, DiagUVs1, DiagUVs2 } view_mode_ = DiagNormals1;

    void DrawMeshSimple(Ren::MeshRef &ref);
    void DrawMeshSkeletal(Ren::MeshRef &ref, float dt_s);

    void PrintUsage();

    Ren::Texture2DRef OnTextureNeeded(const char *name);
    Ren::ProgramRef OnProgramNeeded(const char *name, const char *vs_shader, const char *fs_shader);
    Ren::MaterialRef OnMaterialNeeded(const char *name);

    void ClearColorAndDepth(float r, float g, float b, float a);
};