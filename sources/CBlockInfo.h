#ifndef C_BLOCK_INFO_H
#define C_BLOCK_INFO_H

#include <map>
#include <string>
#include <cstring>

#include <glm/glm.hpp>

class CBlockInfo
{
public:
    enum EBlockSide { RIGHT = 0, LEFT, TOP, BOTTOM, BACK, FRONT };

public:
    CBlockInfo() {}

    void addBlock(const std::string& name, uint16_t id, const glm::vec2 uvCoords[6][4]) {
        idMap[name].id = id;
        memcpy(uvMap[id].UVs, uvCoords, sizeof(glm::vec2)*6*4);
    }

    void remBlock(const std::string& name) {
        uint16_t id = idMap[name].id;
        idMap.erase(name);
        uvMap.erase(id);
    }

    uint16_t getBlockID(const std::string& name) const {
        return idMap.find(name)->second.id;
    }

    void getBlockUVs(uint16_t id, glm::vec2 uvCoords[6][4]) const {
        memcpy(uvCoords, uvMap.find(id)->second.UVs, sizeof(glm::vec2)*6*4);
    }

private:
    struct BlockID
    {
        uint16_t id;
    };
    struct BlockUVs
    {
        glm::vec2 UVs[6][4];
    };
    std::map<std::string, BlockID> idMap;
    std::map<uint16_t, BlockUVs> uvMap;
};

#endif // C_BLOCK_INFO_H
