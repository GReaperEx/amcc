#ifndef C_TEXTURE_H
#define C_TEXTURE_H

#include <GL/glew.h>
#include <GL/gl.h>

#include <string>

#include "IRefCounted.h"

class CTexture : public IRefCounted
{
public:
    CTexture(const std::string& file, bool genMipmaps = true,
                                      GLenum magFilter = GL_NEAREST,
                                      GLenum minFilter = GL_NEAREST_MIPMAP_NEAREST,
                                      GLenum wrapS = GL_REPEAT,
                                      GLenum wrapT = GL_REPEAT);
    ~CTexture() {
        glDeleteTextures(1, &ID);
    }

    int getWidth() const {
        return width;
    }
    int getHeight() const {
        return height;
    }
    bool hasAlphaChannel() const {
        return hasAlpha;
    }

    GLenum getMagFilter() const {
        return magFilter;
    }
    void setMagFilter(GLenum newMagFilter);

    GLenum getMinFilter() const {
        return minFilter;
    }
    void setMinFilter(GLenum newMinFilter);

    GLenum getWrapS() const {
        return wrapS;
    }
    void setWrapS(GLenum newWrapS);

    GLenum getWrapT() const {
        return wrapT;
    }
    void setWrapT(GLenum newWrapT);

    void use() const {
        glBindTexture(GL_TEXTURE_2D, ID);
    }

private:
    void load(const std::string& file, bool genMipmaps);

    GLuint ID;
    int width;
    int height;
    GLenum magFilter;
    GLenum minFilter;
    GLenum wrapS;
    GLenum wrapT;
    bool hasAlpha;
};

#endif // C_TEXTURE_H
