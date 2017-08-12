#ifndef C_TEST_QUAD_H
#define C_TEST_QUAD_H

#include <GL/glew.h>
#include <GL/gl.h>

#include "CShaderManager.h"
#include "CTextureManager.h"

class CTestQuad
{
public:
    CTestQuad() {
        skin = nullptr;
    }

    ~CTestQuad() {
        glDeleteBuffers(2, bufferIDs);
        if (skin) {
            skin->drop();
        }
    }

    void init(CTextureManager& textureManager) {
        glGenBuffers(2, bufferIDs);

        textureManager.addTexture("assets/testSkin.png", true, GL_NEAREST, GL_NEAREST_MIPMAP_NEAREST);
        skin = textureManager.getTexture("assets/testSkin.png");

        float vertices[6][3] = { { 0.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 0.0f },
                                 { 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } };
        float uvs[6][2] = { { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f },
                            { 0.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };

        glBindBuffer(GL_ARRAY_BUFFER, bufferIDs[0]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, bufferIDs[1]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(uvs), uvs, GL_STATIC_DRAW);
    }

    void render(CShaderManager& shaderManager, const glm::mat4& vp) {
        // A real mesh would multiply its own model matrix before passing it
        glUniformMatrix4fv(shaderManager.getUniformLocation("MVP"), 1, GL_FALSE, &(vp[0][0]));

        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, bufferIDs[0]);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, bufferIDs[1]);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

        glDrawArrays(GL_TRIANGLES, 0, 6);

        glDisableVertexAttribArray(2);
        glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(0);
    }

private:
    GLuint bufferIDs[2];
    CTexture* skin;
};

#endif // C_TEST_QUAD_H
