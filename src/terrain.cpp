#include <cstdio>

#include "blob.h"
#include "constants.h"
#include "terrain.h"

namespace organic
{
    Terrain::Terrain(int seed)
    {
        printf("Terrain seed: %d\n", seed);
        m_seed = seed;
        m_chunks = std::vector<Chunk>();
        m_visibleBlobs = std::vector<Blob*>();

        // TEST
        LoadChunk(0, 0);
    }

    Terrain::~Terrain()
    {
        Save();
    }

    Blob* Terrain::GetBlob(int blobX, int blobY, int blobZ)
    {
        Blob* blob = nullptr;
        int chunkX = blobX / CHUNK_SIZE;
        int chunkZ = blobZ / CHUNK_SIZE;
        Chunk* chunk = GetChunk(chunkX, chunkZ);
        if (chunk)
        {
            int index = (CHUNK_SIZE * CHUNK_SIZE * blobY) + (CHUNK_SIZE * (blobZ % CHUNK_SIZE)) + blobX % CHUNK_SIZE;
            return &chunk->blobs[index];
        }

        return blob;
    }

    Chunk* Terrain::GetChunk(int chunkX, int chunkZ)
    {
        for (int i = 0; i < m_chunks.size(); i++)
        {
            if (m_chunks[i].chunkX == chunkX && m_chunks[i].chunkZ == chunkZ)
            {
                return &m_chunks[i];
            }
        }
        return nullptr;
    }

    void Terrain::Save()
    {
        for (Chunk chunk : m_chunks)
        {
            SaveChunk(chunk);
        }
    }

    void Terrain::Update(Context* context, float projMatrix[16])
    {
        // TODO
        // frustrum cull chunks
        // load visible chunks
        // unload hidden chunks

        m_visibleBlobs.clear();

        for (std::vector<Chunk>::iterator chunk = m_chunks.begin(); chunk != m_chunks.end();)
        {
            for (int j = 0; j < BLOBS_IN_CHUNK; j++)
            {
                // Distance cull
                if (distance(chunk->blobs[j].Position, context->camPosition) > FAR_PLANE + chunk->blobs[j].Radius)
                    continue;

                // TODO: frustrum cull
                // TODO: occlusion cull

                m_visibleBlobs.push_back(&chunk->blobs[j]);
            }
            chunk++;
        }
    }

    std::vector<Blob*> Terrain::GetBlobsToRender()
    {
        return m_visibleBlobs;
    }

    void Terrain::LoadChunk(int chunkX, int chunkZ)
    {
        for (Chunk chunk : m_chunks)
        {
            if (chunk.chunkX == chunkX && chunk.chunkZ == chunkZ)
            {
                return;
            }
        }

        Chunk* savedChunk = LoadSavedChunk(chunkX, chunkZ);

        if (savedChunk)
        {
            m_chunks.push_back(*savedChunk);
            delete savedChunk;
        }
        else
        {
            m_chunks.push_back(GenerateChunk(chunkX, chunkZ));
        }
    }

    void Terrain::UnloadChunk(int chunkX, int chunkZ)
    {
        for (std::vector<Chunk>::iterator chunk = m_chunks.begin(); chunk != m_chunks.end();)
        {
            if (chunk->chunkX == chunkX && chunk->chunkZ == chunkZ)
            {
                SaveChunk(*chunk);
                m_chunks.erase(chunk);
                break;
            }
            chunk++;
        }
    }

    Chunk* Terrain::LoadSavedChunk(int chunkX, int chunkZ)
    {
        Chunk* chunk = nullptr;
        // TODO
        return chunk;
    }

    void Terrain::SaveChunk(Chunk chunk)
    {
        // TODO
    }

    Chunk Terrain::GenerateChunk(int chunkX, int chunkZ)
    {
        Chunk chunk = Chunk(chunkX, chunkZ);

        for (int y = 0; y < MAX_HEIGHT; y++)
        {
            for (int z = 0; z < CHUNK_SIZE; z++)
            {
                for (int x = 0; x < CHUNK_SIZE; x++)
                {
                    int index = (CHUNK_SIZE * CHUNK_SIZE * y) + (CHUNK_SIZE * z) + x;
                    int blobX = chunkX * CHUNK_SIZE + x;
                    int blobY = y;
                    int blobZ = chunkZ * CHUNK_SIZE + z;
                    GenerateBlob(blobX, blobY, blobZ, &chunk.blobs[index]);
                }
            }
        }

        return chunk;
    }

    void Terrain::GenerateBlob(int blobX, int blobY, int blobZ, Blob* blob)
    {
        blob->Position.x = blobX;
        blob->Position.y = blobY;
        blob->Position.z = blobZ;
        blob->Radius = 0.5f;
        blob->Type = BlobType::DEFAULT;
    }
}