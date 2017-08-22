#ifndef TEXTURE_H
#define TEXTURE_H

#include <GL/glew.h>
#include <GL/gl.h>

#include <SDL2/SDL.h>

#include <string>

#include "IRefCounted.h"

class Texture : public IRefCounted
{
public:
    Texture(const std::string& file, bool genMipmaps = false,
                                      GLenum magFilter = GL_NEAREST,
                                      GLenum minFilter = GL_NEAREST,
                                      GLenum wrapS = GL_CLAMP_TO_EDGE,
                                      GLenum wrapT = GL_CLAMP_TO_EDGE);
    Texture(SDL_Surface* surface, bool genMipmaps = false,
                                      GLenum magFilter = GL_NEAREST,
                                      GLenum minFilter = GL_NEAREST,
                                      GLenum wrapS = GL_CLAMP_TO_EDGE,
                                      GLenum wrapT = GL_CLAMP_TO_EDGE);
    ~Texture() {
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
    void load(SDL_Surface* surface, bool genMipmaps);

    GLuint ID;
    int width;
    int height;
    GLenum magFilter;
    GLenum minFilter;
    GLenum wrapS;
    GLenum wrapT;
    bool hasAlpha;
};

#endif // TEXTURE_H
