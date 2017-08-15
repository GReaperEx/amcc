#ifndef C_BOX_OUTLINE_H
#define C_BOX_OUTLINE_H

#include <GL/glew.h>
#include <GL/gl.h>
#include <glm/glm.hpp>

#include <vector>

#include "CShaderManager.h"

class CBoxOutline
{
public:
    CBoxOutline(): bufferID(0) {}
    ~CBoxOutline() {
        if (bufferID != 0) {
            glDeleteBuffers(1, &bufferID);
        }
    }

    void init(const glm::vec3& color) {
        glm::vec3 vertices[12][2] = {
            { glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.5f, -0.5f, -0.5f) },
            { glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(0.5f, 0.5f, -0.5f) },
            { glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(-0.5f, 0.5f, -0.5f) },
            { glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(-0.5f, -0.5f, -0.5f) },
            { glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(0.5f, -0.5f, 0.5f) },
            { glm::vec3(0.5f, -0.5f, 0.5f), glm::vec3(0.5f, 0.5f, 0.5f) },
            { glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(-0.5f, 0.5f, 0.5f) },
            { glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(-0.5f, -0.5f, 0.5f) },
            { glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(-0.5f, -0.5f, 0.5f) },
            { glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(0.5f, -0.5f, 0.5f) },
            { glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(0.5f, 0.5f, 0.5f) },
            { glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(-0.5f, 0.5f, 0.5f) }
        };

        glGenBuffers(1, &bufferID);
        glBindBuffer(GL_ARRAY_BUFFER, bufferID);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        lineColor = color;
    }

    void render(CShaderManager& shaderManager, const glm::mat4& vp, const glm::vec3& position) {
        shaderManager.use("simple");

        glm::mat4 mvp = vp*(glm::translate(glm::mat4(1), position + glm::vec3(0.5f, 0.5f, 0.5f))*glm::scale(glm::mat4(1), glm::vec3(1.025f, 1.025f, 1.025f)));
        glUniformMatrix4fv(shaderManager.getUniformLocation("MVP"), 1, GL_FALSE, &(mvp[0][0]));
        glUniform3fv(shaderManager.getUniformLocation("lineColor"), 1, &(lineColor[0]));

        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, bufferID);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

        glDrawArrays(GL_LINES, 0, 24);

        glDisableVertexAttribArray(0);
    }

private:
    GLuint bufferID;
    glm::vec3 lineColor;
};

#endif // C_BOX_OUTLINE_H
