#include "CChunkManager.h"

void CChunkManager::init(CTextureManager& textureManager, const CNoiseGenerator& noiseGen)
{
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

    for (int i = -8; i <= 8; ++i) {
        for (int j = -8; j <= 8; ++j) {
            CChunk* newChunk = new CChunk(glm::vec3(i*16, 0, j*16));
            chunks.insert(newChunk);
        }
    }

    boxOutline.init(glm::vec3(0, 0, 0));
}

void CChunkManager::renderChunks(CShaderManager& shaderManager, const glm::mat4& vp)
{
    shaderManager.use("default");
    blockAtlas->use();

    // Just for testing, generate/update a chunk per frame
    bool generated = false;
    bool updatedMesh = false;

    for (CChunk* curChunk : chunks) {
        if (!curChunk->isChunkGenerated() && !generated) {
            CChunk *adjacent[6];
            findAdjacentChunks(*curChunk, adjacent);
            curChunk->genBlocks(blockInfo, noiseGen, adjacent);
            generated = true;
        }
        if (curChunk->chunkNeedsMeshUpdate() && !updatedMesh) {
            CChunk *adjacent[6];
            findAdjacentChunks(*curChunk, adjacent);
            curChunk->genMesh(blockInfo, adjacent);
            updatedMesh = true;
        }
        if (curChunk->chunkNeedsStateUpdate()) {
            curChunk->updateOpenGLState();
        }

        if (curChunk->isChunkRenderable()) {
            glm::mat4 mvp = vp*glm::translate(glm::mat4(1), curChunk->getPosition());
            glUniformMatrix4fv(shaderManager.getUniformLocation("MVP"), 1, GL_FALSE, &(mvp[0][0]));

            curChunk->render();
        }
    }
}

void CChunkManager::replaceBlock(const CChunk::BlockDetails& newBlock)
{
    glm::vec3 mod((int)glm::floor(newBlock.position.x) % CChunk::CHUNK_WIDTH,
                  (int)glm::floor(newBlock.position.y) % CChunk::CHUNK_HEIGHT,
                  (int)glm::floor(newBlock.position.z) % CChunk::CHUNK_DEPTH);
    glm::vec3 pos;
    pos.x = (int)glm::floor(newBlock.position.x) - ((mod.x < 0) ? (mod.x + CChunk::CHUNK_WIDTH) : mod.x);
    pos.y = (int)glm::floor(newBlock.position.y) - ((mod.y < 0) ? (mod.y + CChunk::CHUNK_HEIGHT) : mod.y);
    pos.z = (int)glm::floor(newBlock.position.z) - ((mod.z < 0) ? (mod.z + CChunk::CHUNK_DEPTH) : mod.z);

    for (CChunk* curChunk : chunks) {
        if (curChunk->isChunkRenderable() && pos == curChunk->getPosition()) {
            CChunk *adjacent[6];
            findAdjacentChunks(*curChunk, adjacent);
            curChunk->replaceBlock(newBlock, adjacent);
            break;
        }
    }
}

bool CChunkManager::traceRayToBlock(CChunk::BlockDetails& lookBlock,
                     const glm::vec3& rayOrigin, const glm::vec3& rayDir, bool ignoreAir)
{
    CChunk::BlockDetails closest;
    closest.position = rayOrigin + rayDir*1024.0f;
    for (CChunk* curChunk : chunks) {
        if (curChunk->isChunkRenderable() &&
            curChunk->traceRayToBlock(lookBlock, rayOrigin, rayDir, blockInfo, ignoreAir)) {
            glm::vec3 distVec1 = closest.position - rayOrigin;
            glm::vec3 distVec2 = lookBlock.position - rayOrigin;
            if (glm::dot(distVec2, distVec2) < glm::dot(distVec1, distVec1)) {
                closest.position = lookBlock.position;
            }
        }
    }

    if (closest.position != rayOrigin + rayDir*1024.0f) {
        lookBlock = closest;
        return true;
    }
    return false;
}

void CChunkManager::findAdjacentChunks(const CChunk& center, CChunk *adjacent[6])
{
    for (int i = 0; i < 6; ++i) {
        adjacent[i] = nullptr;
    }

    for (CChunk *curChunk : chunks) {
        glm::vec3 centerPos = center.getPosition();
        if (curChunk->getPosition() == glm::vec3(centerPos.x+CChunk::CHUNK_WIDTH, centerPos.y, centerPos.z)) {
            adjacent[0] = curChunk;
        } else if (curChunk->getPosition() == glm::vec3(centerPos.x-CChunk::CHUNK_WIDTH, centerPos.y, centerPos.z)) {
            adjacent[1] = curChunk;
        } else if (curChunk->getPosition() == glm::vec3(centerPos.x, centerPos.y+CChunk::CHUNK_HEIGHT, centerPos.z)) {
            adjacent[2] = curChunk;
        } else if (curChunk->getPosition() == glm::vec3(centerPos.x, centerPos.y-CChunk::CHUNK_HEIGHT, centerPos.z)) {
            adjacent[3] = curChunk;
        } else if (curChunk->getPosition() == glm::vec3(centerPos.x, centerPos.y, centerPos.z+CChunk::CHUNK_DEPTH)) {
            adjacent[4] = curChunk;
        } else if (curChunk->getPosition() == glm::vec3(centerPos.x, centerPos.y, centerPos.z-CChunk::CHUNK_DEPTH)) {
            adjacent[5] = curChunk;
        }
    }
}
