#include "Block.h"

void Block::getMeshData(const glm::vec3& localPos, uint16_t metas[7], const Block* sides[6], std::vector<glm::vec3>& vertices, std::vector<glm::vec2>& uvs, std::vector<float>& lighting) const
{
    int i = localPos.x;
    int k = localPos.y;
    int j = localPos.z;

    switch (shape)
    {
    case Shape::CUBE:
        if (sides[0] && sides[0]->isTransparent()) {
            int sunLightLevel = ((metas[0 + 1] >> 4) & 0x0F) - 6;
            int lightLevel = glm::max(metas[0 + 1] & 0x0F, sunLightLevel);
            // RIGHT
            vertices.push_back(glm::vec3(i+1, k, j+1));
            vertices.push_back(glm::vec3(i+1, k, j));
            vertices.push_back(glm::vec3(i+1, k+1, j));
            uvs.push_back(UVs[0][0]);
            uvs.push_back(UVs[0][1]);
            uvs.push_back(UVs[0][2]);
            lighting.push_back(lightLevel);
            lighting.push_back(lightLevel);
            lighting.push_back(lightLevel);

            vertices.push_back(glm::vec3(i+1, k, j+1));
            vertices.push_back(glm::vec3(i+1, k+1, j));
            vertices.push_back(glm::vec3(i+1, k+1, j+1));
            uvs.push_back(UVs[0][0]);
            uvs.push_back(UVs[0][2]);
            uvs.push_back(UVs[0][3]);
            lighting.push_back(lightLevel);
            lighting.push_back(lightLevel);
            lighting.push_back(lightLevel);
        }
        if (sides[1] && sides[1]->isTransparent()) {
            int sunLightLevel = ((metas[1 + 1] >> 4) & 0x0F) - 6;
            int lightLevel = glm::max(metas[1 + 1] & 0x0F, sunLightLevel);
            // LEFT
            vertices.push_back(glm::vec3(i, k, j));
            vertices.push_back(glm::vec3(i, k, j+1));
            vertices.push_back(glm::vec3(i, k+1, j+1));
            uvs.push_back(UVs[1][0]);
            uvs.push_back(UVs[1][1]);
            uvs.push_back(UVs[1][2]);
            lighting.push_back(lightLevel);
            lighting.push_back(lightLevel);
            lighting.push_back(lightLevel);

            vertices.push_back(glm::vec3(i, k, j));
            vertices.push_back(glm::vec3(i, k+1, j+1));
            vertices.push_back(glm::vec3(i, k+1, j));
            uvs.push_back(UVs[1][0]);
            uvs.push_back(UVs[1][2]);
            uvs.push_back(UVs[1][3]);
            lighting.push_back(lightLevel);
            lighting.push_back(lightLevel);
            lighting.push_back(lightLevel);
        }
        if (sides[2] && sides[2]->isTransparent()) {
            int lightLevel = glm::max(metas[2 + 1] & 0x0F, (metas[2 + 1] >> 4) & 0x0F);
            // TOP
            vertices.push_back(glm::vec3(i, k+1, j+1));
            vertices.push_back(glm::vec3(i+1, k+1, j+1));
            vertices.push_back(glm::vec3(i+1, k+1, j));
            uvs.push_back(UVs[2][0]);
            uvs.push_back(UVs[2][1]);
            uvs.push_back(UVs[2][2]);
            lighting.push_back(lightLevel);
            lighting.push_back(lightLevel);
            lighting.push_back(lightLevel);

            vertices.push_back(glm::vec3(i, k+1, j+1));
            vertices.push_back(glm::vec3(i+1, k+1, j));
            vertices.push_back(glm::vec3(i, k+1, j));
            uvs.push_back(UVs[2][0]);
            uvs.push_back(UVs[2][2]);
            uvs.push_back(UVs[2][3]);
            lighting.push_back(lightLevel);
            lighting.push_back(lightLevel);
            lighting.push_back(lightLevel);
        }
        if (sides[3] && sides[3]->isTransparent()) {
            int lightLevel = glm::max(metas[3 + 1] & 0x0F, (metas[3 + 1] >> 4) & 0x0F);
            // BOTTOM
            vertices.push_back(glm::vec3(i, k, j));
            vertices.push_back(glm::vec3(i+1, k, j));
            vertices.push_back(glm::vec3(i+1, k, j+1));
            uvs.push_back(UVs[3][0]);
            uvs.push_back(UVs[3][1]);
            uvs.push_back(UVs[3][2]);
            lighting.push_back(lightLevel);
            lighting.push_back(lightLevel);
            lighting.push_back(lightLevel);

            vertices.push_back(glm::vec3(i, k, j));
            vertices.push_back(glm::vec3(i+1, k, j+1));
            vertices.push_back(glm::vec3(i, k, j+1));
            uvs.push_back(UVs[3][0]);
            uvs.push_back(UVs[3][2]);
            uvs.push_back(UVs[3][3]);
            lighting.push_back(lightLevel);
            lighting.push_back(lightLevel);
            lighting.push_back(lightLevel);
        }
        if (sides[4] && sides[4]->isTransparent()) {
            int sunLightLevel = ((metas[4 + 1] >> 4) & 0x0F) - 6;
            int lightLevel = glm::max(metas[4 + 1] & 0x0F, sunLightLevel);
            // BACK
            vertices.push_back(glm::vec3(i, k, j+1));
            vertices.push_back(glm::vec3(i+1, k, j+1));
            vertices.push_back(glm::vec3(i+1, k+1, j+1));
            uvs.push_back(UVs[4][0]);
            uvs.push_back(UVs[4][1]);
            uvs.push_back(UVs[4][2]);
            lighting.push_back(lightLevel);
            lighting.push_back(lightLevel);
            lighting.push_back(lightLevel);

            vertices.push_back(glm::vec3(i, k, j+1));
            vertices.push_back(glm::vec3(i+1, k+1, j+1));
            vertices.push_back(glm::vec3(i, k+1, j+1));
            uvs.push_back(UVs[4][0]);
            uvs.push_back(UVs[4][2]);
            uvs.push_back(UVs[4][3]);
            lighting.push_back(lightLevel);
            lighting.push_back(lightLevel);
            lighting.push_back(lightLevel);
        }
        if (sides[5] && sides[5]->isTransparent()) {
            int sunLightLevel = ((metas[5 + 1] >> 4) & 0x0F) - 6;
            int lightLevel = glm::max(metas[5 + 1] & 0x0F, sunLightLevel);
            // FRONT
            vertices.push_back(glm::vec3(i+1, k, j));
            vertices.push_back(glm::vec3(i, k, j));
            vertices.push_back(glm::vec3(i, k+1, j));
            uvs.push_back(UVs[5][0]);
            uvs.push_back(UVs[5][1]);
            uvs.push_back(UVs[5][2]);
            lighting.push_back(lightLevel);
            lighting.push_back(lightLevel);
            lighting.push_back(lightLevel);

            vertices.push_back(glm::vec3(i+1, k, j));
            vertices.push_back(glm::vec3(i, k+1, j));
            vertices.push_back(glm::vec3(i+1, k+1, j));
            uvs.push_back(UVs[5][0]);
            uvs.push_back(UVs[5][2]);
            uvs.push_back(UVs[5][3]);
            lighting.push_back(lightLevel);
            lighting.push_back(lightLevel);
            lighting.push_back(lightLevel);
        }
    break;
    case Shape::FAKE3D:
    {
        int sunLightLevel = ((metas[0] >> 4) & 0x0F) - 6;
        int lightLevel = glm::max(metas[0] & 0x0F, sunLightLevel);
        // South-East side

        vertices.push_back(glm::vec3(i, k, j+1));
        vertices.push_back(glm::vec3(i+1, k, j));
        vertices.push_back(glm::vec3(i+1, k+1, j));
        uvs.push_back(UVs[0][0]);
        uvs.push_back(UVs[0][1]);
        uvs.push_back(UVs[0][2]);
        lighting.push_back(lightLevel);
        lighting.push_back(lightLevel);
        lighting.push_back(lightLevel);

        vertices.push_back(glm::vec3(i, k, j+1));
        vertices.push_back(glm::vec3(i+1, k+1, j));
        vertices.push_back(glm::vec3(i, k+1, j+1));
        uvs.push_back(UVs[0][0]);
        uvs.push_back(UVs[0][2]);
        uvs.push_back(UVs[0][3]);
        lighting.push_back(lightLevel);
        lighting.push_back(lightLevel);
        lighting.push_back(lightLevel);

        // North-West side

        vertices.push_back(glm::vec3(i+1, k, j));
        vertices.push_back(glm::vec3(i, k, j+1));
        vertices.push_back(glm::vec3(i, k+1, j+1));
        uvs.push_back(UVs[0][1]);
        uvs.push_back(UVs[0][0]);
        uvs.push_back(UVs[0][3]);
        lighting.push_back(lightLevel);
        lighting.push_back(lightLevel);
        lighting.push_back(lightLevel);

        vertices.push_back(glm::vec3(i+1, k, j));
        vertices.push_back(glm::vec3(i, k+1, j+1));
        vertices.push_back(glm::vec3(i+1, k+1, j));
        uvs.push_back(UVs[0][1]);
        uvs.push_back(UVs[0][3]);
        uvs.push_back(UVs[0][2]);
        lighting.push_back(lightLevel);
        lighting.push_back(lightLevel);
        lighting.push_back(lightLevel);


        // North-East side

        vertices.push_back(glm::vec3(i+1, k, j+1));
        vertices.push_back(glm::vec3(i, k, j));
        vertices.push_back(glm::vec3(i, k+1, j));
        uvs.push_back(UVs[0][0]);
        uvs.push_back(UVs[0][1]);
        uvs.push_back(UVs[0][2]);
        lighting.push_back(lightLevel);
        lighting.push_back(lightLevel);
        lighting.push_back(lightLevel);

        vertices.push_back(glm::vec3(i+1, k, j+1));
        vertices.push_back(glm::vec3(i, k+1, j));
        vertices.push_back(glm::vec3(i+1, k+1, j+1));
        uvs.push_back(UVs[0][0]);
        uvs.push_back(UVs[0][2]);
        uvs.push_back(UVs[0][3]);
        lighting.push_back(lightLevel);
        lighting.push_back(lightLevel);
        lighting.push_back(lightLevel);

        // South-West side

        vertices.push_back(glm::vec3(i, k, j));
        vertices.push_back(glm::vec3(i+1, k, j+1));
        vertices.push_back(glm::vec3(i+1, k+1, j+1));
        uvs.push_back(UVs[0][1]);
        uvs.push_back(UVs[0][0]);
        uvs.push_back(UVs[0][3]);
        lighting.push_back(lightLevel);
        lighting.push_back(lightLevel);
        lighting.push_back(lightLevel);

        vertices.push_back(glm::vec3(i, k, j));
        vertices.push_back(glm::vec3(i+1, k+1, j+1));
        vertices.push_back(glm::vec3(i, k+1, j));
        uvs.push_back(UVs[0][1]);
        uvs.push_back(UVs[0][3]);
        uvs.push_back(UVs[0][2]);
        lighting.push_back(lightLevel);
        lighting.push_back(lightLevel);
        lighting.push_back(lightLevel);
    }
    break;
    }
}
