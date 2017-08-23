#include "Block.h"

void Block::getMeshData(const glm::vec3& localPos, const Block* sides[6], std::vector<glm::vec3>& vertices, std::vector<glm::vec2>& uvs, std::vector<glm::vec3>& normals) const
{
    int i = localPos.x;
    int k = localPos.y;
    int j = localPos.z;

    switch (shape)
    {
    case Shape::CUBE:
        if (sides[0] && sides[0]->isTransparent()) {
            // RIGHT
            vertices.push_back(glm::vec3(i+1, k, j+1));
            vertices.push_back(glm::vec3(i+1, k, j));
            vertices.push_back(glm::vec3(i+1, k+1, j));
            uvs.push_back(UVs[0][0]);
            uvs.push_back(UVs[0][1]);
            uvs.push_back(UVs[0][2]);
            normals.push_back(glm::vec3(1, 0, 0));
            normals.push_back(glm::vec3(1, 0, 0));
            normals.push_back(glm::vec3(1, 0, 0));

            vertices.push_back(glm::vec3(i+1, k, j+1));
            vertices.push_back(glm::vec3(i+1, k+1, j));
            vertices.push_back(glm::vec3(i+1, k+1, j+1));
            uvs.push_back(UVs[0][0]);
            uvs.push_back(UVs[0][2]);
            uvs.push_back(UVs[0][3]);
            normals.push_back(glm::vec3(1, 0, 0));
            normals.push_back(glm::vec3(1, 0, 0));
            normals.push_back(glm::vec3(1, 0, 0));
        }
        if (sides[1] && sides[1]->isTransparent()) {
            // LEFT
            vertices.push_back(glm::vec3(i, k, j));
            vertices.push_back(glm::vec3(i, k, j+1));
            vertices.push_back(glm::vec3(i, k+1, j+1));
            uvs.push_back(UVs[1][0]);
            uvs.push_back(UVs[1][1]);
            uvs.push_back(UVs[1][2]);
            normals.push_back(glm::vec3(-1, 0, 0));
            normals.push_back(glm::vec3(-1, 0, 0));
            normals.push_back(glm::vec3(-1, 0, 0));

            vertices.push_back(glm::vec3(i, k, j));
            vertices.push_back(glm::vec3(i, k+1, j+1));
            vertices.push_back(glm::vec3(i, k+1, j));
            uvs.push_back(UVs[1][0]);
            uvs.push_back(UVs[1][2]);
            uvs.push_back(UVs[1][3]);
            normals.push_back(glm::vec3(-1, 0, 0));
            normals.push_back(glm::vec3(-1, 0, 0));
            normals.push_back(glm::vec3(-1, 0, 0));
        }
        if (sides[2] && sides[2]->isTransparent()) {
            // TOP
            vertices.push_back(glm::vec3(i, k+1, j+1));
            vertices.push_back(glm::vec3(i+1, k+1, j+1));
            vertices.push_back(glm::vec3(i+1, k+1, j));
            uvs.push_back(UVs[2][0]);
            uvs.push_back(UVs[2][1]);
            uvs.push_back(UVs[2][2]);
            normals.push_back(glm::vec3(0, 1, 0));
            normals.push_back(glm::vec3(0, 1, 0));
            normals.push_back(glm::vec3(0, 1, 0));

            vertices.push_back(glm::vec3(i, k+1, j+1));
            vertices.push_back(glm::vec3(i+1, k+1, j));
            vertices.push_back(glm::vec3(i, k+1, j));
            uvs.push_back(UVs[2][0]);
            uvs.push_back(UVs[2][2]);
            uvs.push_back(UVs[2][3]);
            normals.push_back(glm::vec3(0, 1, 0));
            normals.push_back(glm::vec3(0, 1, 0));
            normals.push_back(glm::vec3(0, 1, 0));
        }
        if (sides[3] && sides[3]->isTransparent()) {
            // BOTTOM
            vertices.push_back(glm::vec3(i, k, j));
            vertices.push_back(glm::vec3(i+1, k, j));
            vertices.push_back(glm::vec3(i+1, k, j+1));
            uvs.push_back(UVs[3][0]);
            uvs.push_back(UVs[3][1]);
            uvs.push_back(UVs[3][2]);
            normals.push_back(glm::vec3(0, -1, 0));
            normals.push_back(glm::vec3(0, -1, 0));
            normals.push_back(glm::vec3(0, -1, 0));

            vertices.push_back(glm::vec3(i, k, j));
            vertices.push_back(glm::vec3(i+1, k, j+1));
            vertices.push_back(glm::vec3(i, k, j+1));
            uvs.push_back(UVs[3][0]);
            uvs.push_back(UVs[3][2]);
            uvs.push_back(UVs[3][3]);
            normals.push_back(glm::vec3(0, -1, 0));
            normals.push_back(glm::vec3(0, -1, 0));
            normals.push_back(glm::vec3(0, -1, 0));
        }
        if (sides[4] && sides[4]->isTransparent()) {
            // BACK
            vertices.push_back(glm::vec3(i, k, j+1));
            vertices.push_back(glm::vec3(i+1, k, j+1));
            vertices.push_back(glm::vec3(i+1, k+1, j+1));
            uvs.push_back(UVs[4][0]);
            uvs.push_back(UVs[4][1]);
            uvs.push_back(UVs[4][2]);
            normals.push_back(glm::vec3(0, 0, 1));
            normals.push_back(glm::vec3(0, 0, 1));
            normals.push_back(glm::vec3(0, 0, 1));

            vertices.push_back(glm::vec3(i, k, j+1));
            vertices.push_back(glm::vec3(i+1, k+1, j+1));
            vertices.push_back(glm::vec3(i, k+1, j+1));
            uvs.push_back(UVs[4][0]);
            uvs.push_back(UVs[4][2]);
            uvs.push_back(UVs[4][3]);
            normals.push_back(glm::vec3(0, 0, 1));
            normals.push_back(glm::vec3(0, 0, 1));
            normals.push_back(glm::vec3(0, 0, 1));
        }
        if (sides[5] && sides[5]->isTransparent()) {
            // FRONT
            vertices.push_back(glm::vec3(i+1, k, j));
            vertices.push_back(glm::vec3(i, k, j));
            vertices.push_back(glm::vec3(i, k+1, j));
            uvs.push_back(UVs[5][0]);
            uvs.push_back(UVs[5][1]);
            uvs.push_back(UVs[5][2]);
            normals.push_back(glm::vec3(0, 0, -1));
            normals.push_back(glm::vec3(0, 0, -1));
            normals.push_back(glm::vec3(0, 0, -1));

            vertices.push_back(glm::vec3(i+1, k, j));
            vertices.push_back(glm::vec3(i, k+1, j));
            vertices.push_back(glm::vec3(i+1, k+1, j));
            uvs.push_back(UVs[5][0]);
            uvs.push_back(UVs[5][2]);
            uvs.push_back(UVs[5][3]);
            normals.push_back(glm::vec3(0, 0, -1));
            normals.push_back(glm::vec3(0, 0, -1));
            normals.push_back(glm::vec3(0, 0, -1));
        }
    break;
    case Shape::FAKE3D:
        // South-East side

        vertices.push_back(glm::vec3(i, k, j+1));
        vertices.push_back(glm::vec3(i+1, k, j));
        vertices.push_back(glm::vec3(i+1, k+1, j));
        uvs.push_back(UVs[0][0]);
        uvs.push_back(UVs[0][1]);
        uvs.push_back(UVs[0][2]);
        normals.push_back(glm::vec3(0.7071068, 0, 0.7071068));
        normals.push_back(glm::vec3(0.7071068, 0, 0.7071068));
        normals.push_back(glm::vec3(0.7071068, 0, 0.7071068));

        vertices.push_back(glm::vec3(i, k, j+1));
        vertices.push_back(glm::vec3(i+1, k+1, j));
        vertices.push_back(glm::vec3(i, k+1, j+1));
        uvs.push_back(UVs[0][0]);
        uvs.push_back(UVs[0][2]);
        uvs.push_back(UVs[0][3]);
        normals.push_back(glm::vec3(0.7071068, 0, 0.7071068));
        normals.push_back(glm::vec3(0.7071068, 0, 0.7071068));
        normals.push_back(glm::vec3(0.7071068, 0, 0.7071068));

        // North-West side

        vertices.push_back(glm::vec3(i+1, k, j));
        vertices.push_back(glm::vec3(i, k, j+1));
        vertices.push_back(glm::vec3(i, k+1, j+1));
        uvs.push_back(UVs[0][1]);
        uvs.push_back(UVs[0][0]);
        uvs.push_back(UVs[0][3]);
        normals.push_back(glm::vec3(-0.7071068, 0, -0.7071068));
        normals.push_back(glm::vec3(-0.7071068, 0, -0.7071068));
        normals.push_back(glm::vec3(-0.7071068, 0, -0.7071068));

        vertices.push_back(glm::vec3(i+1, k, j));
        vertices.push_back(glm::vec3(i, k+1, j+1));
        vertices.push_back(glm::vec3(i+1, k+1, j));
        uvs.push_back(UVs[0][1]);
        uvs.push_back(UVs[0][3]);
        uvs.push_back(UVs[0][2]);
        normals.push_back(glm::vec3(-0.7071068, 0, -0.7071068));
        normals.push_back(glm::vec3(-0.7071068, 0, -0.7071068));
        normals.push_back(glm::vec3(-0.7071068, 0, -0.7071068));


        // North-East side

        vertices.push_back(glm::vec3(i+1, k, j+1));
        vertices.push_back(glm::vec3(i, k, j));
        vertices.push_back(glm::vec3(i, k+1, j));
        uvs.push_back(UVs[0][0]);
        uvs.push_back(UVs[0][1]);
        uvs.push_back(UVs[0][2]);
        normals.push_back(glm::vec3(0.7071068, 0, -0.7071068));
        normals.push_back(glm::vec3(0.7071068, 0, -0.7071068));
        normals.push_back(glm::vec3(0.7071068, 0, -0.7071068));

        vertices.push_back(glm::vec3(i+1, k, j+1));
        vertices.push_back(glm::vec3(i, k+1, j));
        vertices.push_back(glm::vec3(i+1, k+1, j+1));
        uvs.push_back(UVs[0][0]);
        uvs.push_back(UVs[0][2]);
        uvs.push_back(UVs[0][3]);
        normals.push_back(glm::vec3(0.7071068, 0, -0.7071068));
        normals.push_back(glm::vec3(0.7071068, 0, -0.7071068));
        normals.push_back(glm::vec3(0.7071068, 0, -0.7071068));

        // South-West side

        vertices.push_back(glm::vec3(i, k, j));
        vertices.push_back(glm::vec3(i+1, k, j+1));
        vertices.push_back(glm::vec3(i+1, k+1, j+1));
        uvs.push_back(UVs[0][1]);
        uvs.push_back(UVs[0][0]);
        uvs.push_back(UVs[0][3]);
        normals.push_back(glm::vec3(-0.7071068, 0, 0.7071068));
        normals.push_back(glm::vec3(-0.7071068, 0, 0.7071068));
        normals.push_back(glm::vec3(-0.7071068, 0, 0.7071068));

        vertices.push_back(glm::vec3(i, k, j));
        vertices.push_back(glm::vec3(i+1, k+1, j+1));
        vertices.push_back(glm::vec3(i, k+1, j));
        uvs.push_back(UVs[0][1]);
        uvs.push_back(UVs[0][3]);
        uvs.push_back(UVs[0][2]);
        normals.push_back(glm::vec3(-0.7071068, 0, 0.7071068));
        normals.push_back(glm::vec3(-0.7071068, 0, 0.7071068));
        normals.push_back(glm::vec3(-0.7071068, 0, 0.7071068));
    break;
    }
}
