#include "CChunkManager.h"

#include <chrono>
#include <fstream>
#include <iostream>
#include <string>

#include <SDL2/SDL_image.h>

void CChunkManager::init(CTextureManager& textureManager, const CNoiseGenerator& noiseGen, CCamera* camera)
{
    this->noiseGen = noiseGen;
    this->camera = camera;

    loadBlockInfo(textureManager);

    for (int i = -16; i < 16; ++i) {
        for (int j = -16; j < 16; ++j) {
            chunkTree.addChunk(glm::vec3(i*16, 0, j*16));
        }
    }
    // ChunkTree shouldn't be used like this, I'm just testing things
    chunkTree.getAllChunks(chunks);

    boxOutline.init(glm::vec3(0, 0, 0));

    chunkGenThread = std::thread(&CChunkManager::genThreadFunc, this);
    meshUpdateThread = std::thread(&CChunkManager::updateThreadFunc, this);
}

void CChunkManager::renderChunks(CShaderManager& shaderManager, const glm::mat4& vp)
{
    shaderManager.use("default");
    blockAtlas->use();

    std::vector<CChunk*> fetchedChunks;
    chunkTree.getFrustumChunks(fetchedChunks, camera->genFrustum());

    for (CChunk* curChunk : fetchedChunks) {
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

    std::unique_lock<std::mutex> lck(chunksBeingUsed);
    ++usageCount;
    usageEvent.notify_all();
    lck.unlock();

    for (CChunk* curChunk : chunks) {
        if (curChunk->isChunkRenderable() && pos == curChunk->getPosition()) {
            CChunk *adjacent[6];
            findAdjacentChunks(*curChunk, adjacent);
            curChunk->replaceBlock(newBlock, adjacent);
            break;
        }
    }

    lck.lock();
    --usageCount;
    usageEvent.notify_all();
}

bool CChunkManager::traceRayToBlock(CChunk::BlockDetails& lookBlock,
                     const glm::vec3& rayOrigin, const glm::vec3& rayDir, bool ignoreAir)
{
    CChunk::BlockDetails closest;
    closest.position = rayOrigin + rayDir*10240.0f;

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

    if (closest.position != rayOrigin + rayDir*10240.0f) {
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
        if (!curChunk->isChunkGenerated()) {
            continue;
        }

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

void CChunkManager::genThreadFunc()
{
    while (true) {
        std::unique_lock<std::mutex> lck(chunksBeingUsed);
        ++usageCount;
        usageEvent.notify_all();
        lck.unlock();

        for (CChunk* curChunk : chunks) {
            if (!curChunk->isChunkGenerated()) {
                CChunk *adjacent[6];
                findAdjacentChunks(*curChunk, adjacent);
                curChunk->genBlocks(blockInfo, noiseGen, adjacent);
            }
        }

        lck.lock();
        --usageCount;
        usageEvent.notify_all();
        lck.unlock();

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void CChunkManager::updateThreadFunc()
{
    while (true) {
        std::unique_lock<std::mutex> lck(chunksBeingUsed);
        ++usageCount;
        usageEvent.notify_all();
        lck.unlock();

        for (CChunk* curChunk : chunks) {
            if (curChunk->chunkNeedsMeshUpdate()) {
                CChunk *adjacent[6];
                findAdjacentChunks(*curChunk, adjacent);
                curChunk->genMesh(blockInfo, adjacent);
            }
        }

        lck.lock();
        --usageCount;
        usageEvent.notify_all();
        lck.unlock();

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void fatalError(const std::string& prefix, const std::string& msg);

void CChunkManager::loadBlockInfo(CTextureManager& textureManager)
{
    using std::cout;
    using std::endl;

    const std::string rootDir = "assets/blocks/";

    std::ifstream infile(rootDir + "blocks.cfg");
    if (!infile.is_open()) {
        fatalError("ChunkManager_Error: ", "Unable to open block config file.");
    }

    struct blockTiles
    {
        std::string right;
        std::string left;
        std::string top;
        std::string bottom;
        std::string back;
        std::string front;

        uint16_t id;
    };
    std::map<std::string, blockTiles> tileMap;
    cout << "Parsing block config file." << endl;
    std::string name;

    int IDs = 1;
    while (infile >> name) {
        infile.ignore(100, '{');
        tileMap[name].id = IDs++;

        std::string side;
        while (infile >> side && side != "}") {
            std::string fileName;
            infile >> fileName;

            if (side == "all") {
                tileMap[name] = blockTiles{fileName, fileName, fileName, fileName, fileName, fileName, tileMap[name].id};
            } else if (side == "right") {
                tileMap[name].right = fileName;
            } else if (side == "left") {
                tileMap[name].left = fileName;
            } else if (side == "top") {
                tileMap[name].top = fileName;
            } else if (side == "bottom") {
                tileMap[name].bottom = fileName;
            } else if (side == "back") {
                tileMap[name].back = fileName;
            } else if (side == "front") {
                tileMap[name].front = fileName;
            }
        }
    }
    infile.close();

    cout << "Loading block tiles." << endl;
    std::map<std::string, SDL_Surface*> images;
    for (auto it = tileMap.begin(); it != tileMap.end(); ++it) {
        if (images.find(it->second.right) == images.end()) {
            auto name = it->second.right;
            images[name] = IMG_Load((rootDir + name).c_str());
            if (images[name] == nullptr) {
                fatalError("Program_Load: ", "Failed to load \'" + (rootDir + name) + "\'.\n" +
                           "IMG_Load: " + IMG_GetError());
            }
        }
        if (images.find(it->second.left) == images.end()) {
            auto name = it->second.left;
            images[name] = IMG_Load((rootDir + name).c_str());
            if (images[name] == nullptr) {
                fatalError("Program_Load: ", "Failed to load \'" + (rootDir + name) + "\'.\n" +
                           "IMG_Load: " + IMG_GetError());
            }
        }
        if (images.find(it->second.top) == images.end()) {
            auto name = it->second.top;
            images[name] = IMG_Load((rootDir + name).c_str());
            if (images[name] == nullptr) {
                fatalError("Program_Load: ", "Failed to load \'" + (rootDir + name) + "\'.\n" +
                           "IMG_Load: " + IMG_GetError());
            }
        }
        if (images.find(it->second.bottom) == images.end()) {
            auto name = it->second.bottom;
            images[name] = IMG_Load((rootDir + name).c_str());
            if (images[name] == nullptr) {
                fatalError("Program_Load: ", "Failed to load \'" + (rootDir + name) + "\'.\n" +
                           "IMG_Load: " + IMG_GetError());
            }
        }
        if (images.find(it->second.back) == images.end()) {
            auto name = it->second.back;
            images[name] = IMG_Load((rootDir + name).c_str());
            if (images[name] == nullptr) {
                fatalError("Program_Load: ", "Failed to load \'" + (rootDir + name) + "\'.\n" +
                           "IMG_Load: " + IMG_GetError());
            }
        }
        if (images.find(it->second.front) == images.end()) {
            auto name = it->second.front;
            images[name] = IMG_Load((rootDir + name).c_str());
            if (images[name] == nullptr) {
                fatalError("Program_Load: ", "Failed to load \'" + (rootDir + name) + "\'.\n" +
                           "IMG_Load: " + IMG_GetError());
            }
        }
    }

    cout << "Generating block atlas." << endl;
    int dims = glm::pow(2.0f, glm::ceil(glm::log2(glm::sqrt((float)images.size()))))*16;
    SDL_Surface *atlas = SDL_CreateRGBSurface(0, dims, dims, 32, 0, 0, 0, 0);

    SDL_Rect curPos;
    curPos.w = curPos.h = 16;
    curPos.x = curPos.y = 0;
    struct tileUVs
    {
        glm::vec2 UVs[4];
    };
    std::map<std::string, tileUVs> imageUVs;
    for (auto it = images.begin(); it != images.end(); ++it) {
        SDL_BlitSurface(it->second, nullptr, atlas, &curPos);
        imageUVs[it->first].UVs[0] = glm::vec2(curPos.x/(float)dims, 1.0f - (curPos.y+16)/(float)dims);
        imageUVs[it->first].UVs[1] = glm::vec2((curPos.x+16)/(float)dims, 1.0f - (curPos.y+16)/(float)dims);
        imageUVs[it->first].UVs[2] = glm::vec2((curPos.x+16)/(float)dims, 1.0f - curPos.y/(float)dims);
        imageUVs[it->first].UVs[3] = glm::vec2(curPos.x/(float)dims, 1.0f - curPos.y/(float)dims);

        curPos.x += 16;
        if (curPos.x == dims) {
            curPos.x = 0;
            curPos.y += 16;
        }
        SDL_FreeSurface(it->second);
    }
    textureManager.addTexture("blockAtlas", new CTexture(atlas));
    SDL_FreeSurface(atlas);

    for (auto it = tileMap.begin(); it != tileMap.end(); ++it) {
        glm::vec2 uvCoords[6][4];
        memcpy(uvCoords[0], imageUVs[it->second.right].UVs, 4*sizeof(glm::vec2));
        memcpy(uvCoords[1], imageUVs[it->second.left].UVs, 4*sizeof(glm::vec2));
        memcpy(uvCoords[2], imageUVs[it->second.top].UVs, 4*sizeof(glm::vec2));
        memcpy(uvCoords[3], imageUVs[it->second.bottom].UVs, 4*sizeof(glm::vec2));
        memcpy(uvCoords[4], imageUVs[it->second.back].UVs, 4*sizeof(glm::vec2));
        memcpy(uvCoords[5], imageUVs[it->second.front].UVs, 4*sizeof(glm::vec2));
        blockInfo.addBlock(it->first, it->second.id, uvCoords);
    }

    blockAtlas = textureManager.getTexture("blockAtlas");
}
