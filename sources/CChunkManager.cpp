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

    CChunk *adjacent[6] = { nullptr };
    for (int i = -8; i <= 8; ++i) {
        for (int j = -8; j <= 8; ++j) {
            CChunk* newChunk = new CChunk(glm::vec3(i*16, 0, j*16));

            chunksToGenerate.insert(newChunk);
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

    if (!chunksToGenerate.empty()) {
        CChunk* chunk = *(chunksToGenerate.begin());
        chunksToGenerate.erase(chunksToGenerate.begin());

        CChunk* adjacent[6] = { nullptr };
        chunk->genBlocks(blockInfo, noiseGen, adjacent);
        chunksToUpdateMesh.insert(chunk);
    }
    if (!chunksToUpdateMesh.empty()) {
        CChunk* chunk = *(chunksToUpdateMesh.begin());
        chunksToUpdateMesh.erase(chunksToUpdateMesh.begin());

        // TODO: Detect adjacent chunks to generate the mesh properly
        CChunk* adjacent[6] = { nullptr };
        chunk->genMesh(blockInfo, adjacent);
        chunksToUpdateState.insert(chunk);
    }
    if (!chunksToUpdateState.empty()) {
        CChunk* chunk = *(chunksToUpdateState.begin());
        chunksToUpdateState.erase(chunksToUpdateState.begin());

        chunk->updateOpenGLState();
        chunksToRender.insert(chunk);
    }

    for (CChunk* curChunk : chunksToRender) {
        glm::mat4 mvp = vp*glm::translate(glm::mat4(1), curChunk->getPosition());
        glUniformMatrix4fv(shaderManager.getUniformLocation("MVP"), 1, GL_FALSE, &(mvp[0][0]));

        curChunk->render();
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

    for (CChunk* curChunk : chunksToRender) {
        if (pos == curChunk->getPosition()) {
            curChunk->replaceBlock(newBlock);
            chunksToRender.erase(curChunk);
            chunksToUpdateMesh.insert(curChunk);
            break;
        }
    }
}

bool CChunkManager::traceRayToBlock(CChunk::BlockDetails& lookBlock,
                     const glm::vec3& rayOrigin, const glm::vec3& rayDir, bool ignoreAir)
{
    CChunk::BlockDetails closest;
    closest.position = rayOrigin + rayDir*1024.0f;
    for (CChunk* curChunk : chunksToRender) {
        if (curChunk->traceRayToBlock(lookBlock, rayOrigin, rayDir, blockInfo, ignoreAir)) {
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
