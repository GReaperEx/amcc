#ifndef C_CHUNK_MANAGER_H
#define C_CHUNK_MANAGER_H

#include "CChunk.h"
#include "CBlockInfo.h"

#include "CTextureManager.h"

class CChunkManager
{
public:
    CChunkManager() {}
    ~CChunkManager() {
        for (CChunk *curChunk : chunks) {
            delete curChunk;
        }
    }

    void init(CTextureManager& textureManager, const CNoiseGenerator& noiseGen) {
        this->noiseGen = noiseGen;
        blockAtlas = textureManager.getTexture("assets/blockAtlas.png");

        // These are for testing
        {
            glm::vec2 texCoords[6][4] = {
                { glm::vec2(0, 0.5f), glm::vec2(0.5f, 0.5f), glm::vec2(0.5f, 1.0f), glm::vec2(0, 1.0f) },
                { glm::vec2(0, 0.5f), glm::vec2(0.5f, 0.5f), glm::vec2(0.5f, 1.0f), glm::vec2(0, 1.0f) },
                { glm::vec2(0, 0.5f), glm::vec2(0.5f, 0.5f), glm::vec2(0.5f, 1.0f), glm::vec2(0, 1.0f) },
                { glm::vec2(0, 0.5f), glm::vec2(0.5f, 0.5f), glm::vec2(0.5f, 1.0f), glm::vec2(0, 1.0f) },
                { glm::vec2(0, 0.5f), glm::vec2(0.5f, 0.5f), glm::vec2(0.5f, 1.0f), glm::vec2(0, 1.0f) },
                { glm::vec2(0, 0.5f), glm::vec2(0.5f, 0.5f), glm::vec2(0.5f, 1.0f), glm::vec2(0, 1.0f) },
            };
            blockInfo.addBlock("stone", 1, texCoords);
        }
        {
            glm::vec2 texCoords[6][4] = {
                { glm::vec2(0.5f, 0.5f), glm::vec2(1.0f, 0.5f), glm::vec2(1.0f, 1.0f), glm::vec2(0.5f, 1.0f) },
                { glm::vec2(0.5f, 0.5f), glm::vec2(1.0f, 0.5f), glm::vec2(1.0f, 1.0f), glm::vec2(0.5f, 1.0f) },
                { glm::vec2(0.5f, 0.5f), glm::vec2(1.0f, 0.5f), glm::vec2(1.0f, 1.0f), glm::vec2(0.5f, 1.0f) },
                { glm::vec2(0.5f, 0.5f), glm::vec2(1.0f, 0.5f), glm::vec2(1.0f, 1.0f), glm::vec2(0.5f, 1.0f) },
                { glm::vec2(0.5f, 0.5f), glm::vec2(1.0f, 0.5f), glm::vec2(1.0f, 1.0f), glm::vec2(0.5f, 1.0f) },
                { glm::vec2(0.5f, 0.5f), glm::vec2(1.0f, 0.5f), glm::vec2(1.0f, 1.0f), glm::vec2(0.5f, 1.0f) },
            };
            blockInfo.addBlock("dirt", 2, texCoords);
        }
        {
            glm::vec2 texCoords[6][4] = {
                { glm::vec2(0, 0.0f), glm::vec2(0.5f, 0.0f), glm::vec2(0.5f, 0.5f), glm::vec2(0, 0.5f) },
                { glm::vec2(0, 0.0f), glm::vec2(0.5f, 0.0f), glm::vec2(0.5f, 0.5f), glm::vec2(0, 0.5f) },
                { glm::vec2(0, 0.0f), glm::vec2(0.5f, 0.0f), glm::vec2(0.5f, 0.5f), glm::vec2(0, 0.5f) },
                { glm::vec2(0, 0.0f), glm::vec2(0.5f, 0.0f), glm::vec2(0.5f, 0.5f), glm::vec2(0, 0.5f) },
                { glm::vec2(0, 0.0f), glm::vec2(0.5f, 0.0f), glm::vec2(0.5f, 0.5f), glm::vec2(0, 0.5f) },
                { glm::vec2(0, 0.0f), glm::vec2(0.5f, 0.0f), glm::vec2(0.5f, 0.5f), glm::vec2(0, 0.5f) },
            };
            blockInfo.addBlock("grass", 3, texCoords);
        }

        CChunk *adjacent[6] = { nullptr };
        for (int i = -8; i <= 8; ++i) {
            for (int j = -8; j <= 8; ++j) {
                CChunk* newChunk = new CChunk(glm::vec3(i*16, 0, j*16));

                chunksToGenerate.push_back(newChunk);
                chunks.push_back(newChunk);
            }
        }
    }

    void renderChunks(CShaderManager& shaderManager, const glm::mat4& vp) {
        shaderManager.use("default");
        blockAtlas->use();

        // Just for testing, generate/update a chunk per frame

        if (!chunksToGenerate.empty()) {
            CChunk* chunk = chunksToGenerate.back();
            chunksToGenerate.pop_back();

            CChunk* adjacent[6] = { nullptr };
            chunk->genBlocks(blockInfo, noiseGen, adjacent);
            chunksToUpdateMesh.push_back(chunk);
        }
        if (!chunksToUpdateMesh.empty()) {
            CChunk* chunk = chunksToUpdateMesh.back();
            chunksToUpdateMesh.pop_back();

            // TODO: Detect adjacent chunks to generate the mesh properly
            CChunk* adjacent[6] = { nullptr };
            chunk->genMesh(blockInfo, adjacent);
            chunksToUpdateState.push_back(chunk);
        }
        if (!chunksToUpdateState.empty()) {
            CChunk* chunk = chunksToUpdateState.back();
            chunksToUpdateState.pop_back();

            chunk->updateOpenGLState();
            chunksToRender.push_back(chunk);
        }

        for (CChunk* curChunk : chunksToRender) {
            glm::mat4 mvp = vp*glm::translate(glm::mat4(1), curChunk->getPosition());
            glUniformMatrix4fv(shaderManager.getUniformLocation("MVP"), 1, GL_FALSE, &(mvp[0][0]));

            curChunk->render();
        }
    }

private:
    // -1 = "Unlimited"
    static const int CHUNKS_FOR_X = -1;
    static const int CHUNKS_FOR_Y =  1;
    static const int CHUNKS_FOR_Z = -1;

    // Naive and temporary solution
    std::vector<CChunk*> chunks;

    std::vector<CChunk*> chunksToRender;
    std::vector<CChunk*> chunksToGenerate;
    std::vector<CChunk*> chunksToUpdateMesh;
    std::vector<CChunk*> chunksToUpdateState;

    CTexture *blockAtlas;
    CBlockInfo blockInfo;
    CNoiseGenerator noiseGen;
};

#endif // C_CHUNK_MANAGER_H
