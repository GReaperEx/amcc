#ifndef CHUNK_MANAGER_H
#define CHUNK_MANAGER_H

#include "Chunk.h"
#include "BlockManager.h"

#include "TextureManager.h"
#include "BoxOutline.h"
#include "Camera.h"
#include "ChunkTree.h"
#include "BiomeManager.h"
#include "StructureManager.h"

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

    void addLightSource(const glm::vec3& pos, int intensity);
    void remLightSource(const glm::vec3& pos);

private:
    void findAdjacentChunks(const Chunk& center, Chunk *adjacent[6]);
    void generateStructure(const Structure& genStruct, const glm::vec3& pos);

    int getLightLevel(const glm::vec3& pos) {
        glm::vec3 temp((float)Chunk::CHUNK_WIDTH, (float)Chunk::CHUNK_HEIGHT, (float)Chunk::CHUNK_DEPTH);
        glm::vec3 chunkPos = glm::floor(pos/temp)*temp;

        Chunk* fetchedChunk = chunkTree.getChunk(chunkPos, ChunkTree::ALL);
        if (fetchedChunk == nullptr) {
            chunkTree.addChunk(chunkPos);
            fetchedChunk = chunkTree.getChunk(chunkPos, ChunkTree::ALL);
        }

        Chunk::SBlock block = fetchedChunk->getBlock(pos);
        if (blockManager.getBlock(block.id).isTransparent()) {
            return block.meta & 0x0F;
        }
        return -1;
    }

    void setLightLevel(const glm::vec3& pos, int newLevel) {
        glm::vec3 temp((float)Chunk::CHUNK_WIDTH, (float)Chunk::CHUNK_HEIGHT, (float)Chunk::CHUNK_DEPTH);
        glm::vec3 chunkPos = glm::floor(pos/temp)*temp;

        Chunk* fetchedChunk = chunkTree.getChunk(chunkPos, ChunkTree::ALL);
        if (fetchedChunk == nullptr) {
            chunkTree.addChunk(chunkPos);
            fetchedChunk = chunkTree.getChunk(chunkPos, ChunkTree::ALL);
        }

        Chunk::SBlock block = fetchedChunk->getBlock(pos);
        if (blockManager.getBlock(block.id).isTransparent()) {
            block.meta = (block.meta & 0xFF00) | (int)glm::clamp(newLevel, 0, 15);
            fetchedChunk->setBlock(pos, block);
            fetchedChunk->forceUpdate();
        }
    }

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
    StructureManager structManager;
    BiomeManager biomeManager;
    std::vector<NoiseGenerator> noiseGens;
    BoxOutline boxOutline;
    Camera* camera;
};

#endif // CHUNK_MANAGER_H
