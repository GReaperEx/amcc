#include "ChunkManager.h"

#include <chrono>
#include <fstream>
#include <iostream>
#include <string>

#include <SDL2/SDL_image.h>

void ChunkManager::init(TextureManager& textureManager, const std::vector<NoiseGenerator>& noiseGens, Camera* camera)
{
    this->noiseGens = noiseGens;
    this->camera = camera;

    blockAtlas = blockManager.loadBlockInfo(textureManager);
    biomeManager.loadBiomeInfo();

    boxOutline.init(glm::vec3(0, 0, 0));

    chunkGenThread = std::thread(&ChunkManager::genThreadFunc, this);
    meshUpdateThread = std::thread(&ChunkManager::updateThreadFunc, this);
    initAndFreeThread = std::thread(&ChunkManager::initfreeThreadFunc, this);
}

void ChunkManager::renderChunks(ShaderManager& shaderManager, const glm::mat4& vp)
{
    shaderManager.use("default");
    blockAtlas->use();

    std::vector<Chunk*> fetchedChunks;
    chunkTree.getFrustumChunks(fetchedChunks, camera->genFrustum(), (ChunkTree::EChunkFlags)(ChunkTree::NEED_STATE_UPDATE | ChunkTree::RENDERABLE));

    for (Chunk* curChunk : fetchedChunks) {
        if (curChunk->chunkNeedsStateUpdate()) {
            curChunk->updateOpenGLState();
        }

        glm::mat4 mvp = vp*glm::translate(glm::mat4(1), curChunk->getPosition());
        glUniformMatrix4fv(shaderManager.getUniformLocation("MVP"), 1, GL_FALSE, &(mvp[0][0]));

        curChunk->render();
    }

    chunkTree.eraseChunks();
    chunkTree.initChunks();
}

void ChunkManager::replaceBlock(const Chunk::BlockDetails& newBlock)
{
    glm::vec3 temp((float)Chunk::CHUNK_WIDTH, (float)Chunk::CHUNK_HEIGHT, (float)Chunk::CHUNK_DEPTH);
    glm::vec3 pos = glm::floor(newBlock.position/temp)*temp;

    Chunk* fetchedChunks[7];
    fetchedChunks[0] = chunkTree.getChunk(pos);
    fetchedChunks[1] = chunkTree.getChunk(pos + glm::vec3((float)Chunk::CHUNK_WIDTH, 0.0f, 0.0f));
    fetchedChunks[2] = chunkTree.getChunk(pos - glm::vec3((float)Chunk::CHUNK_WIDTH, 0.0f, 0.0f));
    fetchedChunks[3] = chunkTree.getChunk(pos + glm::vec3(0.0f, (float)Chunk::CHUNK_HEIGHT, 0.0f));
    fetchedChunks[4] = chunkTree.getChunk(pos - glm::vec3(0.0f, (float)Chunk::CHUNK_HEIGHT, 0.0f));
    fetchedChunks[5] = chunkTree.getChunk(pos + glm::vec3(0.0f, 0.0f, (float)Chunk::CHUNK_DEPTH));
    fetchedChunks[6] = chunkTree.getChunk(pos - glm::vec3(0.0f, 0.0f, (float)Chunk::CHUNK_DEPTH));

    if (fetchedChunks[0] != nullptr) {
        fetchedChunks[0]->replaceBlock(newBlock, &(fetchedChunks[1]));
        userRequest = true;
    }

}

bool ChunkManager::traceRayToBlock(Chunk::BlockDetails& lookBlock,
                     const glm::vec3& rayOrigin, const glm::vec3& rayDir, bool ignoreAir)
{
    Chunk::BlockDetails closest;
    closest.position = rayOrigin + rayDir*10240.0f;

    std::vector<Chunk*> fetchedChunks;
    glm::vec3 rayDir_inverted(1.0f/rayDir.x, 1.0f/rayDir.y, 1.0f/rayDir.z);
    chunkTree.getIntersectingChunks(fetchedChunks, rayOrigin, rayDir_inverted);

    for (Chunk* curChunk : fetchedChunks) {
        if (curChunk->traceRayToBlock(lookBlock, rayOrigin, rayDir, blockManager, ignoreAir)) {
            glm::vec3 distVec1 = closest.position - rayOrigin;
            glm::vec3 distVec2 = lookBlock.position - rayOrigin;
            if (glm::dot(distVec2, distVec2) < glm::dot(distVec1, distVec1)) {
                closest.position = lookBlock.position;
            }
        }
    }

    if (closest.position != rayOrigin + rayDir*10240.0f) {
        lookBlock = closest;
        return true;
    }
    return false;
}

void ChunkManager::genThreadFunc()
{
    using namespace std::chrono;
    while (keepRunning) {
        high_resolution_clock::time_point startTime = high_resolution_clock::now();

        glm::vec3 chunkMin((int)MIN_CHUNK_X, (int)MIN_CHUNK_Y, (int)MIN_CHUNK_Z);
        glm::vec3 chunkMax((int)MAX_CHUNK_X, (int)MAX_CHUNK_Y, (int)MAX_CHUNK_Z);
        glm::vec3 farVec(camera->getFarClipDistance());

        std::vector<Chunk*> fetchedChunks;
        glm::vec3 camPos = camera->getPosition();

        chunkTree.getFrustumChunks(fetchedChunks, camera->genFrustum(), ChunkTree::NEED_GENERATION);

        // Sorting the chunks from closest (to the camera), using a lambda function
        std::sort(fetchedChunks.begin(), fetchedChunks.end(), [this](Chunk* a, Chunk* b) {
            glm::vec3 aVec = a->getPosition() - camera->getPosition();
            glm::vec3 bVec = b->getPosition() - camera->getPosition();
            return glm::dot(aVec, aVec) < glm::dot(bVec, bVec);
        });

        // Generating chunks visible to the camera
        for (Chunk* curChunk : fetchedChunks) {
            Chunk* chunkArea[6];
            glm::vec3 pos = curChunk->getPosition();
            chunkArea[0] = chunkTree.getChunk(pos + glm::vec3((float)Chunk::CHUNK_WIDTH, 0.0f, 0.0f));
            chunkArea[1] = chunkTree.getChunk(pos - glm::vec3((float)Chunk::CHUNK_WIDTH, 0.0f, 0.0f));
            chunkArea[2] = chunkTree.getChunk(pos + glm::vec3(0.0f, (float)Chunk::CHUNK_HEIGHT, 0.0f));
            chunkArea[3] = chunkTree.getChunk(pos - glm::vec3(0.0f, (float)Chunk::CHUNK_HEIGHT, 0.0f));
            chunkArea[4] = chunkTree.getChunk(pos + glm::vec3(0.0f, 0.0f, (float)Chunk::CHUNK_DEPTH));
            chunkArea[5] = chunkTree.getChunk(pos - glm::vec3(0.0f, 0.0f, (float)Chunk::CHUNK_DEPTH));
            curChunk->genBlocks(biomeManager, blockManager, noiseGens, chunkArea);

            if (!keepRunning) {
                return;
            }

            // Don't execute for more than a second
            if (duration_cast<seconds>(high_resolution_clock::now() - startTime).count() >= 1) {
                break;
            }
        }

        fetchedChunks.clear();
        chunkTree.getChunkArea(fetchedChunks, utils3d::AABBox(glm::max(camPos - farVec, chunkMin), glm::min(camPos + farVec, chunkMax)), ChunkTree::NEED_GENERATION);

        std::sort(fetchedChunks.begin(), fetchedChunks.end(), [this](Chunk* a, Chunk* b) {
            glm::vec3 aVec = a->getPosition() - camera->getPosition();
            glm::vec3 bVec = b->getPosition() - camera->getPosition();
            return glm::dot(aVec, aVec) < glm::dot(bVec, bVec);
        });

        // Generating everything else
        for (Chunk* curChunk : fetchedChunks) {
            if (duration_cast<seconds>(high_resolution_clock::now() - startTime).count() >= 1) {
                break;
            }

            Chunk* chunkArea[6];
            glm::vec3 pos = curChunk->getPosition();
            chunkArea[0] = chunkTree.getChunk(pos + glm::vec3((float)Chunk::CHUNK_WIDTH, 0.0f, 0.0f));
            chunkArea[1] = chunkTree.getChunk(pos - glm::vec3((float)Chunk::CHUNK_WIDTH, 0.0f, 0.0f));
            chunkArea[2] = chunkTree.getChunk(pos + glm::vec3(0.0f, (float)Chunk::CHUNK_HEIGHT, 0.0f));
            chunkArea[3] = chunkTree.getChunk(pos - glm::vec3(0.0f, (float)Chunk::CHUNK_HEIGHT, 0.0f));
            chunkArea[4] = chunkTree.getChunk(pos + glm::vec3(0.0f, 0.0f, (float)Chunk::CHUNK_DEPTH));
            chunkArea[5] = chunkTree.getChunk(pos - glm::vec3(0.0f, 0.0f, (float)Chunk::CHUNK_DEPTH));
            curChunk->genBlocks(biomeManager, blockManager, noiseGens, chunkArea);

            if (!keepRunning) {
                return;
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void ChunkManager::updateThreadFunc()
{
    using namespace std::chrono;
    while (keepRunning) {
        high_resolution_clock::time_point startTime = high_resolution_clock::now();

        glm::vec3 chunkMin((int)MIN_CHUNK_X, (int)MIN_CHUNK_Y, (int)MIN_CHUNK_Z);
        glm::vec3 chunkMax((int)MAX_CHUNK_X, (int)MAX_CHUNK_Y, (int)MAX_CHUNK_Z);
        glm::vec3 farVec(camera->getFarClipDistance());

        std::vector<Chunk*> fetchedChunks;
        glm::vec3 camPos = camera->getPosition();

        chunkTree.getFrustumChunks(fetchedChunks, camera->genFrustum(), ChunkTree::NEED_MESH_UPDATE);

        // Sorting the chunks from closest (to the camera), using a lambda function
        std::sort(fetchedChunks.begin(), fetchedChunks.end(), [this](Chunk* a, Chunk* b) {
            glm::vec3 aVec = a->getPosition() - camera->getPosition();
            glm::vec3 bVec = b->getPosition() - camera->getPosition();
            return glm::dot(aVec, aVec) < glm::dot(bVec, bVec);
        });

        // Updating chunks visible to camera
        for (Chunk* curChunk : fetchedChunks) {
            Chunk* chunkArea[6];
            glm::vec3 pos = curChunk->getPosition();
            chunkArea[0] = chunkTree.getChunk(pos + glm::vec3((float)Chunk::CHUNK_WIDTH, 0.0f, 0.0f));
            chunkArea[1] = chunkTree.getChunk(pos - glm::vec3((float)Chunk::CHUNK_WIDTH, 0.0f, 0.0f));
            chunkArea[2] = chunkTree.getChunk(pos + glm::vec3(0.0f, (float)Chunk::CHUNK_HEIGHT, 0.0f));
            chunkArea[3] = chunkTree.getChunk(pos - glm::vec3(0.0f, (float)Chunk::CHUNK_HEIGHT, 0.0f));
            chunkArea[4] = chunkTree.getChunk(pos + glm::vec3(0.0f, 0.0f, (float)Chunk::CHUNK_DEPTH));
            chunkArea[5] = chunkTree.getChunk(pos - glm::vec3(0.0f, 0.0f, (float)Chunk::CHUNK_DEPTH));
            curChunk->genMesh(blockManager, chunkArea);

            if (userRequest) {
                break;
            }
            if (!keepRunning) {
                return;
            }

            // Don't execute for more than a second
            if (duration_cast<seconds>(high_resolution_clock::now() - startTime).count() >= 1) {
                break;
            }
        }

        fetchedChunks.clear();
        chunkTree.getChunkArea(fetchedChunks, utils3d::AABBox(glm::max(camPos - farVec, chunkMin), glm::min(camPos + farVec, chunkMax)), ChunkTree::NEED_MESH_UPDATE);
        std::sort(fetchedChunks.begin(), fetchedChunks.end(), [this](Chunk* a, Chunk* b) {
            glm::vec3 aVec = a->getPosition() - camera->getPosition();
            glm::vec3 bVec = b->getPosition() - camera->getPosition();
            return glm::dot(aVec, aVec) < glm::dot(bVec, bVec);
        });

        // Updating everything else
        for (Chunk* curChunk : fetchedChunks) {
            if (userRequest || duration_cast<seconds>(high_resolution_clock::now() - startTime).count() >= 1) {
                break;
            }

            Chunk* chunkArea[6];
            glm::vec3 pos = curChunk->getPosition();
            chunkArea[0] = chunkTree.getChunk(pos + glm::vec3((float)Chunk::CHUNK_WIDTH, 0.0f, 0.0f));
            chunkArea[1] = chunkTree.getChunk(pos - glm::vec3((float)Chunk::CHUNK_WIDTH, 0.0f, 0.0f));
            chunkArea[2] = chunkTree.getChunk(pos + glm::vec3(0.0f, (float)Chunk::CHUNK_HEIGHT, 0.0f));
            chunkArea[3] = chunkTree.getChunk(pos - glm::vec3(0.0f, (float)Chunk::CHUNK_HEIGHT, 0.0f));
            chunkArea[4] = chunkTree.getChunk(pos + glm::vec3(0.0f, 0.0f, (float)Chunk::CHUNK_DEPTH));
            chunkArea[5] = chunkTree.getChunk(pos - glm::vec3(0.0f, 0.0f, (float)Chunk::CHUNK_DEPTH));
            curChunk->genMesh(blockManager, chunkArea);

            if (userRequest) {
                break;
            }
            if (!keepRunning) {
                return;
            }
        }

        if (userRequest) {
            userRequest = false;
            continue;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void ChunkManager::initfreeThreadFunc()
{
    while (keepRunning) {
        glm::vec3 cameraPos = camera->getPosition();

        glm::vec3 chunkDims((int)Chunk::CHUNK_WIDTH, (int)Chunk::CHUNK_HEIGHT, (int)Chunk::CHUNK_DEPTH);
        glm::vec3 chunkMin((int)MIN_CHUNK_X, (int)MIN_CHUNK_Y, (int)MIN_CHUNK_Z);
        glm::vec3 chunkMax((int)MAX_CHUNK_X, (int)MAX_CHUNK_Y, (int)MAX_CHUNK_Z);
        glm::vec3 farVec(camera->getFarClipDistance());

        utils3d::AABBox activeArea(glm::max(cameraPos - farVec, chunkMin), glm::min(cameraPos + farVec, chunkMax));
        activeArea.minVec = glm::floor(activeArea.minVec/chunkDims)*chunkDims;
        activeArea.maxVec = glm::ceil(activeArea.maxVec/chunkDims)*chunkDims;

        chunkTree.eraseOldChunks(activeArea);
        chunkTree.genNewChunks(activeArea);

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}