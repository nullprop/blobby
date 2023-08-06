#include <bgfx/bgfx.h>
#include <bgfx/platform.h>
#include <bx/math.h>
#include <bx/timer.h>

#include <SDL_events.h>
#include <SDL_mouse.h>
#include <SDL_scancode.h>
#include <SDL_stdinc.h>
#include <SDL_syswm.h>
#include <SDL_video.h>

#include <cmath>
#include <imgui.h>

#include "bgfx-imgui/imgui_impl_bgfx.h"
#include "engine.h"
#include "file-ops.h"
#include "math.h"
#include "renderer.h"
#include "sdl-imgui/imgui_impl_sdl2.h"
#include "terrain.h"

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
                                              height, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

        if (window == nullptr)
        {
            printf("Window could not be created. SDL_Error: %s\n", SDL_GetError());
            throw("Failed to create Engine");
        }

        SDL_SetRelativeMouseMode(SDL_TRUE);

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
        m_context.terrain = new Terrain(0);

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
        delete m_context.terrain;
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

            if (current_event.type == SDL_KEYDOWN)
            {
                switch (current_event.key.keysym.scancode)
                {
                case SDL_SCANCODE_ESCAPE:
                    m_context.quit = true;
                    break;

                case SDL_SCANCODE_W:
                    m_context.keyFlags |= Keys::FORWARD;
                    break;

                case SDL_SCANCODE_S:
                    m_context.keyFlags |= Keys::BACK;
                    break;

                case SDL_SCANCODE_A:
                    m_context.keyFlags |= Keys::LEFT;
                    break;

                case SDL_SCANCODE_D:
                    m_context.keyFlags |= Keys::RIGHT;
                    break;

                case SDL_SCANCODE_SPACE:
                    m_context.keyFlags |= Keys::UP;
                    break;

                case SDL_SCANCODE_LCTRL:
                case SDL_SCANCODE_LSHIFT:
                    m_context.keyFlags |= Keys::DOWN;
                    break;

                default:
                    break;
                }

                if (m_context.quit)
                    break;
            }

            if (current_event.type == SDL_KEYUP)
            {
                switch (current_event.key.keysym.scancode)
                {
                case SDL_SCANCODE_W:
                    m_context.keyFlags &= ~Keys::FORWARD;
                    break;

                case SDL_SCANCODE_S:
                    m_context.keyFlags &= ~Keys::BACK;
                    break;

                case SDL_SCANCODE_A:
                    m_context.keyFlags &= ~Keys::LEFT;
                    break;

                case SDL_SCANCODE_D:
                    m_context.keyFlags &= ~Keys::RIGHT;
                    break;

                case SDL_SCANCODE_SPACE:
                    m_context.keyFlags &= ~Keys::UP;
                    break;

                case SDL_SCANCODE_LCTRL:
                case SDL_SCANCODE_LSHIFT:
                    m_context.keyFlags &= ~Keys::DOWN;
                    break;

                default:
                    break;
                }
            }

            if (current_event.type == SDL_MOUSEMOTION)
            {
                m_context.camYaw -= float(current_event.motion.xrel) * m_context.sensitivity * 0.022f;
                m_context.camYaw = fmodf(m_context.camYaw, 360.0f);

                m_context.camPitch -= float(current_event.motion.yrel) * m_context.sensitivity * 0.022f;
                m_context.camPitch = clamp(m_context.camPitch, -89.0f, 89.0f);
            }
        }

        bx::Vec3 wishMove = {0, 0, 0};
        if ((m_context.keyFlags & Keys::FORWARD) != 0)
            wishMove.z += m_context.deltaTime * m_context.camSpeed;
        if ((m_context.keyFlags & Keys::BACK) != 0)
            wishMove.z -= m_context.deltaTime * m_context.camSpeed;
        if ((m_context.keyFlags & Keys::RIGHT) != 0)
            wishMove.x += m_context.deltaTime * m_context.camSpeed;
        if ((m_context.keyFlags & Keys::LEFT) != 0)
            wishMove.x -= m_context.deltaTime * m_context.camSpeed;
        if ((m_context.keyFlags & Keys::UP) != 0)
            wishMove.y += m_context.deltaTime * m_context.camSpeed;
        if ((m_context.keyFlags & Keys::DOWN) != 0)
            wishMove.y -= m_context.deltaTime * m_context.camSpeed;

        float camRotation[16];
        float camTranslation[16];
        bx::mtxRotateXYZ(camRotation, bx::toRad(m_context.camPitch), bx::toRad(m_context.camYaw), 0.0f);
        wishMove = bx::mul(wishMove, camRotation);
        m_context.camPosition.x += wishMove.x;
        m_context.camPosition.y += wishMove.y;
        m_context.camPosition.z += wishMove.z;
        bx::mtxTranslate(camTranslation, m_context.camPosition.x, m_context.camPosition.y, m_context.camPosition.z);
        bx::mtxMul(m_context.camTransform, camRotation, camTranslation);

        m_context.renderer->Loop(&m_context);
    }
}