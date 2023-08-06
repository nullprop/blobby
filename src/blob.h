#pragma once

#include "math.h"

namespace organic
{
    enum BlobType
    {
        DEFAULT
    };

    class Blob
    {
      public:
        Blob()
        {
        }

        Blob(float x, float y, float z, float radius, BlobType type)
        {
            Blob({x, y, z}, radius, type);
        }

        Blob(Vec3 position, float radius, BlobType type)
        {
            Position = position;
            Radius = radius;
            Type = type;
        }

        struct Vec3 Position;
        float Radius;
        BlobType Type;

        // for std::list::remove
        bool operator==(const Blob& other) const
        {
            return this == &other;
        }
    };
}
