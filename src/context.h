#pragma once

#include <SDL.h>

#include "math.h"

namespace organic
{
    class Renderer;

    enum Keys
    {
        FORWARD = 0x00000001,
        BACK = 0x00000010,
        RIGHT = 0x00000100,
        LEFT = 0x00001000,
        UP = 0x00010000,
        DOWN = 0x00100000,
    };

    struct Context
    {
        SDL_Window* window = nullptr;
        Renderer* renderer = nullptr;

        float camPitch = 0.0f;
        float camYaw = 0.0f;
        float sensitivity = 1.4f;
        Vec3 camPosition = {0.0f, 0.0f, -8.0f};
        float camSpeed = 5.0f;
        int keyFlags;

        float camTransform[16];

        int width = 0;
        int height = 0;

        bool quit = false;

        float time = 0.0f;
        float timeScale = 1.0f;
        float deltaTime = 0.0f;
    };
}