#include <bgfx/bgfx.h>
#include <bgfx/platform.h>
#include <bx/math.h>
#include <bx/timer.h>

#include <SDL_syswm.h>

#include <cmath>
#include <imgui.h>

#include "bgfx-imgui/imgui_impl_bgfx.h"
#include "file-ops.h"
#include "sdl-imgui/imgui_impl_sdl2.h"

#include "engine.h"
#include "math.h"
#include "renderer.h"

namespace organic
{
    Engine::Engine()
    {
        const int width = 1280;
        const int height = 720;

        if (SDL_Init(SDL_INIT_VIDEO) < 0)
        {
            printf("SDL could not initialize. SDL_Error: %s\n", SDL_GetError());
            throw("Failed to create Engine");
        }

        SDL_Window* window = SDL_CreateWindow("organic", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width,
                                              height, SDL_WINDOW_SHOWN);

        if (window == nullptr)
        {
            printf("Window could not be created. SDL_Error: %s\n", SDL_GetError());
            throw("Failed to create Engine");
        }

        SDL_SysWMinfo wmi;
        SDL_VERSION(&wmi.version);
        if (!SDL_GetWindowWMInfo(window, &wmi))
        {
            printf("SDL_SysWMinfo could not be retrieved. SDL_Error: %s\n", SDL_GetError());
            throw("Failed to create Engine");
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

        m_context = Context();
        m_context.width = width;
        m_context.height = height;
        m_context.window = window;
        m_context.renderer = new Renderer();

        if (m_context.renderer->IsValid())
        {
            while (!m_context.quit)
            {
                Loop();
            }
        }
    }

    Engine::~Engine()
    {
        delete m_context.renderer;

        ImGui_ImplSDL2_Shutdown();
        ImGui_Implbgfx_Shutdown();

        ImGui::DestroyContext();
        bgfx::shutdown();

        SDL_DestroyWindow(m_context.window);
        SDL_Quit();
    }

    void Engine::Loop()
    {
        int64_t now = bx::getHPCounter();
        static int64_t last = now;
        const int64_t frameTime = now - last;
        last = now;
        const double freq = double(bx::getHPFrequency());
        m_context.deltaTime = float(frameTime / freq);
        m_context.time += m_context.timeScale * m_context.deltaTime;

        for (SDL_Event current_event; SDL_PollEvent(&current_event) != 0;)
        {
            ImGui_ImplSDL2_ProcessEvent(&current_event);
            if (current_event.type == SDL_QUIT)
            {
                m_context.quit = true;
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
                int delta_x = mouse_x - m_context.prev_mouse_x;
                int delta_y = mouse_y - m_context.prev_mouse_y;
                m_context.cam_yaw += float(-delta_x) * m_context.rot_scale;
                m_context.cam_pitch += float(-delta_y) * m_context.rot_scale;
            }
            m_context.prev_mouse_x = mouse_x;
            m_context.prev_mouse_y = mouse_y;
        }

        m_context.renderer->Loop(&m_context);
    }
}