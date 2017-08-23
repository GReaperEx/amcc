#include "Chunk.h"

#include <GL/glew.h>
#include <GL/gl.h>

void Chunk::genBlocks(const BiomeManager& biomeManager, const BlockManager& blocks, const std::vector<NoiseGenerator>& noiseGens, Chunk* adjacent[6], std::vector<StructToGenerate>& genStructs)
{
    // What should the smoothing distance be? That is the question.
    // 1 : 3x3
    // 2 : 5x5
    // 3 : 7x7
    // 4 : 9x9
    // etc...
    const int dist = 4;

    for (int i = 0; i < CHUNK_WIDTH; ++i) {
        for (int j = 0; j < CHUNK_DEPTH; ++j) {
            // Smoothing the edges between biomes

            glm::vec3 localPos = position + glm::vec3(i, 0, j);
            float biomeNoises[5] = {
                noiseGens[0].noise((localPos.x-dist)*0.001f, 0.0f, (localPos.z-dist)*0.001f),
                noiseGens[0].noise((localPos.x+dist)*0.001f, 0.0f, (localPos.z-dist)*0.001f),
                noiseGens[0].noise((localPos.x+dist)*0.001f, 0.0f, (localPos.z+dist)*0.001f),
                noiseGens[0].noise((localPos.x-dist)*0.001f, 0.0f, (localPos.z+dist)*0.001f),
                noiseGens[0].noise(localPos.x*0.001f, 0.0f, localPos.z*0.001f),
            };

            int biomeAvg;

            // Bailing early, checking only the edges
            if (biomeManager.getBiome(biomeNoises[0]).getName() != biomeManager.getBiome(biomeNoises[1]).getName() ||
                biomeManager.getBiome(biomeNoises[0]).getName() != biomeManager.getBiome(biomeNoises[2]).getName() ||
                biomeManager.getBiome(biomeNoises[0]).getName() != biomeManager.getBiome(biomeNoises[3]).getName()) {

                float heightSum = 0.0f;
                for (int i2 = -dist; i2 <= dist; ++i2) {
                    int lineSum = 0;
                    for (int j2 = -dist; j2 <= dist; ++j2) {
                        float curBiomeNoise = noiseGens[0].noise((localPos.x+i2)*0.001f, 0.0f, (localPos.z+j2)*0.001f);
                        lineSum += biomeManager.getBiome(curBiomeNoise).calcSurfaceHeight(localPos.x+i2, localPos.z+j2, noiseGens[1]);
                    }
                    heightSum += lineSum/(float)(dist*2+1);
                }

                biomeAvg = (int)(heightSum/(dist*2+1));
            } else {
                biomeAvg = biomeManager.getBiome(biomeNoises[4]).calcSurfaceHeight(localPos.x, localPos.z, noiseGens[1]);
            }

            std::string genStruct = biomeManager.getBiome(biomeNoises[4]).genChunkColumn(chunkData[i][j], blocks, biomeAvg, localPos.x, localPos.z, noiseGens[1]);
            if (!genStruct.empty()) {
                genStructs.push_back(StructToGenerate{localPos + glm::vec3(0.0f, biomeAvg+1, 0.0f), genStruct});
            }
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

void Chunk::genMesh(const BlockManager& blocks, Chunk* adjacent[6])
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

                const Block *sides[6] = { nullptr };
                if (i == CHUNK_WIDTH - 1 && adjacent[0]) {
                    sides[0] = &blocks.getBlock(adjacent[0]->chunkData[0][j][k].id);
                } else if (i != CHUNK_WIDTH - 1) {
                    sides[0] = &blocks.getBlock(chunkData[i+1][j][k].id);
                }
                if (i == 0 && adjacent[1]) {
                    sides[1] = &blocks.getBlock(adjacent[1]->chunkData[CHUNK_WIDTH-1][j][k].id);
                } else if (i != 0) {
                    sides[1] = &blocks.getBlock(chunkData[i-1][j][k].id);
                }

                if (k == CHUNK_HEIGHT - 1 && adjacent[2]) {
                    sides[2] = &blocks.getBlock(adjacent[2]->chunkData[i][j][0].id);
                } else if (k != CHUNK_HEIGHT - 1) {
                    sides[2] = &blocks.getBlock(chunkData[i][j][k+1].id);
                }
                if (k == 0 && adjacent[3]) {
                    sides[3] = &blocks.getBlock(adjacent[3]->chunkData[i][j][CHUNK_HEIGHT-1].id);
                } else if (k != 0) {
                    sides[3] = &blocks.getBlock(chunkData[i][j][k-1].id);
                }

                if (j == CHUNK_DEPTH - 1 && adjacent[4]) {
                    sides[4] = &blocks.getBlock(adjacent[4]->chunkData[i][0][k].id);
                } else if (j != CHUNK_DEPTH - 1) {
                    sides[4] = &blocks.getBlock(chunkData[i][j+1][k].id);
                }
                if (j == 0 && adjacent[5]) {
                    sides[5] = &blocks.getBlock(adjacent[5]->chunkData[i][CHUNK_DEPTH-1][k].id);
                } else if (j != 0) {
                    sides[5] = &blocks.getBlock(chunkData[i][j-1][k].id);
                }
                blocks.getBlock(chunkData[i][j][k].id).getMeshData(glm::vec3(i, k, j), sides, vertices, uvs, normals);
            }
        }
    }
    neededMeshUpdate = false;
    neededStateUpdate = true;
}

void Chunk::update(float dT)
{
}

void Chunk::render()
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

void Chunk::replaceBlock(const BlockDetails& newBlock, Chunk *adjacent[6])
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

bool Chunk::traceRayToBlock(BlockDetails& lookBlock, const glm::vec3& rayOrigin,
                             const glm::vec3& rayDir, const BlockManager& blockManager, bool ignoreAir)
{
    glm::vec3 chunkBoxMin = position;
    glm::vec3 chunkBoxMax = position + glm::vec3((float)CHUNK_WIDTH, (float)CHUNK_HEIGHT, (float)CHUNK_DEPTH);
    glm::vec3 rayDir_inverted = glm::vec3(1.0f/rayDir.x, 1.0f/rayDir.y, 1.0f/rayDir.z);

    if (rayBlockIntersection(lookBlock.position, chunkBoxMin, chunkBoxMax, rayOrigin, rayDir, rayDir_inverted, ignoreAir)) {
        glm::vec3 localPos = lookBlock.position - position;
        lookBlock.id = chunkData[(int)glm::floor(localPos.x)][(int)glm::floor(localPos.z)][(int)glm::floor(localPos.y)].id;
        lookBlock.name = blockManager.getBlock(lookBlock.id).getName();
        return true;
    }
    return false;
}

bool Chunk::rayBlockIntersection(glm::vec3& lookBlock, const glm::vec3& boxMin, const glm::vec3& boxMax,
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

bool Chunk::rayBoxIntersection(const glm::vec3& boxMin, const glm::vec3& boxMax,
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
