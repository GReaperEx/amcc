#include "CChunk.h"

#include <GL/glew.h>
#include <GL/gl.h>

void CChunk::genBlocks(const CBlockInfo& blocks, const CNoiseGenerator& noiseGen, CChunk* adjacent[6])
{
    // Simple generation, just for testing

    uint16_t stoneID = blocks.getBlockID("stone");
    uint16_t dirtID = blocks.getBlockID("dirt");
    uint16_t grassID = blocks.getBlockID("grass");

    for (int j = 0; j < CHUNK_DEPTH; ++j) {
        for (int k = 0; k < CHUNK_WIDTH; ++k) {
            float nx = (position.x + k)*0.02f - 0.5f;
            float nz = (position.z + j)*0.02f - 0.5f;
            float noise = noiseGen.noise(nx, 0.0f, nz)/2.0f + 0.5f;
            int maxHeight = 128 + noise*10;

            for (int i = 0; i < maxHeight - 5; ++i) {
                chunkData[i][j][k].id = stoneID;
            }
            for (int i = maxHeight - 5; i < maxHeight; ++i) {
                chunkData[i][j][k].id = dirtID;
            }
            chunkData[maxHeight][j][k].id = grassID;
        }
    }
}

void CChunk::genMesh(const CBlockInfo& blocks, CChunk* adjacent[6])
{
    vertices.clear();
    uvs.clear();
    normals.clear();

    for (int i = 0; i < CHUNK_HEIGHT; ++i) {
        for (int j = 0; j < CHUNK_DEPTH; ++j) {
            for (int k = 0; k < CHUNK_WIDTH; ++k) {
                if (chunkData[i][j][k].id == 0) {
                    continue;
                }
                glm::vec2 blockUVs[6][4];
                blocks.getBlockUVs(chunkData[i][j][k].id, blockUVs);

                if ((k == CHUNK_WIDTH-1 && adjacent[0] && adjacent[0]->chunkData[i][j][0].id == 0) ||
                    (k != CHUNK_WIDTH-1 && chunkData[i][j][k+1].id == 0)) {
                    // RIGHT
                    vertices.push_back(glm::vec3(k+1, i, j+1));
                    vertices.push_back(glm::vec3(k+1, i, j));
                    vertices.push_back(glm::vec3(k+1, i+1, j));
                    uvs.push_back(blockUVs[CBlockInfo::RIGHT][0]);
                    uvs.push_back(blockUVs[CBlockInfo::RIGHT][1]);
                    uvs.push_back(blockUVs[CBlockInfo::RIGHT][2]);
                    normals.push_back(glm::vec3(1, 0, 0));
                    normals.push_back(glm::vec3(1, 0, 0));
                    normals.push_back(glm::vec3(1, 0, 0));

                    vertices.push_back(glm::vec3(k+1, i, j+1));
                    vertices.push_back(glm::vec3(k+1, i+1, j));
                    vertices.push_back(glm::vec3(k+1, i+1, j+1));
                    uvs.push_back(blockUVs[CBlockInfo::RIGHT][0]);
                    uvs.push_back(blockUVs[CBlockInfo::RIGHT][2]);
                    uvs.push_back(blockUVs[CBlockInfo::RIGHT][3]);
                    normals.push_back(glm::vec3(1, 0, 0));
                    normals.push_back(glm::vec3(1, 0, 0));
                    normals.push_back(glm::vec3(1, 0, 0));
                }
                if ((k == 0 && adjacent[1] && adjacent[1]->chunkData[i][j][CHUNK_WIDTH-1].id == 0) ||
                    (k != 0 && chunkData[i][j][k-1].id == 0)) {
                    // LEFT
                    vertices.push_back(glm::vec3(k, i, j));
                    vertices.push_back(glm::vec3(k, i, j+1));
                    vertices.push_back(glm::vec3(k, i+1, j+1));
                    uvs.push_back(blockUVs[CBlockInfo::LEFT][0]);
                    uvs.push_back(blockUVs[CBlockInfo::LEFT][1]);
                    uvs.push_back(blockUVs[CBlockInfo::LEFT][2]);
                    normals.push_back(glm::vec3(-1, 0, 0));
                    normals.push_back(glm::vec3(-1, 0, 0));
                    normals.push_back(glm::vec3(-1, 0, 0));

                    vertices.push_back(glm::vec3(k, i, j));
                    vertices.push_back(glm::vec3(k, i+1, j+1));
                    vertices.push_back(glm::vec3(k, i+1, j));
                    uvs.push_back(blockUVs[CBlockInfo::LEFT][0]);
                    uvs.push_back(blockUVs[CBlockInfo::LEFT][2]);
                    uvs.push_back(blockUVs[CBlockInfo::LEFT][3]);
                    normals.push_back(glm::vec3(-1, 0, 0));
                    normals.push_back(glm::vec3(-1, 0, 0));
                    normals.push_back(glm::vec3(-1, 0, 0));
                }
                if ((i == CHUNK_HEIGHT-1 && adjacent[2] && adjacent[2]->chunkData[0][j][k].id == 0) ||
                    (i != CHUNK_HEIGHT-1 && chunkData[i+1][j][k].id == 0)) {
                    // TOP
                    vertices.push_back(glm::vec3(k, i+1, j+1));
                    vertices.push_back(glm::vec3(k+1, i+1, j+1));
                    vertices.push_back(glm::vec3(k+1, i+1, j));
                    uvs.push_back(blockUVs[CBlockInfo::TOP][0]);
                    uvs.push_back(blockUVs[CBlockInfo::TOP][1]);
                    uvs.push_back(blockUVs[CBlockInfo::TOP][2]);
                    normals.push_back(glm::vec3(0, 1, 0));
                    normals.push_back(glm::vec3(0, 1, 0));
                    normals.push_back(glm::vec3(0, 1, 0));

                    vertices.push_back(glm::vec3(k, i+1, j+1));
                    vertices.push_back(glm::vec3(k+1, i+1, j));
                    vertices.push_back(glm::vec3(k, i+1, j));
                    uvs.push_back(blockUVs[CBlockInfo::TOP][0]);
                    uvs.push_back(blockUVs[CBlockInfo::TOP][2]);
                    uvs.push_back(blockUVs[CBlockInfo::TOP][3]);
                    normals.push_back(glm::vec3(0, 1, 0));
                    normals.push_back(glm::vec3(0, 1, 0));
                    normals.push_back(glm::vec3(0, 1, 0));
                }
                if ((i == 0 && adjacent[3] && adjacent[3]->chunkData[CHUNK_HEIGHT-1][j][k].id == 0) ||
                    (i != 0 && chunkData[i-1][j][k].id == 0)) {
                    // BOTTOM
                    vertices.push_back(glm::vec3(k, i, j));
                    vertices.push_back(glm::vec3(k+1, i, j));
                    vertices.push_back(glm::vec3(k+1, i, j+1));
                    uvs.push_back(blockUVs[CBlockInfo::BOTTOM][0]);
                    uvs.push_back(blockUVs[CBlockInfo::BOTTOM][1]);
                    uvs.push_back(blockUVs[CBlockInfo::BOTTOM][2]);
                    normals.push_back(glm::vec3(0, -1, 0));
                    normals.push_back(glm::vec3(0, -1, 0));
                    normals.push_back(glm::vec3(0, -1, 0));

                    vertices.push_back(glm::vec3(k, i, j));
                    vertices.push_back(glm::vec3(k+1, i, j+1));
                    vertices.push_back(glm::vec3(k, i, j+1));
                    uvs.push_back(blockUVs[CBlockInfo::BOTTOM][0]);
                    uvs.push_back(blockUVs[CBlockInfo::BOTTOM][2]);
                    uvs.push_back(blockUVs[CBlockInfo::BOTTOM][3]);
                    normals.push_back(glm::vec3(0, -1, 0));
                    normals.push_back(glm::vec3(0, -1, 0));
                    normals.push_back(glm::vec3(0, -1, 0));
                }
                if ((j == CHUNK_DEPTH-1 && adjacent[4] && adjacent[4]->chunkData[i][0][k].id == 0) ||
                    (j != CHUNK_DEPTH-1 && chunkData[i][j+1][k].id == 0)) {
                    // BACK
                    vertices.push_back(glm::vec3(k, i, j+1));
                    vertices.push_back(glm::vec3(k+1, i, j+1));
                    vertices.push_back(glm::vec3(k+1, i+1, j+1));
                    uvs.push_back(blockUVs[CBlockInfo::BACK][0]);
                    uvs.push_back(blockUVs[CBlockInfo::BACK][1]);
                    uvs.push_back(blockUVs[CBlockInfo::BACK][2]);
                    normals.push_back(glm::vec3(0, 0, 1));
                    normals.push_back(glm::vec3(0, 0, 1));
                    normals.push_back(glm::vec3(0, 0, 1));

                    vertices.push_back(glm::vec3(k, i, j+1));
                    vertices.push_back(glm::vec3(k+1, i+1, j+1));
                    vertices.push_back(glm::vec3(k, i+1, j+1));
                    uvs.push_back(blockUVs[CBlockInfo::BACK][0]);
                    uvs.push_back(blockUVs[CBlockInfo::BACK][2]);
                    uvs.push_back(blockUVs[CBlockInfo::BACK][3]);
                    normals.push_back(glm::vec3(0, 0, 1));
                    normals.push_back(glm::vec3(0, 0, 1));
                    normals.push_back(glm::vec3(0, 0, 1));
                }
                if ((j == 0 && adjacent[5] && adjacent[5]->chunkData[i][CHUNK_DEPTH-1][k].id == 0) ||
                    (j != 0 && chunkData[i][j-1][k].id == 0)) {
                    // FRONT
                    vertices.push_back(glm::vec3(k+1, i, j));
                    vertices.push_back(glm::vec3(k, i, j));
                    vertices.push_back(glm::vec3(k, i+1, j));
                    uvs.push_back(blockUVs[CBlockInfo::FRONT][0]);
                    uvs.push_back(blockUVs[CBlockInfo::FRONT][1]);
                    uvs.push_back(blockUVs[CBlockInfo::FRONT][2]);
                    normals.push_back(glm::vec3(0, 0, -1));
                    normals.push_back(glm::vec3(0, 0, -1));
                    normals.push_back(glm::vec3(0, 0, -1));

                    vertices.push_back(glm::vec3(k+1, i, j));
                    vertices.push_back(glm::vec3(k, i+1, j));
                    vertices.push_back(glm::vec3(k+1, i+1, j));
                    uvs.push_back(blockUVs[CBlockInfo::FRONT][0]);
                    uvs.push_back(blockUVs[CBlockInfo::FRONT][2]);
                    uvs.push_back(blockUVs[CBlockInfo::FRONT][3]);
                    normals.push_back(glm::vec3(0, 0, -1));
                    normals.push_back(glm::vec3(0, 0, -1));
                    normals.push_back(glm::vec3(0, 0, -1));
                }

            }
        }
    }
}

void CChunk::update(float dT)
{
}

void CChunk::render()
{
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, bufferIDs[0]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, bufferIDs[1]);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, bufferIDs[2]);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    glDrawArrays(GL_TRIANGLES, 0, vtxCount);

    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(0);
}
