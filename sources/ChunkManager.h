#ifndef CHUNK_MANAGER_H
#define CHUNK_MANAGER_H

#include "Chunk.h"
#include "BlockManager.h"

#include "TextureManager.h"
#include "BoxOutline.h"
#include "Camera.h"
#include "ChunkTree.h"
#include "BiomeManager.h"

#include <set>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>

class ChunkManager
{
public:
    // Setting ultimate limits for generation
    static const int MIN_CHUNK_X = INT_MIN;
    static const int MAX_CHUNK_X = INT_MAX;
    static const int MIN_CHUNK_Z = INT_MIN;
    static const int MAX_CHUNK_Z = INT_MAX;
    static const int MIN_CHUNK_Y = 0;
    static const int MAX_CHUNK_Y = Chunk::CHUNK_HEIGHT;

public:
    ChunkManager() {
        blockAtlas = nullptr;
        keepRunning = true;
        userRequest = false;
    }

    ~ChunkManager() {
        if (blockAtlas) {
            blockAtlas->drop();
        }

        keepRunning = false;
        if (chunkGenThread.joinable()) {
            chunkGenThread.join();
        }
        if (meshUpdateThread.joinable()) {
            meshUpdateThread.join();
        }
        if (initAndFreeThread.joinable()) {
            initAndFreeThread.join();
        }
    }

    void init(TextureManager& textureManager, const std::vector<NoiseGenerator>& noiseGens, Camera* camera);
    void renderChunks(ShaderManager& shaderManager, const glm::mat4& vp);

    void replaceBlock(const Chunk::BlockDetails& newBlock);
    bool traceRayToBlock(Chunk::BlockDetails& lookBlock, const glm::vec3& rayOrigin, const glm::vec3& rayDir,
                         bool ignoreAir = true);

    void renderOutline(ShaderManager& shaderManager, const glm::mat4& vp) {
        Chunk::BlockDetails lookBlock;
        if (traceRayToBlock(lookBlock, camera->getPosition(), camera->getLookVector())) {
            boxOutline.render(shaderManager, vp, lookBlock.position);
        }
    }

private:
    void findAdjacentChunks(const Chunk& center, Chunk *adjacent[6]);

    ChunkTree chunkTree;

    std::thread chunkGenThread;
    void genThreadFunc();
    std::thread meshUpdateThread;
    void updateThreadFunc();
    std::thread initAndFreeThread;
    void initfreeThreadFunc();
    std::atomic_bool keepRunning;
    std::atomic_bool userRequest;

    Texture *blockAtlas;
    BlockManager blockManager;
    BiomeManager biomeManager;
    std::vector<NoiseGenerator> noiseGens;
    BoxOutline boxOutline;
    Camera* camera;
};

#endif // CHUNK_MANAGER_H
