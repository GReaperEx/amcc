#ifndef C_BOX_OUTLINE_H
#define C_BOX_OUTLINE_H

#include <GL/glew.h>
#include <GL/gl.h>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

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

    void init(const glm::vec3& color);
    void render(CShaderManager& shaderManager, const glm::mat4& vp, const glm::vec3& position);

private:
    GLuint bufferID;
    glm::vec3 lineColor;
};

#endif // C_BOX_OUTLINE_H
