#ifndef CHUNK_H
#define CHUNK_H

#include <GL/glew.h>
#include <GL/gl.h>

#include <vector>
#include <atomic>

#include "BlockManager.h"
#include "NoiseGenerator.h"
#include "BiomeManager.h"

class Chunk
{
public:
    struct BlockDetails
    {
        uint16_t id;
        std::string name;
        glm::vec3 position;
    };

    struct SBlock
    {
        uint16_t id;
        uint16_t meta;

        bool operator== (const SBlock& other) const {
            return id == other.id && meta == other.meta;
        }
    };

    struct StructToGenerate
    {
        glm::vec3 position;
        std::string name;
    };

    static const int CHUNK_WIDTH = 16;
    static const int CHUNK_DEPTH = 16;
    static const int CHUNK_HEIGHT = 256;

public:
    Chunk(const glm::vec3& gPos) {
        position = gPos;
        memset(chunkData, 0, CHUNK_HEIGHT*CHUNK_DEPTH*CHUNK_WIDTH*sizeof(SBlock));

        isStateInited = false;
        isGenerated = false;
        isRenderable = false;
        neededMeshUpdate = false;
        neededStateUpdate = false;
        wasEdited = false;
    }
    Chunk(const glm::vec3& gPos, const SBlock *bakedData, bool wasGenerated) {
        position = gPos;
        memcpy(chunkData, bakedData, CHUNK_HEIGHT*CHUNK_DEPTH*CHUNK_WIDTH*sizeof(SBlock));

        isStateInited = false;
        isGenerated = wasGenerated;
        isRenderable = false;
        neededMeshUpdate = wasGenerated;
        neededStateUpdate = false;
        wasEdited = false;
    }
    ~Chunk() {
        glDeleteBuffers(3, bufferIDs);
    }

    void initOpenGLState() {
        glGenBuffers(3, bufferIDs);
        isStateInited = true;
    }

    void getBlockData(SBlock *outputData) const {
        memcpy(outputData, chunkData, CHUNK_HEIGHT*CHUNK_DEPTH*CHUNK_WIDTH*sizeof(SBlock));
    }
    void setBlockData(const SBlock *inputData, bool wasGenerated) {
        memcpy(chunkData, inputData, CHUNK_HEIGHT*CHUNK_DEPTH*CHUNK_WIDTH*sizeof(SBlock));
        isGenerated = wasGenerated;
        neededMeshUpdate = wasGenerated;
    }

    void genBlocks(const BiomeManager& biomeManager, const BlockManager& blocks, const std::vector<NoiseGenerator>& noiseGen, Chunk* adjacent[6], std::vector<StructToGenerate>& genStructs);
    void genMesh(const BlockManager& blocks, Chunk* adjacent[6]); // Just generates data, doesn't call OpenGL
    void update(float dT);
    void render();

    // This should only be called by the main rendering thread
    void updateOpenGLState() {
        glBindBuffer(GL_ARRAY_BUFFER, bufferIDs[0]);
        glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(glm::vec3), &(vertices[0]), GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, bufferIDs[1]);
        glBufferData(GL_ARRAY_BUFFER, uvs.size()*sizeof(glm::vec2), &(uvs[0]), GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, bufferIDs[2]);
        glBufferData(GL_ARRAY_BUFFER, normals.size()*sizeof(glm::vec3), &(normals[0]), GL_DYNAMIC_DRAW);

        vtxCount = vertices.size();

        neededStateUpdate = false;
        isRenderable = true;
    }

    const glm::vec3& getPosition() const {
        return position;
    }

    void replaceBlock(const BlockDetails& newBlock, Chunk *adjacent[6]);
    bool traceRayToBlock(BlockDetails& lookBlock, const glm::vec3& rayOrigin, const glm::vec3& rayDir,
                        const BlockManager& blockManager, bool ignoreAir = true);

    bool isStateInitialized() const {
        return isStateInited;
    }

    bool isChunkGenerated() const {
        return isGenerated;
    }

    bool isChunkRenderable() const {
        return isRenderable;
    }

    bool wasChunkEdited() const {
        return wasEdited;
    }

    bool chunkNeedsMeshUpdate() const {
        return neededMeshUpdate;
    }

    bool chunkNeedsStateUpdate() const {
        return neededStateUpdate;
    }

private:
    SBlock chunkData[CHUNK_WIDTH][CHUNK_DEPTH][CHUNK_HEIGHT];
    glm::vec3 position;

    GLuint bufferIDs[3];
    size_t vtxCount;

    std::vector<glm::vec3> vertices;
    std::vector<glm::vec2> uvs;
    std::vector<glm::vec3> normals;

    std::atomic_bool isStateInited;
    std::atomic_bool isGenerated;
    std::atomic_bool isRenderable;
    std::atomic_bool neededMeshUpdate;
    std::atomic_bool neededStateUpdate;
    std::atomic_bool wasEdited;

    bool rayBlockIntersection(glm::vec3& lookBlock, const glm::vec3& boxMin, const glm::vec3& boxMax,
                              const glm::vec3& rayPos, const glm::vec3& rayDir,
                              const glm::vec3& rayDir_inverted, bool ignoreAir);
    bool rayBoxIntersection(const glm::vec3& boxMin, const glm::vec3& boxMax,
                            const glm::vec3& rayPos, const glm::vec3& rayDir_inverted);
};

#endif // CHUNK_H
