#include "BlockManager.h"

#include <fstream>
#include <iostream>
#include <algorithm>

#include <SDL2/SDL_image.h>

void fatalError(const std::string& prefix, const std::string& msg);

Texture* BlockManager::loadBlockInfo(TextureManager& g_TextureManager)
{
    using std::cout;
    using std::endl;

    const std::string rootDir = "assets/blocks/";

    std::ifstream infile(rootDir + "blocks.cfg");
    if (!infile.is_open()) {
        fatalError("World_Error: ", "Unable to open block config file.");
    }

    struct blockTiles
    {
        std::string right;
        std::string left;
        std::string top;
        std::string bottom;
        std::string back;
        std::string front;

        Block::Shape shape;
        bool isTransparent;
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

            tileMap[name].shape = Block::Shape::CUBE;
            if (side == "all") {
                tileMap[name] = blockTiles{fileName, fileName, fileName, fileName, fileName, fileName, Block::Shape::CUBE, false, tileMap[name].id};
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
            } else if (side == "shape") {
                if (fileName == "cube") {
                    tileMap[name].shape = Block::Shape::CUBE;
                } else if (fileName == "fake3d") {
                    tileMap[name].shape = Block::Shape::FAKE3D;
                }
            } else if (side == "isTransparent") {
                if (fileName == "true") {
                    tileMap[name].isTransparent = true;
                } else {
                    tileMap[name].isTransparent = false;
                }
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
    SDL_Surface *atlas = SDL_CreateRGBSurface(0, dims, dims, 32, 0xFF, 0xFF00, 0xFF0000, 0xFF000000);
    SDL_FillRect(atlas, nullptr, SDL_MapRGBA(atlas->format, 0, 0, 0, 0));

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
    g_TextureManager.addTexture("blockAtlas", new Texture(atlas));
    SDL_FreeSurface(atlas);

    for (auto it = tileMap.begin(); it != tileMap.end(); ++it) {
        glm::vec2 uvCoords[6][4];
        memcpy(uvCoords[0], imageUVs[it->second.right].UVs, 4*sizeof(glm::vec2));
        memcpy(uvCoords[1], imageUVs[it->second.left].UVs, 4*sizeof(glm::vec2));
        memcpy(uvCoords[2], imageUVs[it->second.top].UVs, 4*sizeof(glm::vec2));
        memcpy(uvCoords[3], imageUVs[it->second.bottom].UVs, 4*sizeof(glm::vec2));
        memcpy(uvCoords[4], imageUVs[it->second.back].UVs, 4*sizeof(glm::vec2));
        memcpy(uvCoords[5], imageUVs[it->second.front].UVs, 4*sizeof(glm::vec2));
        blocks.push_back(Block(it->second.id, it->first, it->second.shape, uvCoords, it->second.isTransparent || it->second.shape == Block::Shape::FAKE3D));
    }

    std::sort(blocks.begin(), blocks.end());

    return g_TextureManager.getTexture("blockAtlas");
}

BlockManager g_BlockManager;
