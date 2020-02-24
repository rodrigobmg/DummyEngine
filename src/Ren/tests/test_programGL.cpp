#include "test_common.h"

#include "../Context.h"
#include "../GL.h"
#include "../Material.h"

#if defined(_WIN32)
#include <Windows.h>

#define WGL_CONTEXT_MAJOR_VERSION_ARB 0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB 0x2092
#define WGL_CONTEXT_FLAGS_ARB 0x2094
#define WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB 0x00000002

#else
#include <SDL2/SDL.h>
#endif

class ProgramTest : public Ren::Context {
#if defined(_WIN32)
    HINSTANCE hInstance;
    HWND hWnd;
    HDC hDC;
    HGLRC hRC;
#else
    SDL_Window *window_;
    void *gl_ctx_main_;
#endif
    Ren::LogNull log_;

  public:
    ProgramTest() {
#if defined(_WIN32)
        hInstance = GetModuleHandle(NULL);
        WNDCLASS wc;
        wc.style = CS_OWNDC;
        wc.lpfnWndProc = ::DefWindowProc;
        wc.cbClsExtra = 0;
        wc.cbWndExtra = 0;
        wc.hInstance = hInstance;
        wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = NULL;
        wc.lpszMenuName = NULL;
        wc.lpszClassName = "ProgramTest";

        if (!RegisterClass(&wc)) {
            throw std::runtime_error("Cannot register window class!");
        }

        hWnd = CreateWindow("ProgramTest", "!!",
                            WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0, 0,
                            100, 100, NULL, NULL, hInstance, NULL);

        if (hWnd == NULL) {
            throw std::runtime_error("Cannot create window!");
        }

        hDC = GetDC(hWnd);

        PIXELFORMATDESCRIPTOR pfd;
        memset(&pfd, 0, sizeof(pfd));
        pfd.nSize = sizeof(pfd);
        pfd.nVersion = 1;
        pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL;
        pfd.iPixelType = PFD_TYPE_RGBA;
        pfd.cColorBits = 32;

        int pf = ChoosePixelFormat(hDC, &pfd);
        if (pf == 0) {
            throw std::runtime_error("Cannot find pixel format!");
        }

        if (SetPixelFormat(hDC, pf, &pfd) == FALSE) {
            throw std::runtime_error("Cannot set pixel format!");
        }

        DescribePixelFormat(hDC, pf, sizeof(PIXELFORMATDESCRIPTOR), &pfd);

        HGLRC temp_context = wglCreateContext(hDC);
        wglMakeCurrent(hDC, temp_context);

        typedef HGLRC(APIENTRY * PFNWGLCREATECONTEXTATTRIBSARBPROC)(
            HDC hDC, HGLRC hShareContext, const int *attribList);
        static PFNWGLCREATECONTEXTATTRIBSARBPROC pfnCreateContextAttribsARB =
            reinterpret_cast<PFNWGLCREATECONTEXTATTRIBSARBPROC>(
                wglGetProcAddress("wglCreateContextAttribsARB"));

        int attriblist[] = {WGL_CONTEXT_MAJOR_VERSION_ARB,
                            4,
                            WGL_CONTEXT_MINOR_VERSION_ARB,
                            3,
                            WGL_CONTEXT_FLAGS_ARB,
                            WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
                            0,
                            0};

        hRC = pfnCreateContextAttribsARB(hDC, 0, attriblist);
        wglMakeCurrent(hDC, hRC);

        wglDeleteContext(temp_context);
#else
        SDL_Init(SDL_INIT_VIDEO);

        window_ =
            SDL_CreateWindow("View", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                             256, 256, SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
        gl_ctx_main_ = SDL_GL_CreateContext(window_);
#endif
        Context::Init(256, 256, &log_);
    }

    ~ProgramTest() {
#if defined(_WIN32)
        wglMakeCurrent(NULL, NULL);
        ReleaseDC(hWnd, hDC);
        wglDeleteContext(hRC);
        DestroyWindow(hWnd);
        UnregisterClass("ProgramTest", hInstance);
#else
        SDL_GL_DeleteContext(gl_ctx_main_);
        SDL_DestroyWindow(window_);
#ifndef EMSCRIPTEN
        SDL_Quit();
#endif
#endif
    }
};

void test_program() {
    {
        // Load glsl program
        ProgramTest test;

        const char vs_src[] =
            R"(
#version 100

/*
ATTRIBUTES
    aVertexPosition : 0
    aVertexPosition1 : 1
UNIFORMS
    uMVPMatrix : 0
*/

attribute vec3 aVertexPosition;
uniform mat4 uMVPMatrix;

void main(void) {
    gl_Position = uMVPMatrix * vec4(aVertexPosition, 1.0);
})";

        const char fs_src[] =
            R"(
#version 100

#ifdef GL_ES
	precision mediump float;
#else
	#define lowp
	#define mediump
	#define highp
#endif
/*
UNIFORMS
    asdasd : 1
*/
uniform vec3 col;

void main(void) {
    gl_FragColor = vec4(col, 1.0);
})";

        Ren::eProgLoadStatus status;
        Ren::ProgramRef p = test.LoadProgram("constant", {}, {}, {}, {}, &status);

        require(status == Ren::eProgLoadStatus::SetToDefault);
        require(p->name() == "constant");
        require(p->id() == 0); // not initialized
        require(p->ready() == false);

        Ren::eShaderLoadStatus sh_status;
        Ren::ShaderRef vs_ref = test.LoadShaderGLSL("constant_vs", vs_src,
                                                    Ren::eShaderType::Vert, &sh_status);
        require(sh_status == Ren::eShaderLoadStatus::CreatedFromData);
        Ren::ShaderRef fs_ref = test.LoadShaderGLSL("constant_fs", fs_src,
                                                    Ren::eShaderType::Frag, &sh_status);
        require(sh_status == Ren::eShaderLoadStatus::CreatedFromData);

        test.LoadProgram("constant", vs_ref, fs_ref, {}, {}, &status);

        require(status == Ren::eProgLoadStatus::CreatedFromData);

        require(p->name() == "constant");
        require(p->ready() == true);

        require(p->attribute(0).name == "aVertexPosition");
        require(p->attribute(0).loc != -1);
        require(p->attribute(1).name.empty());
        require(p->attribute(1).loc == -1);

        require(p->uniform(0).name == "uMVPMatrix");
        require(p->uniform(0).loc != -1);
        require(p->uniform(1).name == "col");
        require(p->uniform(1).loc != -1);
    }

    { // Load compute
        ProgramTest test;

        const char cs_source[] =
            R"(
#version 430

/*
UNIFORMS
    delta : 0
*/

uniform vec4 delta;

struct AttribData {
	vec4 p;
	vec4 c;
};

layout(std430, binding = 0) buffer dest_buffer {
	AttribData data[];
} inout_buffer;

layout (local_size_x = 64, local_size_y = 1, local_size_z = 1) in;

void main() {
	uint global_index = gl_GlobalInvocationID.x;
	uint work_size = gl_WorkGroupSize.x * gl_NumWorkGroups.x;

	inout_buffer.data[global_index].p = inout_buffer.data[global_index].p + delta;
	inout_buffer.data[global_index].c = vec4(1.0, 0.0, 1.0, 1.0);
})";

        Ren::eShaderLoadStatus sh_status;
        Ren::ShaderRef cs_ref = test.LoadShaderGLSL("sample_cs", cs_source,
                                                    Ren::eShaderType::Comp, &sh_status);
        require(sh_status == Ren::eShaderLoadStatus::CreatedFromData);

        Ren::eProgLoadStatus status;
        Ren::ProgramRef p = test.LoadProgram("sample", cs_ref, &status);
        require(status == Ren::eProgLoadStatus::CreatedFromData);

        require(p->uniform(0).name == "delta");
        require(p->uniform(0).loc != -1);

        struct AttribData {
            Ren::Vec4f p, c;
        };

        auto buf = Ren::Buffer{"buf", Ren::eBufferType::VertexAttribs,
                               Ren::eBufferAccessType::Draw,
                               Ren::eBufferAccessFreq::Static, sizeof(AttribData) * 128};

        std::vector<AttribData> _data;
        for (int i = 0; i < 128; i++) {
            _data.push_back({Ren::Vec4f{0.0f, float(i), 0.0f, 0.0f}, Ren::Vec4f{0.0f}});
        }

        uint32_t offset = buf.Alloc(128 * sizeof(AttribData), _data.data());
        require(offset == 0);

        glUseProgram(p->id());
        glUniform4f(p->uniform("delta").loc, 0.0f, -0.1f, 0.0f, 0.0f);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, buf.id());

        glDispatchCompute(2, 1, 1);

        glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);
        glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, 128 * sizeof(AttribData),
                           _data.data());

        for (int i = 0; i < 128; i++) {
            require(_data[i].p[0] == Approx(0.0));
            require(_data[i].p[1] == Approx(double(i) - 0.1f));
            require(_data[i].p[2] == Approx(0.0));
            require(_data[i].p[3] == Approx(0.0));

            require(_data[i].c[0] == Approx(1.0));
            require(_data[i].c[1] == Approx(0.0));
            require(_data[i].c[2] == Approx(1.0));
            require(_data[i].c[3] == Approx(1.0));
        }

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
    }

    { // load spirv
        ProgramTest test;

        if (test.capabilities.gl_spirv) {
            /* Contents of file frag.spv */
            static const int frag_spv_size = 1208;
            static const uint8_t frag_spv[1208] = {
                0x03, 0x02, 0x23, 0x07, 0x00, 0x00, 0x01, 0x00, 0x07, 0x00, 0x08, 0x00,
                0x2C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x00, 0x02, 0x00,
                0x01, 0x00, 0x00, 0x00, 0x0B, 0x00, 0x06, 0x00, 0x01, 0x00, 0x00, 0x00,
                0x47, 0x4C, 0x53, 0x4C, 0x2E, 0x73, 0x74, 0x64, 0x2E, 0x34, 0x35, 0x30,
                0x00, 0x00, 0x00, 0x00, 0x0E, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x01, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x0A, 0x00, 0x04, 0x00, 0x00, 0x00,
                0x04, 0x00, 0x00, 0x00, 0x6D, 0x61, 0x69, 0x6E, 0x00, 0x00, 0x00, 0x00,
                0x09, 0x00, 0x00, 0x00, 0x11, 0x00, 0x00, 0x00, 0x15, 0x00, 0x00, 0x00,
                0x17, 0x00, 0x00, 0x00, 0x2B, 0x00, 0x00, 0x00, 0x10, 0x00, 0x03, 0x00,
                0x04, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x10, 0x00, 0x03, 0x00,
                0x04, 0x00, 0x00, 0x00, 0x0C, 0x00, 0x00, 0x00, 0x03, 0x00, 0x03, 0x00,
                0x02, 0x00, 0x00, 0x00, 0x90, 0x01, 0x00, 0x00, 0x04, 0x00, 0x09, 0x00,
                0x47, 0x4C, 0x5F, 0x41, 0x52, 0x42, 0x5F, 0x73, 0x65, 0x70, 0x61, 0x72,
                0x61, 0x74, 0x65, 0x5F, 0x73, 0x68, 0x61, 0x64, 0x65, 0x72, 0x5F, 0x6F,
                0x62, 0x6A, 0x65, 0x63, 0x74, 0x73, 0x00, 0x00, 0x04, 0x00, 0x09, 0x00,
                0x47, 0x4C, 0x5F, 0x41, 0x52, 0x42, 0x5F, 0x73, 0x68, 0x61, 0x64, 0x69,
                0x6E, 0x67, 0x5F, 0x6C, 0x61, 0x6E, 0x67, 0x75, 0x61, 0x67, 0x65, 0x5F,
                0x34, 0x32, 0x30, 0x70, 0x61, 0x63, 0x6B, 0x00, 0x05, 0x00, 0x04, 0x00,
                0x04, 0x00, 0x00, 0x00, 0x6D, 0x61, 0x69, 0x6E, 0x00, 0x00, 0x00, 0x00,
                0x05, 0x00, 0x05, 0x00, 0x09, 0x00, 0x00, 0x00, 0x75, 0x46, 0x72, 0x61,
                0x67, 0x43, 0x6F, 0x6C, 0x6F, 0x72, 0x00, 0x00, 0x05, 0x00, 0x05, 0x00,
                0x0D, 0x00, 0x00, 0x00, 0x74, 0x65, 0x78, 0x53, 0x61, 0x6D, 0x70, 0x6C,
                0x65, 0x72, 0x00, 0x00, 0x05, 0x00, 0x03, 0x00, 0x11, 0x00, 0x00, 0x00,
                0x75, 0x76, 0x5F, 0x00, 0x05, 0x00, 0x06, 0x00, 0x15, 0x00, 0x00, 0x00,
                0x67, 0x6C, 0x5F, 0x46, 0x72, 0x61, 0x67, 0x44, 0x65, 0x70, 0x74, 0x68,
                0x00, 0x00, 0x00, 0x00, 0x05, 0x00, 0x06, 0x00, 0x17, 0x00, 0x00, 0x00,
                0x67, 0x6C, 0x5F, 0x46, 0x72, 0x61, 0x67, 0x43, 0x6F, 0x6F, 0x72, 0x64,
                0x00, 0x00, 0x00, 0x00, 0x05, 0x00, 0x04, 0x00, 0x2B, 0x00, 0x00, 0x00,
                0x63, 0x6F, 0x6C, 0x5F, 0x00, 0x00, 0x00, 0x00, 0x47, 0x00, 0x04, 0x00,
                0x09, 0x00, 0x00, 0x00, 0x1E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x47, 0x00, 0x04, 0x00, 0x0D, 0x00, 0x00, 0x00, 0x22, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x47, 0x00, 0x04, 0x00, 0x0D, 0x00, 0x00, 0x00,
                0x21, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x47, 0x00, 0x04, 0x00,
                0x11, 0x00, 0x00, 0x00, 0x1E, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00,
                0x47, 0x00, 0x04, 0x00, 0x15, 0x00, 0x00, 0x00, 0x0B, 0x00, 0x00, 0x00,
                0x16, 0x00, 0x00, 0x00, 0x47, 0x00, 0x04, 0x00, 0x17, 0x00, 0x00, 0x00,
                0x0B, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x00, 0x47, 0x00, 0x04, 0x00,
                0x2B, 0x00, 0x00, 0x00, 0x1E, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
                0x13, 0x00, 0x02, 0x00, 0x02, 0x00, 0x00, 0x00, 0x21, 0x00, 0x03, 0x00,
                0x03, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x16, 0x00, 0x03, 0x00,
                0x06, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x17, 0x00, 0x04, 0x00,
                0x07, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00,
                0x20, 0x00, 0x04, 0x00, 0x08, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
                0x07, 0x00, 0x00, 0x00, 0x3B, 0x00, 0x04, 0x00, 0x08, 0x00, 0x00, 0x00,
                0x09, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x19, 0x00, 0x09, 0x00,
                0x0A, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1B, 0x00, 0x03, 0x00,
                0x0B, 0x00, 0x00, 0x00, 0x0A, 0x00, 0x00, 0x00, 0x20, 0x00, 0x04, 0x00,
                0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0B, 0x00, 0x00, 0x00,
                0x3B, 0x00, 0x04, 0x00, 0x0C, 0x00, 0x00, 0x00, 0x0D, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x17, 0x00, 0x04, 0x00, 0x0F, 0x00, 0x00, 0x00,
                0x06, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x20, 0x00, 0x04, 0x00,
                0x10, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x00,
                0x3B, 0x00, 0x04, 0x00, 0x10, 0x00, 0x00, 0x00, 0x11, 0x00, 0x00, 0x00,
                0x01, 0x00, 0x00, 0x00, 0x20, 0x00, 0x04, 0x00, 0x14, 0x00, 0x00, 0x00,
                0x03, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x3B, 0x00, 0x04, 0x00,
                0x14, 0x00, 0x00, 0x00, 0x15, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
                0x20, 0x00, 0x04, 0x00, 0x16, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
                0x07, 0x00, 0x00, 0x00, 0x3B, 0x00, 0x04, 0x00, 0x16, 0x00, 0x00, 0x00,
                0x17, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x15, 0x00, 0x04, 0x00,
                0x18, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x2B, 0x00, 0x04, 0x00, 0x18, 0x00, 0x00, 0x00, 0x19, 0x00, 0x00, 0x00,
                0x02, 0x00, 0x00, 0x00, 0x20, 0x00, 0x04, 0x00, 0x1A, 0x00, 0x00, 0x00,
                0x01, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x2B, 0x00, 0x04, 0x00,
                0x18, 0x00, 0x00, 0x00, 0x1D, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
                0x2B, 0x00, 0x04, 0x00, 0x06, 0x00, 0x00, 0x00, 0x23, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x14, 0x00, 0x02, 0x00, 0x24, 0x00, 0x00, 0x00,
                0x2B, 0x00, 0x04, 0x00, 0x06, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x80, 0x3F, 0x2B, 0x00, 0x04, 0x00, 0x18, 0x00, 0x00, 0x00,
                0x29, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3B, 0x00, 0x04, 0x00,
                0x16, 0x00, 0x00, 0x00, 0x2B, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
                0x36, 0x00, 0x05, 0x00, 0x02, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0xF8, 0x00, 0x02, 0x00,
                0x05, 0x00, 0x00, 0x00, 0x3D, 0x00, 0x04, 0x00, 0x0B, 0x00, 0x00, 0x00,
                0x0E, 0x00, 0x00, 0x00, 0x0D, 0x00, 0x00, 0x00, 0x3D, 0x00, 0x04, 0x00,
                0x0F, 0x00, 0x00, 0x00, 0x12, 0x00, 0x00, 0x00, 0x11, 0x00, 0x00, 0x00,
                0x57, 0x00, 0x05, 0x00, 0x07, 0x00, 0x00, 0x00, 0x13, 0x00, 0x00, 0x00,
                0x0E, 0x00, 0x00, 0x00, 0x12, 0x00, 0x00, 0x00, 0x3E, 0x00, 0x03, 0x00,
                0x09, 0x00, 0x00, 0x00, 0x13, 0x00, 0x00, 0x00, 0x41, 0x00, 0x05, 0x00,
                0x1A, 0x00, 0x00, 0x00, 0x1B, 0x00, 0x00, 0x00, 0x17, 0x00, 0x00, 0x00,
                0x19, 0x00, 0x00, 0x00, 0x3D, 0x00, 0x04, 0x00, 0x06, 0x00, 0x00, 0x00,
                0x1C, 0x00, 0x00, 0x00, 0x1B, 0x00, 0x00, 0x00, 0x41, 0x00, 0x05, 0x00,
                0x1A, 0x00, 0x00, 0x00, 0x1E, 0x00, 0x00, 0x00, 0x17, 0x00, 0x00, 0x00,
                0x1D, 0x00, 0x00, 0x00, 0x3D, 0x00, 0x04, 0x00, 0x06, 0x00, 0x00, 0x00,
                0x1F, 0x00, 0x00, 0x00, 0x1E, 0x00, 0x00, 0x00, 0x88, 0x00, 0x05, 0x00,
                0x06, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x1C, 0x00, 0x00, 0x00,
                0x1F, 0x00, 0x00, 0x00, 0x3E, 0x00, 0x03, 0x00, 0x15, 0x00, 0x00, 0x00,
                0x20, 0x00, 0x00, 0x00, 0x41, 0x00, 0x05, 0x00, 0x1A, 0x00, 0x00, 0x00,
                0x21, 0x00, 0x00, 0x00, 0x17, 0x00, 0x00, 0x00, 0x19, 0x00, 0x00, 0x00,
                0x3D, 0x00, 0x04, 0x00, 0x06, 0x00, 0x00, 0x00, 0x22, 0x00, 0x00, 0x00,
                0x21, 0x00, 0x00, 0x00, 0xB4, 0x00, 0x05, 0x00, 0x24, 0x00, 0x00, 0x00,
                0x25, 0x00, 0x00, 0x00, 0x22, 0x00, 0x00, 0x00, 0x23, 0x00, 0x00, 0x00,
                0xF7, 0x00, 0x03, 0x00, 0x27, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0xFA, 0x00, 0x04, 0x00, 0x25, 0x00, 0x00, 0x00, 0x26, 0x00, 0x00, 0x00,
                0x27, 0x00, 0x00, 0x00, 0xF8, 0x00, 0x02, 0x00, 0x26, 0x00, 0x00, 0x00,
                0x41, 0x00, 0x05, 0x00, 0x14, 0x00, 0x00, 0x00, 0x2A, 0x00, 0x00, 0x00,
                0x09, 0x00, 0x00, 0x00, 0x29, 0x00, 0x00, 0x00, 0x3E, 0x00, 0x03, 0x00,
                0x2A, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0xF9, 0x00, 0x02, 0x00,
                0x27, 0x00, 0x00, 0x00, 0xF8, 0x00, 0x02, 0x00, 0x27, 0x00, 0x00, 0x00,
                0xFD, 0x00, 0x01, 0x00, 0x38, 0x00, 0x01, 0x00};

            /* Contents of file vert.spv */
            static const int vert_spv_size = 1752;
            static const uint8_t vert_spv[1752] = {
                0x03, 0x02, 0x23, 0x07, 0x00, 0x00, 0x01, 0x00, 0x07, 0x00, 0x08, 0x00,
                0x35, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x00, 0x02, 0x00,
                0x01, 0x00, 0x00, 0x00, 0x0B, 0x00, 0x06, 0x00, 0x01, 0x00, 0x00, 0x00,
                0x47, 0x4C, 0x53, 0x4C, 0x2E, 0x73, 0x74, 0x64, 0x2E, 0x34, 0x35, 0x30,
                0x00, 0x00, 0x00, 0x00, 0x0E, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x01, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x0B, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x04, 0x00, 0x00, 0x00, 0x6D, 0x61, 0x69, 0x6E, 0x00, 0x00, 0x00, 0x00,
                0x0D, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x2C, 0x00, 0x00, 0x00,
                0x2D, 0x00, 0x00, 0x00, 0x31, 0x00, 0x00, 0x00, 0x33, 0x00, 0x00, 0x00,
                0x03, 0x00, 0x03, 0x00, 0x02, 0x00, 0x00, 0x00, 0x90, 0x01, 0x00, 0x00,
                0x04, 0x00, 0x09, 0x00, 0x47, 0x4C, 0x5F, 0x41, 0x52, 0x42, 0x5F, 0x73,
                0x65, 0x70, 0x61, 0x72, 0x61, 0x74, 0x65, 0x5F, 0x73, 0x68, 0x61, 0x64,
                0x65, 0x72, 0x5F, 0x6F, 0x62, 0x6A, 0x65, 0x63, 0x74, 0x73, 0x00, 0x00,
                0x04, 0x00, 0x09, 0x00, 0x47, 0x4C, 0x5F, 0x41, 0x52, 0x42, 0x5F, 0x73,
                0x68, 0x61, 0x64, 0x69, 0x6E, 0x67, 0x5F, 0x6C, 0x61, 0x6E, 0x67, 0x75,
                0x61, 0x67, 0x65, 0x5F, 0x34, 0x32, 0x30, 0x70, 0x61, 0x63, 0x6B, 0x00,
                0x05, 0x00, 0x04, 0x00, 0x04, 0x00, 0x00, 0x00, 0x6D, 0x61, 0x69, 0x6E,
                0x00, 0x00, 0x00, 0x00, 0x05, 0x00, 0x06, 0x00, 0x0B, 0x00, 0x00, 0x00,
                0x67, 0x6C, 0x5F, 0x50, 0x65, 0x72, 0x56, 0x65, 0x72, 0x74, 0x65, 0x78,
                0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x06, 0x00, 0x0B, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x67, 0x6C, 0x5F, 0x50, 0x6F, 0x73, 0x69, 0x74,
                0x69, 0x6F, 0x6E, 0x00, 0x06, 0x00, 0x07, 0x00, 0x0B, 0x00, 0x00, 0x00,
                0x01, 0x00, 0x00, 0x00, 0x67, 0x6C, 0x5F, 0x50, 0x6F, 0x69, 0x6E, 0x74,
                0x53, 0x69, 0x7A, 0x65, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x07, 0x00,
                0x0B, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x67, 0x6C, 0x5F, 0x43,
                0x6C, 0x69, 0x70, 0x44, 0x69, 0x73, 0x74, 0x61, 0x6E, 0x63, 0x65, 0x00,
                0x05, 0x00, 0x03, 0x00, 0x0D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x05, 0x00, 0x03, 0x00, 0x11, 0x00, 0x00, 0x00, 0x55, 0x42, 0x4F, 0x00,
                0x06, 0x00, 0x05, 0x00, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x6D, 0x6F, 0x64, 0x6C, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x05, 0x00,
                0x11, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x76, 0x69, 0x65, 0x77,
                0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x05, 0x00, 0x11, 0x00, 0x00, 0x00,
                0x02, 0x00, 0x00, 0x00, 0x70, 0x72, 0x6F, 0x6A, 0x00, 0x00, 0x00, 0x00,
                0x05, 0x00, 0x03, 0x00, 0x13, 0x00, 0x00, 0x00, 0x75, 0x62, 0x6F, 0x00,
                0x05, 0x00, 0x03, 0x00, 0x20, 0x00, 0x00, 0x00, 0x70, 0x6F, 0x73, 0x00,
                0x05, 0x00, 0x04, 0x00, 0x2C, 0x00, 0x00, 0x00, 0x63, 0x6F, 0x6C, 0x5F,
                0x00, 0x00, 0x00, 0x00, 0x05, 0x00, 0x03, 0x00, 0x2D, 0x00, 0x00, 0x00,
                0x63, 0x6F, 0x6C, 0x00, 0x05, 0x00, 0x03, 0x00, 0x31, 0x00, 0x00, 0x00,
                0x75, 0x76, 0x5F, 0x00, 0x05, 0x00, 0x03, 0x00, 0x33, 0x00, 0x00, 0x00,
                0x75, 0x76, 0x00, 0x00, 0x48, 0x00, 0x05, 0x00, 0x0B, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x0B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x48, 0x00, 0x05, 0x00, 0x0B, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
                0x0B, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x48, 0x00, 0x05, 0x00,
                0x0B, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x0B, 0x00, 0x00, 0x00,
                0x03, 0x00, 0x00, 0x00, 0x47, 0x00, 0x03, 0x00, 0x0B, 0x00, 0x00, 0x00,
                0x02, 0x00, 0x00, 0x00, 0x48, 0x00, 0x04, 0x00, 0x11, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x48, 0x00, 0x05, 0x00,
                0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x23, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x48, 0x00, 0x05, 0x00, 0x11, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00,
                0x48, 0x00, 0x04, 0x00, 0x11, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
                0x05, 0x00, 0x00, 0x00, 0x48, 0x00, 0x05, 0x00, 0x11, 0x00, 0x00, 0x00,
                0x01, 0x00, 0x00, 0x00, 0x23, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00,
                0x48, 0x00, 0x05, 0x00, 0x11, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
                0x07, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x48, 0x00, 0x04, 0x00,
                0x11, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00,
                0x48, 0x00, 0x05, 0x00, 0x11, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
                0x23, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x48, 0x00, 0x05, 0x00,
                0x11, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00,
                0x10, 0x00, 0x00, 0x00, 0x47, 0x00, 0x03, 0x00, 0x11, 0x00, 0x00, 0x00,
                0x02, 0x00, 0x00, 0x00, 0x47, 0x00, 0x04, 0x00, 0x13, 0x00, 0x00, 0x00,
                0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x47, 0x00, 0x04, 0x00,
                0x13, 0x00, 0x00, 0x00, 0x21, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x47, 0x00, 0x04, 0x00, 0x20, 0x00, 0x00, 0x00, 0x1E, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x47, 0x00, 0x04, 0x00, 0x2C, 0x00, 0x00, 0x00,
                0x1E, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x47, 0x00, 0x04, 0x00,
                0x2D, 0x00, 0x00, 0x00, 0x1E, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
                0x47, 0x00, 0x04, 0x00, 0x31, 0x00, 0x00, 0x00, 0x1E, 0x00, 0x00, 0x00,
                0x04, 0x00, 0x00, 0x00, 0x47, 0x00, 0x04, 0x00, 0x33, 0x00, 0x00, 0x00,
                0x1E, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x13, 0x00, 0x02, 0x00,
                0x02, 0x00, 0x00, 0x00, 0x21, 0x00, 0x03, 0x00, 0x03, 0x00, 0x00, 0x00,
                0x02, 0x00, 0x00, 0x00, 0x16, 0x00, 0x03, 0x00, 0x06, 0x00, 0x00, 0x00,
                0x20, 0x00, 0x00, 0x00, 0x17, 0x00, 0x04, 0x00, 0x07, 0x00, 0x00, 0x00,
                0x06, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x15, 0x00, 0x04, 0x00,
                0x08, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x2B, 0x00, 0x04, 0x00, 0x08, 0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00,
                0x01, 0x00, 0x00, 0x00, 0x1C, 0x00, 0x04, 0x00, 0x0A, 0x00, 0x00, 0x00,
                0x06, 0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00, 0x1E, 0x00, 0x05, 0x00,
                0x0B, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00,
                0x0A, 0x00, 0x00, 0x00, 0x20, 0x00, 0x04, 0x00, 0x0C, 0x00, 0x00, 0x00,
                0x03, 0x00, 0x00, 0x00, 0x0B, 0x00, 0x00, 0x00, 0x3B, 0x00, 0x04, 0x00,
                0x0C, 0x00, 0x00, 0x00, 0x0D, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
                0x15, 0x00, 0x04, 0x00, 0x0E, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00,
                0x01, 0x00, 0x00, 0x00, 0x2B, 0x00, 0x04, 0x00, 0x0E, 0x00, 0x00, 0x00,
                0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x00, 0x04, 0x00,
                0x10, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00,
                0x1E, 0x00, 0x05, 0x00, 0x11, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00,
                0x10, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x20, 0x00, 0x04, 0x00,
                0x12, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x11, 0x00, 0x00, 0x00,
                0x3B, 0x00, 0x04, 0x00, 0x12, 0x00, 0x00, 0x00, 0x13, 0x00, 0x00, 0x00,
                0x02, 0x00, 0x00, 0x00, 0x2B, 0x00, 0x04, 0x00, 0x0E, 0x00, 0x00, 0x00,
                0x14, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x20, 0x00, 0x04, 0x00,
                0x15, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00,
                0x2B, 0x00, 0x04, 0x00, 0x0E, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00,
                0x01, 0x00, 0x00, 0x00, 0x20, 0x00, 0x04, 0x00, 0x1F, 0x00, 0x00, 0x00,
                0x01, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x3B, 0x00, 0x04, 0x00,
                0x1F, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
                0x20, 0x00, 0x04, 0x00, 0x23, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
                0x07, 0x00, 0x00, 0x00, 0x2B, 0x00, 0x04, 0x00, 0x06, 0x00, 0x00, 0x00,
                0x25, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3F, 0x2B, 0x00, 0x04, 0x00,
                0x08, 0x00, 0x00, 0x00, 0x26, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
                0x20, 0x00, 0x04, 0x00, 0x27, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
                0x06, 0x00, 0x00, 0x00, 0x2B, 0x00, 0x04, 0x00, 0x06, 0x00, 0x00, 0x00,
                0x29, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x3F, 0x2B, 0x00, 0x04, 0x00,
                0x08, 0x00, 0x00, 0x00, 0x2A, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
                0x3B, 0x00, 0x04, 0x00, 0x23, 0x00, 0x00, 0x00, 0x2C, 0x00, 0x00, 0x00,
                0x03, 0x00, 0x00, 0x00, 0x3B, 0x00, 0x04, 0x00, 0x1F, 0x00, 0x00, 0x00,
                0x2D, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x17, 0x00, 0x04, 0x00,
                0x2F, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
                0x20, 0x00, 0x04, 0x00, 0x30, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
                0x2F, 0x00, 0x00, 0x00, 0x3B, 0x00, 0x04, 0x00, 0x30, 0x00, 0x00, 0x00,
                0x31, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x20, 0x00, 0x04, 0x00,
                0x32, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x2F, 0x00, 0x00, 0x00,
                0x3B, 0x00, 0x04, 0x00, 0x32, 0x00, 0x00, 0x00, 0x33, 0x00, 0x00, 0x00,
                0x01, 0x00, 0x00, 0x00, 0x36, 0x00, 0x05, 0x00, 0x02, 0x00, 0x00, 0x00,
                0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
                0xF8, 0x00, 0x02, 0x00, 0x05, 0x00, 0x00, 0x00, 0x41, 0x00, 0x05, 0x00,
                0x15, 0x00, 0x00, 0x00, 0x16, 0x00, 0x00, 0x00, 0x13, 0x00, 0x00, 0x00,
                0x14, 0x00, 0x00, 0x00, 0x3D, 0x00, 0x04, 0x00, 0x10, 0x00, 0x00, 0x00,
                0x17, 0x00, 0x00, 0x00, 0x16, 0x00, 0x00, 0x00, 0x41, 0x00, 0x05, 0x00,
                0x15, 0x00, 0x00, 0x00, 0x19, 0x00, 0x00, 0x00, 0x13, 0x00, 0x00, 0x00,
                0x18, 0x00, 0x00, 0x00, 0x3D, 0x00, 0x04, 0x00, 0x10, 0x00, 0x00, 0x00,
                0x1A, 0x00, 0x00, 0x00, 0x19, 0x00, 0x00, 0x00, 0x92, 0x00, 0x05, 0x00,
                0x10, 0x00, 0x00, 0x00, 0x1B, 0x00, 0x00, 0x00, 0x17, 0x00, 0x00, 0x00,
                0x1A, 0x00, 0x00, 0x00, 0x41, 0x00, 0x05, 0x00, 0x15, 0x00, 0x00, 0x00,
                0x1C, 0x00, 0x00, 0x00, 0x13, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x00,
                0x3D, 0x00, 0x04, 0x00, 0x10, 0x00, 0x00, 0x00, 0x1D, 0x00, 0x00, 0x00,
                0x1C, 0x00, 0x00, 0x00, 0x92, 0x00, 0x05, 0x00, 0x10, 0x00, 0x00, 0x00,
                0x1E, 0x00, 0x00, 0x00, 0x1B, 0x00, 0x00, 0x00, 0x1D, 0x00, 0x00, 0x00,
                0x3D, 0x00, 0x04, 0x00, 0x07, 0x00, 0x00, 0x00, 0x21, 0x00, 0x00, 0x00,
                0x20, 0x00, 0x00, 0x00, 0x91, 0x00, 0x05, 0x00, 0x07, 0x00, 0x00, 0x00,
                0x22, 0x00, 0x00, 0x00, 0x1E, 0x00, 0x00, 0x00, 0x21, 0x00, 0x00, 0x00,
                0x41, 0x00, 0x05, 0x00, 0x23, 0x00, 0x00, 0x00, 0x24, 0x00, 0x00, 0x00,
                0x0D, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x00, 0x3E, 0x00, 0x03, 0x00,
                0x24, 0x00, 0x00, 0x00, 0x22, 0x00, 0x00, 0x00, 0x41, 0x00, 0x06, 0x00,
                0x27, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x0D, 0x00, 0x00, 0x00,
                0x0F, 0x00, 0x00, 0x00, 0x26, 0x00, 0x00, 0x00, 0x3E, 0x00, 0x03, 0x00,
                0x28, 0x00, 0x00, 0x00, 0x25, 0x00, 0x00, 0x00, 0x41, 0x00, 0x06, 0x00,
                0x27, 0x00, 0x00, 0x00, 0x2B, 0x00, 0x00, 0x00, 0x0D, 0x00, 0x00, 0x00,
                0x0F, 0x00, 0x00, 0x00, 0x2A, 0x00, 0x00, 0x00, 0x3E, 0x00, 0x03, 0x00,
                0x2B, 0x00, 0x00, 0x00, 0x29, 0x00, 0x00, 0x00, 0x3D, 0x00, 0x04, 0x00,
                0x07, 0x00, 0x00, 0x00, 0x2E, 0x00, 0x00, 0x00, 0x2D, 0x00, 0x00, 0x00,
                0x3E, 0x00, 0x03, 0x00, 0x2C, 0x00, 0x00, 0x00, 0x2E, 0x00, 0x00, 0x00,
                0x3D, 0x00, 0x04, 0x00, 0x2F, 0x00, 0x00, 0x00, 0x34, 0x00, 0x00, 0x00,
                0x33, 0x00, 0x00, 0x00, 0x3E, 0x00, 0x03, 0x00, 0x31, 0x00, 0x00, 0x00,
                0x34, 0x00, 0x00, 0x00, 0xFD, 0x00, 0x01, 0x00, 0x38, 0x00, 0x01, 0x00};

            Ren::eShaderLoadStatus sh_status;
            Ren::ShaderRef vs_ref = test.LoadShaderSPIRV(
                "simple_vs", vert_spv, vert_spv_size, Ren::eShaderType::Vert, &sh_status);
            require(sh_status == Ren::eShaderLoadStatus::CreatedFromData);
            Ren::ShaderRef fs_ref = test.LoadShaderSPIRV(
                "simple_fs", frag_spv, frag_spv_size, Ren::eShaderType::Frag, &sh_status);
            require(sh_status == Ren::eShaderLoadStatus::CreatedFromData);

            Ren::eProgLoadStatus status;
            Ren::ProgramRef p =
                test.LoadProgram("simple", vs_ref, fs_ref, {}, {}, &status);
            require(status == Ren::eProgLoadStatus::CreatedFromData);

            require(p->name() == "simple");
            require(p->ready() == true);

            // require(p->attribute(0).loc == 0);
            // require(p->attribute(1).loc == 1);
            // require(p->attribute(2).loc == 2);
            // require(p->attribute(3).loc == -1);

            // require(p->uniform(0).loc == 0);
            // require(p->uniform(1).loc == -1);
        } else {
            printf("Could not test spirv loading!\n");
        }
    }
}
