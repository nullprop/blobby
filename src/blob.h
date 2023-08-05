#pragma once

#include "math.h"

namespace organic
{
    class Blob
    {
      public:
        Blob(float x, float y, float z, float radius)
        {
            Blob({x, y, z}, radius);
        }

        Blob(Vec3 position, float radius)
        {
            Position = position;
            Radius = radius;
        }

        struct Vec3 Position;
        float Radius;

        // for std::list::remove
        bool operator==(const Blob& other) const
        {
            return this == &other;
        }
    };
}
