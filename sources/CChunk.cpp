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
            float nx = (position.x + k);
            float nz = (position.z + j);
            float noise = noiseGen.noise(nx*0.002f, 0.0f, nz*0.002f) +
                          noiseGen.noise(nx*0.003f, 0.0f, nz*0.003f) +
                          noiseGen.noise(nx*0.005f, 0.0f, nz*0.005f) +
                          noiseGen.noise(nx*0.007f, 0.0f, nz*0.007f) +
                          noiseGen.noise(nx*0.011f, 0.0f, nz*0.011f) +
                          noiseGen.noise(nx*0.013f, 0.0f, nz*0.013f) +
                          noiseGen.noise(nx*0.017f, 0.0f, nz*0.017f) +
                          noiseGen.noise(nx*0.023f, 0.0f, nz*0.023f);

            int maxHeight = glm::clamp(128 + noise*10, 0.0f, (float)CHUNK_HEIGHT - 1);

            for (int i = 0; i < std::max(maxHeight - 5, 0); ++i) {
                chunkData[i][j][k].id = stoneID;
            }
            for (int i = std::max(maxHeight - 5, 0); i < maxHeight; ++i) {
                chunkData[i][j][k].id = dirtID;
            }
            chunkData[maxHeight][j][k].id = grassID;
        }
    }
    isGenerated = true;
    neededMeshUpdate = true;

    for (int i = 0; i < 6; i++) {
        if (adjacent[i]) {
            adjacent[i]->neededMeshUpdate = true;
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
    neededMeshUpdate = false;
    neededStateUpdate = true;
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

void CChunk::replaceBlock(const BlockDetails& newBlock, CChunk *adjacent[6])
{
    int localX = (int)(newBlock.position.x - position.x);
    int localY = (int)(newBlock.position.y - position.y);
    int localZ = (int)(newBlock.position.z - position.z);

    chunkData[localY][localZ][localX].id = newBlock.id;

    neededMeshUpdate = true;
    if (adjacent[0] && localX == CHUNK_WIDTH-1) {
        adjacent[0]->neededMeshUpdate = true;
    } else if (adjacent[1] && localX == 0) {
        adjacent[1]->neededMeshUpdate = true;
    }
    if (adjacent[2] && localY == CHUNK_HEIGHT-1) {
        adjacent[2]->neededMeshUpdate = true;
    } else if (adjacent[3] && localY == 0) {
        adjacent[3]->neededMeshUpdate = true;
    }
    if (adjacent[4] && localZ == CHUNK_DEPTH-1) {
        adjacent[4]->neededMeshUpdate = true;
    } else if (adjacent[5] && localZ == 0) {
        adjacent[5]->neededMeshUpdate = true;
    }
}

bool CChunk::traceRayToBlock(BlockDetails& lookBlock, const glm::vec3& rayOrigin,
                             const glm::vec3& rayDir, const CBlockInfo& blockInfo, bool ignoreAir)
{
    glm::vec3 chunkBoxMin = position;
    glm::vec3 chunkBoxMax = position + glm::vec3((float)CHUNK_WIDTH, (float)CHUNK_HEIGHT, (float)CHUNK_DEPTH);
    glm::vec3 rayDir_inverted = glm::vec3(1.0f/rayDir.x, 1.0f/rayDir.y, 1.0f/rayDir.z);

    if (rayBlockIntersection(lookBlock.position, chunkBoxMin, chunkBoxMax, rayOrigin, rayDir, rayDir_inverted, ignoreAir)) {
        glm::vec3 localPos = lookBlock.position - position;
        lookBlock.id = chunkData[(int)glm::floor(localPos.y)][(int)glm::floor(localPos.z)][(int)glm::floor(localPos.x)].id;
        lookBlock.name = blockInfo.getBlockName(lookBlock.id);
        return true;
    }
    return false;
}

bool CChunk::rayBlockIntersection(glm::vec3& lookBlock, const glm::vec3& boxMin, const glm::vec3& boxMax,
                                  const glm::vec3& rayPos, const glm::vec3& rayDir,
                                  const glm::vec3& rayDir_inverted, bool ignoreAir)
{
    glm::vec3 closest = rayPos + rayDir*10240.0f;
    glm::vec3 dims = boxMax - boxMin;

    if (rayBoxIntersection(boxMin, boxMax, rayPos, rayDir_inverted)) {
        if (dims.x >= 2 || dims.y >= 2 || dims.z >= 2) {
            for (int i = 0; i < 2; ++i) {
                for (int j = 0; j < 2; ++j) {
                    for (int k = 0; k < 2; ++k) {
                        glm::vec3 localMin = boxMin + glm::vec3(i, j, k)*dims*0.5f;
                        glm::vec3 localMax = localMin + dims*0.5f;
                        glm::vec3 temp;
                        if (rayBlockIntersection(temp, localMin, localMax, rayPos, rayDir, rayDir_inverted, ignoreAir)) {
                            float dist1 = glm::dot(closest-rayPos, closest-rayPos);
                            float dist2 = glm::dot(temp-rayPos, temp-rayPos);
                            if (dist2 < dist1) {
                                closest = temp;
                            }
                        }
                    }
                }
            }
        } else {
            lookBlock = glm::floor(boxMin);
            glm::vec3 localPos = lookBlock - position;
            uint16_t itsID = chunkData[(int)localPos.y][(int)localPos.z][(int)localPos.x].id;

            if ((ignoreAir && itsID != 0) || !ignoreAir) {
                return true;
            } else {
                return false;
            }
        }

        lookBlock = closest;
        return closest != rayPos + rayDir*10240.0f;
    }
    return false;
}

bool CChunk::rayBoxIntersection(const glm::vec3& boxMin, const glm::vec3& boxMax,
                                const glm::vec3& rayPos, const glm::vec3& rayDir_inverted)
{
    using glm::min;
    using glm::max;

    glm::vec3 minCheck = (boxMin - rayPos)*rayDir_inverted;
    glm::vec3 maxCheck = (boxMax - rayPos)*rayDir_inverted;

    float tmin = max(max(min(minCheck.x, maxCheck.x), min(minCheck.y, maxCheck.y)), min(minCheck.z, maxCheck.z));
    float tmax = min(min(max(minCheck.x, maxCheck.x), max(minCheck.y, maxCheck.y)), max(minCheck.z, maxCheck.z));

    return tmax >= tmin && tmax >= 0;
}
