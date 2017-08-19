#ifndef C_CHUNK_H
#define C_CHUNK_H

#include <GL/glew.h>
#include <GL/gl.h>

#include <vector>
#include <atomic>

#include "CBlockInfo.h"
#include "CNoiseGenerator.h"
#include "IRefCounted.h"

class CChunk
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
    };

    static const int CHUNK_WIDTH = 16;
    static const int CHUNK_DEPTH = 16;
    static const int CHUNK_HEIGHT = 256;

public:
    CChunk(const glm::vec3& gPos) {
        position = gPos;
        memset(chunkData, 0, CHUNK_HEIGHT*CHUNK_DEPTH*CHUNK_WIDTH*sizeof(SBlock));

        isStateInited = false;
        isGenerated = false;
        isRenderable = false;
        neededMeshUpdate = false;
        neededStateUpdate = false;

        vertices = nullptr;
        uvs = nullptr;
        normals = nullptr;
    }
    CChunk(const glm::vec3& gPos, const SBlock bakedData[CHUNK_HEIGHT][CHUNK_DEPTH][CHUNK_WIDTH]) {
        position = gPos;
        memcpy(chunkData, bakedData, CHUNK_HEIGHT*CHUNK_DEPTH*CHUNK_WIDTH*sizeof(SBlock));

        isStateInited = false;
        isGenerated = true;
        isRenderable = false;
        neededMeshUpdate = true;
        neededStateUpdate = false;
    }
    ~CChunk() {
        glDeleteBuffers(3, bufferIDs);

        if (vertices) {
            delete vertices;
        }
        if (uvs) {
            delete uvs;
        }
        if (normals) {
            delete normals;
        }
    }

    void initOpenGLState() {
        glGenBuffers(3, bufferIDs);
        isStateInited = true;
    }

    void getBlockData(SBlock outputData[CHUNK_HEIGHT][CHUNK_DEPTH][CHUNK_WIDTH]) const {
        memcpy(outputData, chunkData, CHUNK_HEIGHT*CHUNK_DEPTH*CHUNK_WIDTH*sizeof(SBlock));
    }
    void setBlockData(const SBlock inputData[CHUNK_HEIGHT][CHUNK_DEPTH][CHUNK_WIDTH]) {
        memcpy(chunkData, inputData, CHUNK_HEIGHT*CHUNK_DEPTH*CHUNK_WIDTH*sizeof(SBlock));
        isGenerated = true;
    }

    void genBlocks(const CBlockInfo& blocks, const CNoiseGenerator& noiseGen, CChunk* adjacent[6]);
    void genMesh(const CBlockInfo& blocks, CChunk* adjacent[6]); // Just generates data, doesn't call OpenGL
    void update(float dT);
    void render();

    // This should only be called by the main rendering thread
    void updateOpenGLState() {
        glBindBuffer(GL_ARRAY_BUFFER, bufferIDs[0]);
        glBufferData(GL_ARRAY_BUFFER, vertices->size()*sizeof(glm::vec3), &((*vertices)[0]), GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, bufferIDs[1]);
        glBufferData(GL_ARRAY_BUFFER, uvs->size()*sizeof(glm::vec2), &((*uvs)[0]), GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, bufferIDs[2]);
        glBufferData(GL_ARRAY_BUFFER, normals->size()*sizeof(glm::vec3), &((*normals)[0]), GL_DYNAMIC_DRAW);

        vtxCount = vertices->size();

        neededStateUpdate = false;
        isRenderable = true;

        delete vertices;
        vertices = nullptr;
        delete uvs;
        uvs = nullptr;
        delete normals;
        normals = nullptr;
    }

    const glm::vec3& getPosition() const {
        return position;
    }

    void replaceBlock(const BlockDetails& newBlock, CChunk *adjacent[6]);
    bool traceRayToBlock(BlockDetails& lookBlock, const glm::vec3& rayOrigin, const glm::vec3& rayDir,
                        const CBlockInfo& blockInfo, bool ignoreAir = true);

    bool isStateInitialized() const {
        return isStateInited;
    }

    bool isChunkGenerated() const {
        return isGenerated;
    }

    bool isChunkRenderable() const {
        return isRenderable;
    }

    bool chunkNeedsMeshUpdate() const {
        return neededMeshUpdate;
    }

    bool chunkNeedsStateUpdate() const {
        return neededStateUpdate;
    }

private:
    SBlock chunkData[CHUNK_HEIGHT][CHUNK_DEPTH][CHUNK_WIDTH];
    glm::vec3 position;

    GLuint bufferIDs[3];
    size_t vtxCount;

    std::vector<glm::vec3> *vertices;
    std::vector<glm::vec2> *uvs;
    std::vector<glm::vec3> *normals;

    std::atomic_bool isStateInited;
    std::atomic_bool isGenerated;
    std::atomic_bool isRenderable;
    std::atomic_bool neededMeshUpdate;
    std::atomic_bool neededStateUpdate;

    bool rayBlockIntersection(glm::vec3& lookBlock, const glm::vec3& boxMin, const glm::vec3& boxMax,
                              const glm::vec3& rayPos, const glm::vec3& rayDir,
                              const glm::vec3& rayDir_inverted, bool ignoreAir);
    bool rayBoxIntersection(const glm::vec3& boxMin, const glm::vec3& boxMax,
                            const glm::vec3& rayPos, const glm::vec3& rayDir_inverted);
};

#endif // C_CHUNK_H
