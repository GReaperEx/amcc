#ifndef BLOCK_INFO_H
#define BLOCK_INFO_H

#include <map>
#include <string>
#include <cstring>

#include <glm/glm.hpp>

class BlockInfo
{
public:
    enum EBlockSide { RIGHT = 0, LEFT, TOP, BOTTOM, BACK, FRONT };

public:
    BlockInfo() {
        SBlock temp;
        temp.name = "air";
        blockInfoMap[0] = temp;
        idMap["air"] = 0;
    }

    void addBlock(const std::string& name, uint16_t id, const glm::vec2 uvCoords[6][4]) {
        idMap[name] = id;
        blockInfoMap[id].name = name;
        memcpy(blockInfoMap[id].UVs, uvCoords, sizeof(glm::vec2)*6*4);
    }

    uint16_t getBlockID(const std::string& name) const {
        return idMap.find(name)->second;
    }

    void getBlockUVs(uint16_t id, glm::vec2 uvCoords[6][4]) const {
        memcpy(uvCoords, blockInfoMap.find(id)->second.UVs, sizeof(glm::vec2)*6*4);
    }

    const std::string& getBlockName(uint16_t id) const {
        return blockInfoMap.find(id)->second.name;
    }

private:
    struct SBlock
    {
        glm::vec2 UVs[6][4];
        std::string name;
    };
    std::map<std::string, uint16_t> idMap;
    std::map<uint16_t, SBlock> blockInfoMap;
};

#endif // BLOCK_INFO_H
