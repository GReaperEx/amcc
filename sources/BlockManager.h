#ifndef BLOCK_MANAGER_H
#define BLOCK_MANAGER_H

#include "Block.h"
#include "TextureManager.h"

class BlockManager
{
public:
    BlockManager() {
        glm::vec2 dummyCoords[6][4] = { glm::vec2(0) };
        blocks.push_back(Block(0, "air", Block::Shape::CUBE, dummyCoords, true));
    }

    const Block& getBlock(uint16_t id) const {
        return *std::lower_bound(blocks.begin(), blocks.end(), Block(id));
    }

    const Block& getBlock(const std::string& name) const {
        auto it = blocks.begin();
        for (; it != blocks.end(); ++it) {
            if (it->getName() == name) {
                break;
            }
        }
        return *it;
    }

    /**
     * \return The texture atlas that was created while loading
     *
     * Loads all defined blocks from config file
     */
    Texture* loadBlockInfo(TextureManager& textureManager);

private:
    std::vector<Block> blocks;
};

#endif // BLOCK_MANAGER_H
