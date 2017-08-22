#ifndef BOX_OUTLINE_H
#define BOX_OUTLINE_H

#include <GL/glew.h>
#include <GL/gl.h>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include <vector>

#include "ShaderManager.h"

class BoxOutline
{
public:
    BoxOutline(): bufferID(0) {}
    ~BoxOutline() {
        if (bufferID != 0) {
            glDeleteBuffers(1, &bufferID);
        }
    }

    void init(const glm::vec3& color);
    void render(ShaderManager& shaderManager, const glm::mat4& vp, const glm::vec3& position);

private:
    GLuint bufferID;
    glm::vec3 lineColor;
};

#endif // BOX_OUTLINE_H
