#include "CChunk.h"

#include <GL/glew.h>
#include <GL/gl.h>

void CChunk::genBlocks(const CBiomeManager& biomeManager, const CBlockInfo& blocks, const std::vector<CNoiseGenerator>& noiseGens, CChunk* adjacent[6])
{

    for (int i = 0; i < CHUNK_WIDTH; ++i) {
        for (int j = 0; j < CHUNK_DEPTH; ++j) {
            glm::vec3 localPos = position + glm::vec3(i, 0, j);
            biomeManager.getBiome(noiseGens[0].noise(localPos.x*0.001f, 0.0f, localPos.z*0.001f)).genChunkColumn(chunkData[i][j], localPos.x, localPos.z, blocks, noiseGens[1]);
        }
    }

    isGenerated = true;
    neededMeshUpdate = true;

    for (int i = 0; i < 6; i++) {
        if (adjacent[i]) {
            adjacent[i]->neededStateUpdate = false;
            adjacent[i]->neededMeshUpdate = true;
        }
    }
}

void CChunk::genMesh(const CBlockInfo& blocks, CChunk* adjacent[6])
{
    vertices.clear();
    uvs.clear();
    normals.clear();

    for (int i = 0; i < CHUNK_WIDTH; ++i) {
        for (int j = 0; j < CHUNK_DEPTH; ++j) {
            for (int k = 0; k < CHUNK_HEIGHT; ++k) {
                if (chunkData[i][j][k].id == 0) {
                    continue;
                }
                glm::vec2 blockUVs[6][4];
                blocks.getBlockUVs(chunkData[i][j][k].id, blockUVs);

                if ((i == CHUNK_WIDTH-1 && adjacent[0] && adjacent[0]->chunkData[0][j][k].id == 0) ||
                    (i != CHUNK_WIDTH-1 && chunkData[i+1][j][k].id == 0)) {
                    // RIGHT
                    vertices.push_back(glm::vec3(i+1, k, j+1));
                    vertices.push_back(glm::vec3(i+1, k, j));
                    vertices.push_back(glm::vec3(i+1, k+1, j));
                    uvs.push_back(blockUVs[CBlockInfo::RIGHT][0]);
                    uvs.push_back(blockUVs[CBlockInfo::RIGHT][1]);
                    uvs.push_back(blockUVs[CBlockInfo::RIGHT][2]);
                    normals.push_back(glm::vec3(1, 0, 0));
                    normals.push_back(glm::vec3(1, 0, 0));
                    normals.push_back(glm::vec3(1, 0, 0));

                    vertices.push_back(glm::vec3(i+1, k, j+1));
                    vertices.push_back(glm::vec3(i+1, k+1, j));
                    vertices.push_back(glm::vec3(i+1, k+1, j+1));
                    uvs.push_back(blockUVs[CBlockInfo::RIGHT][0]);
                    uvs.push_back(blockUVs[CBlockInfo::RIGHT][2]);
                    uvs.push_back(blockUVs[CBlockInfo::RIGHT][3]);
                    normals.push_back(glm::vec3(1, 0, 0));
                    normals.push_back(glm::vec3(1, 0, 0));
                    normals.push_back(glm::vec3(1, 0, 0));
                }
                if ((i == 0 && adjacent[1] && adjacent[1]->chunkData[CHUNK_WIDTH-1][j][k].id == 0) ||
                    (i != 0 && chunkData[i-1][j][k].id == 0)) {
                    // LEFT
                    vertices.push_back(glm::vec3(i, k, j));
                    vertices.push_back(glm::vec3(i, k, j+1));
                    vertices.push_back(glm::vec3(i, k+1, j+1));
                    uvs.push_back(blockUVs[CBlockInfo::LEFT][0]);
                    uvs.push_back(blockUVs[CBlockInfo::LEFT][1]);
                    uvs.push_back(blockUVs[CBlockInfo::LEFT][2]);
                    normals.push_back(glm::vec3(-1, 0, 0));
                    normals.push_back(glm::vec3(-1, 0, 0));
                    normals.push_back(glm::vec3(-1, 0, 0));

                    vertices.push_back(glm::vec3(i, k, j));
                    vertices.push_back(glm::vec3(i, k+1, j+1));
                    vertices.push_back(glm::vec3(i, k+1, j));
                    uvs.push_back(blockUVs[CBlockInfo::LEFT][0]);
                    uvs.push_back(blockUVs[CBlockInfo::LEFT][2]);
                    uvs.push_back(blockUVs[CBlockInfo::LEFT][3]);
                    normals.push_back(glm::vec3(-1, 0, 0));
                    normals.push_back(glm::vec3(-1, 0, 0));
                    normals.push_back(glm::vec3(-1, 0, 0));
                }
                if ((k == CHUNK_HEIGHT-1 && adjacent[2] && adjacent[2]->chunkData[i][j][0].id == 0) ||
                    (k != CHUNK_HEIGHT-1 && chunkData[i][j][k+1].id == 0)) {
                    // TOP
                    vertices.push_back(glm::vec3(i, k+1, j+1));
                    vertices.push_back(glm::vec3(i+1, k+1, j+1));
                    vertices.push_back(glm::vec3(i+1, k+1, j));
                    uvs.push_back(blockUVs[CBlockInfo::TOP][0]);
                    uvs.push_back(blockUVs[CBlockInfo::TOP][1]);
                    uvs.push_back(blockUVs[CBlockInfo::TOP][2]);
                    normals.push_back(glm::vec3(0, 1, 0));
                    normals.push_back(glm::vec3(0, 1, 0));
                    normals.push_back(glm::vec3(0, 1, 0));

                    vertices.push_back(glm::vec3(i, k+1, j+1));
                    vertices.push_back(glm::vec3(i+1, k+1, j));
                    vertices.push_back(glm::vec3(i, k+1, j));
                    uvs.push_back(blockUVs[CBlockInfo::TOP][0]);
                    uvs.push_back(blockUVs[CBlockInfo::TOP][2]);
                    uvs.push_back(blockUVs[CBlockInfo::TOP][3]);
                    normals.push_back(glm::vec3(0, 1, 0));
                    normals.push_back(glm::vec3(0, 1, 0));
                    normals.push_back(glm::vec3(0, 1, 0));
                }
                if ((k == 0 && adjacent[3] && adjacent[3]->chunkData[i][j][CHUNK_HEIGHT-1].id == 0) ||
                    (k != 0 && chunkData[i][j][k-1].id == 0)) {
                    // BOTTOM
                    vertices.push_back(glm::vec3(i, k, j));
                    vertices.push_back(glm::vec3(i+1, k, j));
                    vertices.push_back(glm::vec3(i+1, k, j+1));
                    uvs.push_back(blockUVs[CBlockInfo::BOTTOM][0]);
                    uvs.push_back(blockUVs[CBlockInfo::BOTTOM][1]);
                    uvs.push_back(blockUVs[CBlockInfo::BOTTOM][2]);
                    normals.push_back(glm::vec3(0, -1, 0));
                    normals.push_back(glm::vec3(0, -1, 0));
                    normals.push_back(glm::vec3(0, -1, 0));

                    vertices.push_back(glm::vec3(i, k, j));
                    vertices.push_back(glm::vec3(i+1, k, j+1));
                    vertices.push_back(glm::vec3(i, k, j+1));
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
                    vertices.push_back(glm::vec3(i, k, j+1));
                    vertices.push_back(glm::vec3(i+1, k, j+1));
                    vertices.push_back(glm::vec3(i+1, k+1, j+1));
                    uvs.push_back(blockUVs[CBlockInfo::BACK][0]);
                    uvs.push_back(blockUVs[CBlockInfo::BACK][1]);
                    uvs.push_back(blockUVs[CBlockInfo::BACK][2]);
                    normals.push_back(glm::vec3(0, 0, 1));
                    normals.push_back(glm::vec3(0, 0, 1));
                    normals.push_back(glm::vec3(0, 0, 1));

                    vertices.push_back(glm::vec3(i, k, j+1));
                    vertices.push_back(glm::vec3(i+1, k+1, j+1));
                    vertices.push_back(glm::vec3(i, k+1, j+1));
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
                    vertices.push_back(glm::vec3(i+1, k, j));
                    vertices.push_back(glm::vec3(i, k, j));
                    vertices.push_back(glm::vec3(i, k+1, j));
                    uvs.push_back(blockUVs[CBlockInfo::FRONT][0]);
                    uvs.push_back(blockUVs[CBlockInfo::FRONT][1]);
                    uvs.push_back(blockUVs[CBlockInfo::FRONT][2]);
                    normals.push_back(glm::vec3(0, 0, -1));
                    normals.push_back(glm::vec3(0, 0, -1));
                    normals.push_back(glm::vec3(0, 0, -1));

                    vertices.push_back(glm::vec3(i+1, k, j));
                    vertices.push_back(glm::vec3(i, k+1, j));
                    vertices.push_back(glm::vec3(i+1, k+1, j));
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

    chunkData[localX][localZ][localY].id = newBlock.id;

    neededMeshUpdate = true;
    if (adjacent[0] && localX == CHUNK_WIDTH-1) {
        adjacent[0]->neededStateUpdate = false;
        adjacent[0]->neededMeshUpdate = true;
    } else if (adjacent[1] && localX == 0) {
        adjacent[1]->neededStateUpdate = false;
        adjacent[1]->neededMeshUpdate = true;
    }
    if (adjacent[2] && localY == CHUNK_HEIGHT-1) {
        adjacent[2]->neededStateUpdate = false;
        adjacent[2]->neededMeshUpdate = true;
    } else if (adjacent[3] && localY == 0) {
        adjacent[3]->neededStateUpdate = false;
        adjacent[3]->neededMeshUpdate = true;
    }
    if (adjacent[4] && localZ == CHUNK_DEPTH-1) {
        adjacent[4]->neededStateUpdate = false;
        adjacent[4]->neededMeshUpdate = true;
    } else if (adjacent[5] && localZ == 0) {
        adjacent[5]->neededStateUpdate = false;
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
        lookBlock.id = chunkData[(int)glm::floor(localPos.x)][(int)glm::floor(localPos.z)][(int)glm::floor(localPos.y)].id;
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
            uint16_t itsID = chunkData[(int)localPos.x][(int)localPos.z][(int)localPos.y].id;

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
