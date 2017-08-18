#ifndef C_CHUNK_MANAGER_H
#define C_CHUNK_MANAGER_H

#include "CChunk.h"
#include "CBlockInfo.h"

#include "CTextureManager.h"
#include "CBoxOutline.h"
#include "CCamera.h"
#include "CChunkTree.h"

#include <set>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>

class CChunkManager
{
public:
    CChunkManager() {
        blockAtlas = nullptr;
        keepRunning = true;
        userRequest = false;
    }

    ~CChunkManager() {
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

    void init(CTextureManager& textureManager, const CNoiseGenerator& noiseGen, CCamera* camera);
    void renderChunks(CShaderManager& shaderManager, const glm::mat4& vp);

    void replaceBlock(const CChunk::BlockDetails& newBlock);
    bool traceRayToBlock(CChunk::BlockDetails& lookBlock, const glm::vec3& rayOrigin, const glm::vec3& rayDir,
                         bool ignoreAir = true);

    void renderOutline(CShaderManager& shaderManager, const glm::mat4& vp) {
        CChunk::BlockDetails lookBlock;
        if (traceRayToBlock(lookBlock, camera->getPosition(), camera->getLookVector())) {
            boxOutline.render(shaderManager, vp, lookBlock.position);
        }
    }

private:
    void findAdjacentChunks(const CChunk& center, CChunk *adjacent[6]);
    void loadBlockInfo(CTextureManager& textureManager);

    CChunkTree chunkTree;

    std::thread chunkGenThread;
    void genThreadFunc();
    std::thread meshUpdateThread;
    void updateThreadFunc();
    std::thread initAndFreeThread;
    void initfreeThreadFunc();
    std::atomic_bool keepRunning;
    std::atomic_bool userRequest;

    CTexture *blockAtlas;
    CBlockInfo blockInfo;
    CNoiseGenerator noiseGen;
    CBoxOutline boxOutline;
    CCamera* camera;
};

#endif // C_CHUNK_MANAGER_H
