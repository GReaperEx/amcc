#ifndef STRUCTURE_H
#define STRUCTURE_H

#include <vector>
#include <string>

#include <glm/glm.hpp>

class Structure
{
public:
    struct Data
    {
        glm::vec3 blockPos;
        std::string blockName;
    };

public:
    Structure() {}
    Structure(const std::string& filename);

    void getBlocks(std::vector<Data>& output) const {
        output = blocks;
    }

private:
    std::vector<Data> blocks;
};

#endif // STRUCTURE_H
