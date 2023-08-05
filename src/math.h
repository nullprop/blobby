#pragma once

namespace organic
{
    struct Vec3
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

    static float clamp(float f, float min, float max)
    {
        if (f < min)
            return min;
        if (f > max)
            return max;
        return f;
    }
}
