#pragma once

#include <list>
#include <vector>

#include "blob.h"
#include "context.h"

#define CHUNK_SIZE 16
#define MAX_HEIGHT 128
#define BLOBS_IN_CHUNK (MAX_HEIGHT * CHUNK_SIZE * CHUNK_SIZE)

namespace blobby
{
    struct Chunk
    {
        int chunkX;
        int chunkZ;

        Blob blobs[BLOBS_IN_CHUNK];
        Blob lod1Blobs[BLOBS_IN_CHUNK / 8];    // 2x2x2
        Blob lod2Blobs[BLOBS_IN_CHUNK / 64];   // 4x4x4
        Blob lod3Blobs[BLOBS_IN_CHUNK / 512];  // 8x8x8
        Blob lod4Blobs[BLOBS_IN_CHUNK / 4096]; // 16x16x16

        Chunk(int x, int z)
        {
            this->chunkX = x;
            this->chunkZ = z;
        }

        Vec3 Center()
        {
            return {(chunkX + 0.5f) * CHUNK_SIZE, 0.5f * MAX_HEIGHT, (chunkX + 0.5f) * CHUNK_SIZE};
        }

        // Distance from point p to the closest point inside this chunk
        float Distance(Vec3 p)
        {
            Vec3 c = Center();
            Vec3 d = c - p;
            float halfWidth = CHUNK_SIZE * 0.5f;
            float halfHeight = MAX_HEIGHT * 0.5f;

            // distance on each axis to surface of chunk
            if (fabs(d.x) < halfWidth)
                d.x = 0;
            else if (d.x < 0)
                d.x += halfWidth;
            else
                d.x -= halfWidth;

            if (fabs(d.y) < halfHeight)
                d.y = 0;
            else if (d.y < 0)
                d.y += halfHeight;
            else
                d.y -= halfHeight;

            if (fabs(d.z) < halfWidth)
                d.z = 0;
            else if (d.z < 0)
                d.z += halfWidth;
            else
                d.z -= halfWidth;

            return length(d);
        }

        // for std::list::remove
        bool operator==(const Chunk& other) const
        {
            return this == &other;
        }
    };

    class Terrain
    {
      public:
        Terrain(int seed);
        ~Terrain();

        void Update(Context* context, float projMatrix[16]);
        std::vector<Blob*> GetBlobsToRender();

        Blob* GetBlob(int blobX, int blobY, int blobZ);
        Chunk* GetChunk(int chunkX, int chunkZ);

        void Save();

      private:
        void LoadChunk(int chunkX, int chunkZ);
        void UnloadChunk(int chunkX, int chunkZ);
        void UnloadChunk(Chunk chunk);

        Chunk* LoadSavedChunk(int chunkX, int chunkZ);
        void SaveChunk(Chunk chunk);

        Chunk GenerateChunk(int chunkX, int chunkZ);
        void GenerateBlob(int blobX, int blobY, int blobZ, Blob* blob);

        void DownsampleChunk(Chunk* chunk);

        int m_seed;
        std::vector<Chunk> m_chunks;
        std::vector<Blob*> m_visibleBlobs;
    };
}