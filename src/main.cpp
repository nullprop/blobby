#include <bgfx/bgfx.h>
#include <bgfx/platform.h>
#include <bx/math.h>
#include <bx/timer.h>

#include <SDL.h>
#include <SDL_syswm.h>

#include <cmath>
#include <imgui.h>

#include "bgfx-imgui/imgui_impl_bgfx.h"
#include "file-ops.h"
#include "sdl-imgui/imgui_impl_sdl2.h"

struct Vertex
{
    float x;
    float y;
    float z;
};

struct Vec4
{
    float x;
    float y;
    float z;
    float w;
};

static Vertex screen_vertices[] = {
    {-1.0f, -1.0f, 0.0f}, // tl
    {1.0f, -1.0f, 0.0f},  // tr
    {1.0f, 1.0f, 0.0f},   // br
    {-1.0f, 1.0f, 0.0f},  // bl
};
static const uint16_t screen_tri_list[] = {0, 1, 2, 0, 2, 3};
// static const uint16_t screen_tri_list[] = {0, 1, 2, 3, 0}; // interleaved

static float m_time = 0.0f;
static float m_timeScale = 1.0f;

static bgfx::ShaderHandle create_shader(const std::string& shader, const char* name)
{
    const bgfx::Memory* mem = bgfx::copy(shader.data(), shader.size());
    const bgfx::ShaderHandle handle = bgfx::createShader(mem);
    bgfx::setName(handle, name);
    return handle;
}

struct context_t
{
    SDL_Window* window = nullptr;
    bgfx::ProgramHandle program = BGFX_INVALID_HANDLE;
    bgfx::VertexBufferHandle vbh = BGFX_INVALID_HANDLE;
    bgfx::IndexBufferHandle ibh = BGFX_INVALID_HANDLE;

    bgfx::UniformHandle u_globals = BGFX_INVALID_HANDLE;
    bgfx::UniformHandle u_positions = BGFX_INVALID_HANDLE;

    float cam_pitch = 0.0f;
    float cam_yaw = 0.0f;
    float rot_scale = 0.01f;

    int prev_mouse_x = 0;
    int prev_mouse_y = 0;

    int width = 0;
    int height = 0;

    bool quit = false;
};

void main_loop(void* data)
{
    auto context = static_cast<context_t*>(data);

    int64_t now = bx::getHPCounter();
    static int64_t last = now;
    const int64_t frameTime = now - last;
    last = now;
    const double freq = double(bx::getHPFrequency());
    const float deltaTime = float(frameTime / freq);
    m_time += m_timeScale * deltaTime;

    float globals[4] = {m_time, 0, 0, 0};
    bgfx::setUniform(context->u_globals, globals, 1);

    Vec4 positions[16] = {};
    for (int i = 0; i < 16; i++)
    {
        positions[i] = {
            ((float)i - 8.0f),
            sin(i * m_time * 0.1f),
            0,
            0
            };
    }
    bgfx::setUniform(context->u_positions, positions, 16);

    for (SDL_Event current_event; SDL_PollEvent(&current_event) != 0;)
    {
        ImGui_ImplSDL2_ProcessEvent(&current_event);
        if (current_event.type == SDL_QUIT)
        {
            context->quit = true;
            break;
        }
    }

    ImGui_Implbgfx_NewFrame();
    ImGui_ImplSDL2_NewFrame();

    ImGui::NewFrame();
    ImGui::ShowDemoWindow(); // your drawing here
    ImGui::Render();
    ImGui_Implbgfx_RenderDrawLists(ImGui::GetDrawData());

    if (!ImGui::GetIO().WantCaptureMouse)
    {
        // simple input code for orbit camera
        int mouse_x, mouse_y;
        const int buttons = SDL_GetMouseState(&mouse_x, &mouse_y);
        if ((buttons & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0)
        {
            int delta_x = mouse_x - context->prev_mouse_x;
            int delta_y = mouse_y - context->prev_mouse_y;
            context->cam_yaw += float(-delta_x) * context->rot_scale;
            context->cam_pitch += float(-delta_y) * context->rot_scale;
        }
        context->prev_mouse_x = mouse_x;
        context->prev_mouse_y = mouse_y;
    }

    float cam_rotation[16];
    bx::mtxRotateXYZ(cam_rotation, context->cam_pitch, context->cam_yaw, 0.0f);

    float cam_translation[16];
    bx::mtxTranslate(cam_translation, 0.0f, 0.0f, -8.0f);

    float cam_transform[16];
    bx::mtxMul(cam_transform, cam_translation, cam_rotation);

    float view[16];
    bx::mtxInverse(view, cam_transform);

    float proj[16];
    bx::mtxProj(proj, 60.0f, float(context->width) / float(context->height), 0.1f, 100.0f,
                bgfx::getCaps()->homogeneousDepth);

    bgfx::setViewTransform(0, view, proj);

    float model[16];
    bx::mtxIdentity(model);
    bgfx::setTransform(model);

    bgfx::setVertexBuffer(0, context->vbh);
    bgfx::setIndexBuffer(context->ibh);

    bgfx::submit(0, context->program);

    bgfx::frame();
}

int main(int argc, char** argv)
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("SDL could not initialize. SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    const int width = 1280;
    const int height = 720;
    SDL_Window* window =
        SDL_CreateWindow(argv[0], SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN);

    if (window == nullptr)
    {
        printf("Window could not be created. SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_SysWMinfo wmi;
    SDL_VERSION(&wmi.version);
    if (!SDL_GetWindowWMInfo(window, &wmi))
    {
        printf("SDL_SysWMinfo could not be retrieved. SDL_Error: %s\n", SDL_GetError());
        return 1;
    }
    bgfx::renderFrame(); // single threaded mode

    bgfx::PlatformData pd{};
#if BX_PLATFORM_WINDOWS
    pd.nwh = wmi.info.win.window;
#elif BX_PLATFORM_OSX
    pd.nwh = wmi.info.cocoa.window;
#elif BX_PLATFORM_LINUX
    pd.ndt = wmi.info.x11.display;
    pd.nwh = (void*)(uintptr_t)wmi.info.x11.window;
#endif

    bgfx::Init bgfx_init;
    // bgfx_init.type = bgfx::RendererType::Count; // auto choose renderer
    bgfx_init.type = bgfx::RendererType::OpenGL;
    bgfx_init.resolution.width = width;
    bgfx_init.resolution.height = height;
    bgfx_init.resolution.reset = BGFX_RESET_VSYNC;
    bgfx_init.platformData = pd;
    bgfx::init(bgfx_init);

    bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x6495EDFF, 1.0f, 0);
    bgfx::setViewRect(0, 0, 0, width, height);

    ImGui::CreateContext();

    ImGui_Implbgfx_Init(255);

    // TODO: use type from bgfx::RenderType?
#if BX_PLATFORM_WINDOWS
    ImGui_ImplSDL2_InitForD3D(window);
#elif BX_PLATFORM_OSX
    ImGui_ImplSDL2_InitForMetal(window);
#elif BX_PLATFORM_LINUX
    // ImGui_ImplSDL2_InitForOpenGL(window, nullptr);
    ImGui_ImplSDL2_InitForVulkan(window);
#endif

    bgfx::VertexLayout pos_col_vert_layout;
    pos_col_vert_layout.begin().add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float).end();
    bgfx::VertexBufferHandle vbh =
        bgfx::createVertexBuffer(bgfx::makeRef(screen_vertices, sizeof(screen_vertices)), pos_col_vert_layout);
    bgfx::IndexBufferHandle ibh = bgfx::createIndexBuffer(bgfx::makeRef(screen_tri_list, sizeof(screen_tri_list)));

    const std::string shader_root = "shader/build/";

    std::string vshader;
    if (!fileops::read_file(shader_root + "v_simple.bin", vshader))
    {
        printf("Could not find shader vertex shader (ensure shaders have been "
               "compiled).\n"
               "Run compile-shaders-<platform>.sh/bat\n");
        return 1;
    }

    std::string fshader;
    if (!fileops::read_file(shader_root + "f_simple.bin", fshader))
    {
        printf("Could not find shader fragment shader (ensure shaders have "
               "been compiled).\n"
               "Run compile-shaders-<platform>.sh/bat\n");
        return 1;
    }

    bgfx::ShaderHandle vsh = create_shader(vshader, "vshader");
    bgfx::ShaderHandle fsh = create_shader(fshader, "fshader");
    bgfx::ProgramHandle program = bgfx::createProgram(vsh, fsh, true);

    context_t context;
    context.width = width;
    context.height = height;
    context.program = program;
    context.window = window;
    context.vbh = vbh;
    context.ibh = ibh;

    context.u_globals = bgfx::createUniform("u_globals", bgfx::UniformType::Vec4, 1);
    context.u_positions = bgfx::createUniform("u_positions", bgfx::UniformType::Vec4, 16);

    while (!context.quit)
    {
        main_loop(&context);
    }

    bgfx::destroy(context.u_positions);
    bgfx::destroy(context.u_globals);

    bgfx::destroy(vbh);
    bgfx::destroy(ibh);
    bgfx::destroy(program);

    ImGui_ImplSDL2_Shutdown();
    ImGui_Implbgfx_Shutdown();

    ImGui::DestroyContext();
    bgfx::shutdown();

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
