#include "World.h"

#include <chrono>
#include <fstream>
#include <iostream>
#include <string>
#include <queue>

#include <SDL2/SDL_image.h>

void World::init(TextureManager& g_TextureManager, const std::vector<NoiseGenerator>& noiseGens, Camera* camera)
{
    this->noiseGens = noiseGens;
    this->camera = camera;

    blockAtlas = g_BlockManager.loadBlockInfo(g_TextureManager);
    biomeManager.loadBiomeInfo();

    boxOutline.init(glm::vec3(0, 0, 0));

    chunkGenThread = std::thread(&World::genThreadFunc, this);
    meshUpdateThread = std::thread(&World::updateThreadFunc, this);
    initAndFreeThread = std::thread(&World::initfreeThreadFunc, this);
}

void World::renderChunks(ShaderManager& g_ShaderManager, const glm::mat4& vp)
{
    g_ShaderManager.use("default");
    blockAtlas->use();

    std::vector<Chunk*> fetchedChunks;
    chunkTree.getFrustumChunks(fetchedChunks, camera->genFrustum(), (ChunkTree::EChunkFlags)(ChunkTree::NEED_STATE_UPDATE | ChunkTree::RENDERABLE));

    for (Chunk* curChunk : fetchedChunks) {
        if (curChunk->chunkNeedsStateUpdate()) {
            curChunk->updateOpenGLState();
        }

        glm::mat4 mvp = vp*glm::translate(glm::mat4(1), curChunk->getPosition());
        glUniformMatrix4fv(g_ShaderManager.getUniformLocation("MVP"), 1, GL_FALSE, &(mvp[0][0]));

        curChunk->render();
    }

    chunkTree.eraseChunks();
    chunkTree.initChunks();
}

void World::replaceBlock(const Chunk::BlockDetails& newBlock)
{
    glm::vec3 temp((float)Chunk::CHUNK_WIDTH, (float)Chunk::CHUNK_HEIGHT, (float)Chunk::CHUNK_DEPTH);
    glm::vec3 pos = glm::floor(newBlock.position/temp)*temp;

    Chunk* fetchedChunks[7];
    fetchedChunks[0] = chunkTree.getChunk(pos, ChunkTree::ALL);
    fetchedChunks[1] = chunkTree.getChunk(pos + glm::vec3((float)Chunk::CHUNK_WIDTH, 0.0f, 0.0f), ChunkTree::ALL);
    fetchedChunks[2] = chunkTree.getChunk(pos - glm::vec3((float)Chunk::CHUNK_WIDTH, 0.0f, 0.0f), ChunkTree::ALL);
    fetchedChunks[3] = chunkTree.getChunk(pos + glm::vec3(0.0f, (float)Chunk::CHUNK_HEIGHT, 0.0f), ChunkTree::ALL);
    fetchedChunks[4] = chunkTree.getChunk(pos - glm::vec3(0.0f, (float)Chunk::CHUNK_HEIGHT, 0.0f), ChunkTree::ALL);
    fetchedChunks[5] = chunkTree.getChunk(pos + glm::vec3(0.0f, 0.0f, (float)Chunk::CHUNK_DEPTH), ChunkTree::ALL);
    fetchedChunks[6] = chunkTree.getChunk(pos - glm::vec3(0.0f, 0.0f, (float)Chunk::CHUNK_DEPTH), ChunkTree::ALL);

    // Force generation of a blank chunk
    if (fetchedChunks[0] == nullptr) {
        chunkTree.addChunk(pos);
        fetchedChunks[0] = chunkTree.getChunk(pos, ChunkTree::ALL);
    }

    fetchedChunks[0]->replaceBlock(newBlock, &(fetchedChunks[1]));
    userRequest = true;
}

bool World::traceRayToBlock(Chunk::BlockDetails& lookBlock,
                     const glm::vec3& rayOrigin, const glm::vec3& rayDir, bool ignoreAir)
{
    Chunk::BlockDetails closest;
    closest.position = rayOrigin + rayDir*10240.0f;

    std::vector<Chunk*> fetchedChunks;
    glm::vec3 rayDir_inverted(1.0f/rayDir.x, 1.0f/rayDir.y, 1.0f/rayDir.z);
    chunkTree.getIntersectingChunks(fetchedChunks, rayOrigin, rayDir_inverted);

    for (Chunk* curChunk : fetchedChunks) {
        if (curChunk->traceRayToBlock(lookBlock, rayOrigin, rayDir, g_BlockManager, ignoreAir)) {
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

void World::generateStructure(const Structure& genStruct, const glm::vec3& pos)
{
    Chunk::BlockDetails block;
    std::vector<Structure::Data> structData;

    genStruct.getBlocks(structData);
    for (auto structBlock : structData) {
        block.id = g_BlockManager.getBlock(structBlock.blockName).getID();
        block.position = structBlock.blockPos + pos;

        replaceBlock(block);
    }
}

void World::addLightSource(const glm::vec3& pos, int intensity, bool sunlight)
{
    std::queue<glm::vec3> lightBFS;

    // I tried a simple Breadth-First Search algorithm to flood-fill everything
    setLightLevel(pos, intensity, sunlight);
    lightBFS.push(pos);
    while (!lightBFS.empty()) {
        glm::vec3 curBlock = lightBFS.front();
        lightBFS.pop();

        int lightLevel = getLightLevel(curBlock, sunlight);
        glm::vec3 diffs[6] = {
            glm::vec3(1, 0, 0), glm::vec3(-1, 0, 0),
            glm::vec3(0, 1, 0), glm::vec3(0, -1, 0),
            glm::vec3(0, 0, 1), glm::vec3(0, 0, -1)
        };

        for (int i = 0; i < 6; ++i) {
            int curLevel = getLightLevel(curBlock + diffs[i], sunlight);
            if (curLevel >= 0 && curLevel + 2 <= lightLevel) {
                glm::vec3 nextBlock = curBlock + diffs[i];

                setLightLevel(nextBlock, lightLevel - 1, sunlight);
                lightBFS.push(nextBlock);
            }
        }
    }
}

void World::remLightSource(const glm::vec3& pos, bool sunlight)
{
    struct node {
        node(const glm::vec3& p, int pi): pos(p), prevIntensity(pi) {}
        glm::vec3 pos;
        int prevIntensity;
    };
    std::queue<node> lightBFSrem;
    std::queue<glm::vec3> lightBFSadd;

    lightBFSrem.emplace(pos, getLightLevel(pos, sunlight));
    setLightLevel(pos, 0, sunlight);

    while (!lightBFSrem.empty()) {
        node curNode = lightBFSrem.front();
        lightBFSrem.pop();

        glm::vec3 diffs[6] = {
            glm::vec3(1, 0, 0), glm::vec3(-1, 0, 0),
            glm::vec3(0, 1, 0), glm::vec3(0, -1, 0),
            glm::vec3(0, 0, 1), glm::vec3(0, 0, -1)
        };

        for (int i = 0; i < 6; ++i) {
            int curLevel = getLightLevel(curNode.pos + diffs[i], sunlight);
            if (curLevel >= 0) {
                if (curLevel != 0 && curLevel < curNode.prevIntensity) {
                    setLightLevel(curNode.pos + diffs[i], 0, sunlight);
                    lightBFSrem.emplace(curNode.pos + diffs[i], curLevel);
                } else if (curLevel >= curNode.prevIntensity) {
                    lightBFSadd.push(curNode.pos + diffs[i]);
                }
            }
        }
    }

    // Needs an additional pass with addLightSource algorithm to fill in the gaps
    while (!lightBFSadd.empty()) {
        glm::vec3 curBlock = lightBFSadd.front();
        lightBFSadd.pop();

        int lightLevel = getLightLevel(curBlock, sunlight);
        glm::vec3 diffs[6] = {
            glm::vec3(1, 0, 0), glm::vec3(-1, 0, 0),
            glm::vec3(0, 1, 0), glm::vec3(0, -1, 0),
            glm::vec3(0, 0, 1), glm::vec3(0, 0, -1)
        };

        for (int i = 0; i < 6; ++i) {
            int curLevel = getLightLevel(curBlock + diffs[i], sunlight);
            if (curLevel >= 0 && curLevel + 2 <= lightLevel) {
                glm::vec3 nextBlock = curBlock + diffs[i];

                setLightLevel(nextBlock, lightLevel - 1, sunlight);
                lightBFSadd.push(nextBlock);
            }
        }
    }
}

void World::changeSunlight(int intensity)
{
    glm::vec3 chunkMin((int)MIN_CHUNK_X, (int)MIN_CHUNK_Y, (int)MIN_CHUNK_Z);
    glm::vec3 chunkMax((int)MAX_CHUNK_X, (int)MAX_CHUNK_Y, (int)MAX_CHUNK_Z);
    glm::vec3 farVec(camera->getFarClipDistance());

    std::vector<Chunk*> fetchedChunks;
    glm::vec3 camPos = camera->getPosition();

    chunkTree.getChunkArea(fetchedChunks, utils3d::AABBox(glm::max(camPos - farVec, chunkMin), glm::min(camPos + farVec, chunkMax)));

    // Fill air above surface with the correct value
    for (Chunk* chunk : fetchedChunks) {
        Chunk::SBlock chunkData[Chunk::CHUNK_WIDTH][Chunk::CHUNK_DEPTH][Chunk::CHUNK_HEIGHT];
        chunk->getBlockData((Chunk::SBlock*)chunkData);

        for (int i = 0; i < Chunk::CHUNK_WIDTH; ++i) {
            for (int j = 0; j < Chunk::CHUNK_DEPTH; ++j) {
                for (int k = Chunk::CHUNK_HEIGHT - 1; k >= 0; --k) {
                    if (g_BlockManager.getBlock(chunkData[i][j][k].id).isTransparent()) {
                        chunkData[i][j][k].meta = (chunkData[i][j][k].meta & 0xFF0F) | (glm::clamp(intensity, 0, 15) << 4);
                    } else {
                        break;
                    }
                }
            }
        }
        chunk->setBlockData((Chunk::SBlock*)chunkData, true);
    }
/*
    // Remove previous lighting
    for (Chunk* chunk : fetchedChunks) {
        Chunk::SBlock chunkData[Chunk::CHUNK_WIDTH][Chunk::CHUNK_DEPTH][Chunk::CHUNK_HEIGHT];
        chunk->getBlockData((Chunk::SBlock*)chunkData);

        for (int i = 0; i < Chunk::CHUNK_WIDTH; ++i) {
            for (int j = 0; j < Chunk::CHUNK_DEPTH; ++j) {
                int k;
                for (k = Chunk::CHUNK_HEIGHT - 1; k >= 0; --k) {
                    if (!g_BlockManager.getBlock(chunkData[i][j][k].id).isTransparent()) {
                        ++k;
                        break;
                    }
                }

                if (k >= 0 && k < Chunk::CHUNK_HEIGHT) {
                    remLightSource(chunk->getPosition() + glm::vec3(i, j, k), true);
                }
            }
        }
    }

    // Fill in the new lighting
    for (Chunk* chunk : fetchedChunks) {
        Chunk::SBlock chunkData[Chunk::CHUNK_WIDTH][Chunk::CHUNK_DEPTH][Chunk::CHUNK_HEIGHT];
        chunk->getBlockData((Chunk::SBlock*)chunkData);

        for (int i = 0; i < Chunk::CHUNK_WIDTH; ++i) {
            for (int j = 0; j < Chunk::CHUNK_DEPTH; ++j) {
                int k;
                for (k = Chunk::CHUNK_HEIGHT - 1; k >= 0; --k) {
                    if (!g_BlockManager.getBlock(chunkData[i][j][k].id).isTransparent()) {
                        ++k;
                        break;
                    }
                }

                if (k >= 0 && k < Chunk::CHUNK_HEIGHT) {
                    addLightSource(chunk->getPosition() + glm::vec3(i, j, k), intensity, true);
                }
            }
        }
    }
    */
}

void World::genThreadFunc()
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

            std::vector<Chunk::StructToGenerate> genStructs;
            curChunk->genBlocks(biomeManager, g_BlockManager, noiseGens, chunkArea, genStructs);

            for (auto genStruct : genStructs) {
                generateStructure(g_StructureManager.getStructure(genStruct.name), genStruct.position);
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

            std::vector<Chunk::StructToGenerate> genStructs;
            curChunk->genBlocks(biomeManager, g_BlockManager, noiseGens, chunkArea, genStructs);

            for (auto genStruct : genStructs) {
                generateStructure(g_StructureManager.getStructure(genStruct.name), genStruct.position);
            }

            if (!keepRunning) {
                return;
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void World::updateThreadFunc()
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
            curChunk->genMesh(g_BlockManager, chunkArea);

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
            curChunk->genMesh(g_BlockManager, chunkArea);

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

void World::initfreeThreadFunc()
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
        chunkTree.genNewChunks(activeArea, 15);

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}
