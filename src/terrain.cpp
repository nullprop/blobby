#include <cstdio>

#include "blob.h"
#include "constants.h"
#include "terrain.h"

namespace blobby
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
            // Select LOD based on distance
            int lodLevel = 0;
            float chunkDistance = chunk->Distance(context->camPosition);

            if (chunkDistance > FAR_PLANE * 0.8f)
                lodLevel = 4;
            else if (chunkDistance > FAR_PLANE * 0.6f)
                lodLevel = 3;
            else if (chunkDistance > FAR_PLANE * 0.4f)
                lodLevel = 2;
            else if (chunkDistance > FAR_PLANE * 0.2f)
                lodLevel = 1;

            int blobCount = BLOBS_IN_CHUNK / pow(8, lodLevel);
            Blob* blobArrays[] = {
                chunk->blobs, chunk->lod1Blobs, chunk->lod2Blobs, chunk->lod3Blobs, chunk->lod4Blobs,
            };
            Blob* blobs = blobArrays[lodLevel];

            // Check and add blobs from selected LOD
            for (int j = 0; j < blobCount; j++)
            {
                // Distance cull
                if (distance(blobs[j].Position, context->camPosition) > FAR_PLANE + blobs[j].Radius)
                    continue;

                // Cull empty air
                if (blobs[j].Type == BlobType::AIR)
                    continue;

                // TODO: frustrum cull blobs? might be slower with already frustrum culled chunks
                // TODO: occlusion cull blobs
                // TODO: pre-calc pixel bounds for blobs and
                // only check blobs near current pixel in fragment shader

                m_visibleBlobs.push_back(&blobs[j]);
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
            DownsampleChunk(savedChunk);
            m_chunks.push_back(*savedChunk);
            delete savedChunk;
        }
        else
        {
            Chunk chunk = GenerateChunk(chunkX, chunkZ);
            DownsampleChunk(&chunk);
            m_chunks.push_back(chunk);
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
        blob->Type = blobY > 0 ? BlobType::AIR : BlobType::GROUND;
    }

    void Terrain::DownsampleChunk(Chunk* chunk)
    {
        // TODO perf: when editing,
        // we shouldn't have to update the whole chunk.
        // just update the affected areas.

        int width = CHUNK_SIZE;
        int height = MAX_HEIGHT;
        float blobRadius = 1.0f;
        Blob* blobs[] = {
            chunk->blobs, chunk->lod1Blobs, chunk->lod2Blobs, chunk->lod3Blobs, chunk->lod4Blobs,
        };

        for (int level = 0; level < 4; level++)
        {
            Blob* sourceLod = blobs[level];
            Blob* targetLod = blobs[level + 1];

            for (int y = 0; y < height; y += 2)
            {
                for (int z = 0; z < width; z += 2)
                {
                    for (int x = 0; x < width; x += 2)
                    {
                        int sourceIndex = (width * width * y) + (width * z) + x;
                        int targetIndex = (width * width * y / 8) + (width * z / 4) + x / 2;
                        int sourceTypeCount[BlobType::MAX];

                        for (int yD = 0; yD < 2; yD++)
                        {
                            for (int zD = 0; zD < 2; zD++)
                            {
                                for (int xD = 0; xD < 2; xD++)
                                {
                                    int index = sourceIndex + (width * width * yD) + (width * zD) + xD;
                                    Blob sourceBlob = sourceLod[index];
                                    sourceTypeCount[sourceBlob.Type]++;
                                }
                            }
                        }

                        BlobType mostCommonType = BlobType::GROUND;
                        for (int t = BlobType::GROUND + 1; t < BlobType::MAX; t++)
                        {
                            if (sourceTypeCount[t] > sourceTypeCount[mostCommonType])
                            {
                                mostCommonType = (BlobType)t;
                            }
                        }

                        targetLod[targetIndex].Radius = blobRadius;
                        targetLod[targetIndex].Type = mostCommonType;
                        targetLod[targetIndex].Position.x = sourceLod[sourceIndex].Position.x + blobRadius * 0.5f;
                        targetLod[targetIndex].Position.y = sourceLod[sourceIndex].Position.y + blobRadius * 0.5f;
                        targetLod[targetIndex].Position.z = sourceLod[sourceIndex].Position.z + blobRadius * 0.5f;
                    }
                }
            }

            width /= 2;
            height /= 2;
            blobRadius *= 2;
        }
    }
}