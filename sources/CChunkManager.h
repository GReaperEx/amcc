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
    }

    ~CChunkManager() {
        if (blockAtlas) {
            blockAtlas->drop();
        }

        chunkGenThread.detach();
        meshUpdateThread.detach();
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
    std::vector<CChunk*> chunks;
    std::mutex chunksBeingUsed;
    std::condition_variable usageEvent;
    std::atomic_int usageCount;

    std::thread chunkGenThread;
    void genThreadFunc();
    std::thread meshUpdateThread;
    void updateThreadFunc();

    CTexture *blockAtlas;
    CBlockInfo blockInfo;
    CNoiseGenerator noiseGen;
    CBoxOutline boxOutline;
    CCamera* camera;
};

#endif // C_CHUNK_MANAGER_H
