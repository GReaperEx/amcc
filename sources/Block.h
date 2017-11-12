#ifndef BLOCK_H
#define BLOCK_H

#include "utils3d.h"

#include <string>
#include <vector>
#include <cstring>

/**
 * An attribute container for differentiating between blocks.
 */
class Block
{
public:
    enum class Shape {
        CUBE,   ///< Simple 1x1x1 cube
        //SLAB,   ///< CUBE cut in half (Default: transparent)
        //STAIRS, ///< Block with indentation resembling stairs (Default: transparent)
        FAKE3D  ///< X-shaped, aligned around Y-axis (Default: transparent)
    };

    struct Data
    {
        uint16_t id;   ///< Numeric ID, unique for different blocks
        uint16_t meta; ///< Holds additional information

        Data(uint16_t _id, uint16_t _meta): id(_id), meta(_meta) {}
        bool operator== (const Data& other) const {
            return id == other.id;
        }
    };

public:
    explicit Block(uint16_t _id)
    : id(_id)
    {}

    Block(uint16_t _id, const std::string& _name, Shape _shape, const glm::vec2 _UVs[6][4], bool _transparent)
    : id(_id), name(_name), shape(_shape), transparent(_transparent)
    {
        memcpy(UVs, _UVs, sizeof(glm::vec2)*6*4);
    }

    uint16_t getID() const {
        return id;
    }
    const std::string& getName() const {
        return name;
    }
    Shape getShape() const {
        return shape;
    }
    bool isTransparent() const {
        return transparent;
    }

    bool operator< (const Block& other) const {
        return id < other.id;
    }

    /**
     * \param localPos The position of the desired block relative to its chunk
     * \param    sides A pointer for every adjacent block, if existent
     * \param vertices 3D vertex data go here
     * \param      uvs 2D texCoord data go here
     * \param  normals 3D surface normal data go here
     *
     * Produces triangles for every visible side of this block.
     */
    void getMeshData(const glm::vec3& localPos, uint16_t metas[7], const Block* sides[6], std::vector<glm::vec3>& vertices, std::vector<glm::vec2>& uvs, std::vector<float>& lighting) const;

private:
    uint16_t id;
    std::string name;    ///< Internally-used name for portability
    Shape shape;         ///< The silhouette used when rendering
    glm::vec2 UVs[6][4]; ///< Texture coordinates for each and every side

    bool transparent;    ///< Is block see-through (Default: false)
};

#endif // BLOCK_H
