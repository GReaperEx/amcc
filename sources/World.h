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

class World
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
    World() {
        blockAtlas = nullptr;
        keepRunning = true;
        userRequest = false;
    }

    ~World() {
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

    void init(TextureManager& g_TextureManager, const std::vector<NoiseGenerator>& noiseGens, Camera* camera);
    void renderChunks(ShaderManager& g_ShaderManager, const glm::mat4& vp);

    void replaceBlock(const Chunk::BlockDetails& newBlock);
    bool traceRayToBlock(Chunk::BlockDetails& lookBlock, const glm::vec3& rayOrigin, const glm::vec3& rayDir,
                         bool ignoreAir = true);

    void renderOutline(ShaderManager& g_ShaderManager, const glm::mat4& vp) {
        Chunk::BlockDetails lookBlock;
        if (traceRayToBlock(lookBlock, camera->getPosition(), camera->getLookVector())) {
            boxOutline.render(g_ShaderManager, vp, lookBlock.position);
        }
    }

    void addLightSource(const glm::vec3& pos, int intensity, bool sunlight = false);
    void remLightSource(const glm::vec3& pos, bool sunlight = false);

    void changeSunlight(int intensity);

private:
    void findAdjacentChunks(const Chunk& center, Chunk *adjacent[6]);
    void generateStructure(const Structure& genStruct, const glm::vec3& pos);

    int getLightLevel(const glm::vec3& pos, bool sunlight) {
        glm::vec3 temp((float)Chunk::CHUNK_WIDTH, (float)Chunk::CHUNK_HEIGHT, (float)Chunk::CHUNK_DEPTH);
        glm::vec3 chunkPos = glm::floor(pos/temp)*temp;

        Chunk* fetchedChunk = chunkTree.getChunk(chunkPos, ChunkTree::ALL);
        if (fetchedChunk == nullptr) {
            chunkTree.addChunk(chunkPos);
            fetchedChunk = chunkTree.getChunk(chunkPos, ChunkTree::ALL);
        }

        Chunk::SBlock block = fetchedChunk->getBlock(pos);
        if (g_BlockManager.getBlock(block.id).isTransparent()) {
            if (sunlight) {
                return (block.meta >> 4) & 0x0F;
            }
            return block.meta & 0x0F;
        }
        return -1;
    }

    void setLightLevel(const glm::vec3& pos, int newLevel, bool sunlight) {
        glm::vec3 temp((float)Chunk::CHUNK_WIDTH, (float)Chunk::CHUNK_HEIGHT, (float)Chunk::CHUNK_DEPTH);
        glm::vec3 chunkPos = glm::floor(pos/temp)*temp;

        Chunk* fetchedChunk = chunkTree.getChunk(chunkPos, ChunkTree::ALL);
        if (fetchedChunk == nullptr) {
            chunkTree.addChunk(chunkPos);
            fetchedChunk = chunkTree.getChunk(chunkPos, ChunkTree::ALL);
        }

        Chunk::SBlock block = fetchedChunk->getBlock(pos);
        if (g_BlockManager.getBlock(block.id).isTransparent()) {
            if (sunlight) {
                block.meta = (block.meta & 0xFF0F) | ((int)glm::clamp(newLevel, 0, 15) << 4);
            } else {
                block.meta = (block.meta & 0xFFF0) | (int)glm::clamp(newLevel, 0, 15);
            }
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
    BiomeManager biomeManager;
    std::vector<NoiseGenerator> noiseGens;
    BoxOutline boxOutline;
    Camera* camera;
};

#endif // CHUNK_MANAGER_H
