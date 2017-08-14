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
        for (int i = -1; i <= 1; ++i) {
            for (int j = -1; j <= 1; ++j) {
                CChunk* newChunk = new CChunk;

                newChunk->genBlocks(blockInfo, noiseGen, adjacent, glm::vec3(i*16, 0, j*16));
                newChunk->genMesh(blockInfo, adjacent);
                newChunk->updateOpenGLState();

                chunks.push_back(newChunk);
            }
        }
    }

    void renderChunks(CShaderManager& shaderManager, const glm::mat4& vp) {
        shaderManager.use("default");
        blockAtlas->use();

        for (CChunk* curChunk : chunks) {
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

    CTexture *blockAtlas;
    CBlockInfo blockInfo;
    CNoiseGenerator noiseGen;
};

#endif // C_CHUNK_MANAGER_H
