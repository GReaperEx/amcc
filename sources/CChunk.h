#ifndef C_CHUNK_H
#define C_CHUNK_H

#include <GL/glew.h>
#include <GL/gl.h>

#include <vector>

#include "CBlockInfo.h"

class CChunk
{
public:
    struct SBlock
    {
        SBlock(): id(0) {}

        uint16_t id;
    };

    static const int CHUNK_WIDTH = 16;
    static const int CHUNK_DEPTH = 16;
    static const int CHUNK_HEIGHT = 256;

public:
    CChunk() {
        glGenBuffers(2, bufferIDs);
    }
    CChunk(const SBlock bakedData[CHUNK_HEIGHT][CHUNK_DEPTH][CHUNK_WIDTH]) {
        glGenBuffers(2, bufferIDs);
        memcpy(chunkData, bakedData, CHUNK_HEIGHT*CHUNK_DEPTH*CHUNK_WIDTH*sizeof(SBlock));
    }
    ~CChunk() {
        glDeleteBuffers(2, bufferIDs);
    }

    void getBlockData(SBlock outputData[CHUNK_HEIGHT][CHUNK_DEPTH][CHUNK_WIDTH]) const {
        memcpy(outputData, chunkData, CHUNK_HEIGHT*CHUNK_DEPTH*CHUNK_WIDTH*sizeof(SBlock));
    }

    void genBlocks(const CBlockInfo& blocks, unsigned seed, CChunk* adjacent[6], const glm::vec3& globalPos);
    void genMesh(const CBlockInfo& blocks, CChunk* adjacent[6]); // Just generates data, doesn't call OpenGL
    void update(float dT);
    void render();

    // This should only be called by the main rendering thread
    void updateOpenGLState() {
        glBindBuffer(GL_ARRAY_BUFFER, bufferIDs[0]);
        glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(glm::vec3), &(vertices[0]), GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, bufferIDs[1]);
        glBufferData(GL_ARRAY_BUFFER, uvs.size()*sizeof(glm::vec2), &(uvs[0]), GL_DYNAMIC_DRAW);

        vtxCount = vertices.size();
    }

    const glm::vec3& getPosition() const {
        return position;
    }

private:
    SBlock chunkData[CHUNK_HEIGHT][CHUNK_DEPTH][CHUNK_WIDTH];
    glm::vec3 position;

    GLuint bufferIDs[2];
    size_t vtxCount;

    std::vector<glm::vec3> vertices;
    std::vector<glm::vec2> uvs;
    // std::vector<glm::vec3> normals;
};

#endif // C_CHUNK_H
