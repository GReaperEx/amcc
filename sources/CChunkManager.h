#ifndef C_CHUNK_MANAGER_H
#define C_CHUNK_MANAGER_H

#include "CChunk.h"
#include "CBlockInfo.h"

#include "CTextureManager.h"
#include "CBoxOutline.h"
#include "CCamera.h"

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
        for (CChunk *curChunk : chunks) {
            delete curChunk;
        }
        if (blockAtlas) {
            blockAtlas->drop();
        }

        chunkGenThread.detach();
        meshUpdateThread.detach();
    }

    void init(CTextureManager& textureManager, const CNoiseGenerator& noiseGen);
    void renderChunks(CShaderManager& shaderManager, const glm::mat4& vp);

    void replaceBlock(const CChunk::BlockDetails& newBlock);
    bool traceRayToBlock(CChunk::BlockDetails& lookBlock, const glm::vec3& rayOrigin, const glm::vec3& rayDir,
                         bool ignoreAir = true);

    void renderOutline(CShaderManager& shaderManager, const glm::mat4& vp, const glm::vec3& cameraPos, const glm::vec3& cameraLook) {
        CChunk::BlockDetails lookBlock;
        if (traceRayToBlock(lookBlock, cameraPos, cameraLook)) {
            boxOutline.render(shaderManager, vp, lookBlock.position);
        }
    }

private:
    void findAdjacentChunks(const CChunk& center, CChunk *adjacent[6]);

    // -1 = "Unlimited"
    static const int CHUNKS_FOR_X = -1;
    static const int CHUNKS_FOR_Y =  1;
    static const int CHUNKS_FOR_Z = -1;

    // Naive and temporary solution
    std::set<CChunk*> chunks;
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
};

#endif // C_CHUNK_MANAGER_H
