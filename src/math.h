#pragma once

#include <cmath>

namespace organic
{
    struct Vec3
    {
        float x;
        float y;
        float z;

        Vec3 operator-(const Vec3& other)
        {
            return {
                this->x - other.x,
                this->y - other.y,
                this->z - other.z,
            };
        }
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

    static float lengthSquared(Vec3 a)
    {
        return powf(a.x, 2) + powf(a.y, 2) + powf(a.z, 2);
    }

    static float length(Vec3 a)
    {
        return sqrtf(lengthSquared(a));
    }

    static float distance(Vec3 a, Vec3 b)
    {
        return length(b - a);
    }
}
