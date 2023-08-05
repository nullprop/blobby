#pragma once

#include <SDL.h>

namespace organic
{
    class Renderer;

    struct Context
    {
        SDL_Window* window = nullptr;
        Renderer* renderer = nullptr;

        float cam_pitch = 0.0f;
        float cam_yaw = 0.0f;
        float rot_scale = 0.01f;

        int prev_mouse_x = 0;
        int prev_mouse_y = 0;

        int width = 0;
        int height = 0;

        bool quit = false;

        float time = 0.0f;
        float timeScale = 1.0f;
        float deltaTime = 0.0f;
    };
}