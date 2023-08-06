#pragma once

#include <list>
#include <vector>

#include "blob.h"
#include "context.h"

#define CHUNK_SIZE 2
#define MAX_HEIGHT 1 // TODO: 128
#define BLOBS_IN_CHUNK (MAX_HEIGHT * CHUNK_SIZE * CHUNK_SIZE)

namespace organic
{
    struct Chunk
    {
        int chunkX;
        int chunkZ;
        Blob blobs[BLOBS_IN_CHUNK];

        Chunk(int x, int z)
        {
            this->chunkX = x;
            this->chunkZ = z;
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

        int m_seed;
        std::vector<Chunk> m_chunks;
        std::vector<Blob*> m_visibleBlobs;
    };
}