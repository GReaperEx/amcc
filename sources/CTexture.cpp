#include "CTexture.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

CTexture::CTexture(const std::string& file, bool genMipmaps, GLenum magFilter, GLenum minFilter, GLenum wrapS, GLenum wrapT)
{
    this->magFilter = magFilter;
    this->minFilter = minFilter;
    this->wrapS = wrapS;
    this->wrapT = wrapT;
    load(file, genMipmaps);
}

void CTexture::setMagFilter(GLenum newMagFilter)
{
    magFilter = newMagFilter;
    glBindTexture(GL_TEXTURE_2D, ID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
}

void CTexture::setMinFilter(GLenum newMinFilter)
{
    minFilter = newMinFilter;
    glBindTexture(GL_TEXTURE_2D, ID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
}

void CTexture::setWrapS(GLenum newWrapS)
{
    wrapS = newWrapS;
    glBindTexture(GL_TEXTURE_2D, ID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS);
}

void CTexture::setWrapT(GLenum newWrapT)
{
    wrapT = newWrapT;
    glBindTexture(GL_TEXTURE_2D, ID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT);
}

void fatalError(const std::string& prefix, const std::string& message);
void CTexture::load(const std::string& file, bool genMipmaps)
{
    SDL_Surface *image = IMG_Load(file.c_str());
    if (image == nullptr) {
        fatalError("Program_Error: ", "Unable to load \'" + file + "\'.\n"
                   "IMG_Load: " + IMG_GetError());
    }

    width = image->w;
    height = image->h;

    GLenum internalFormat, format;
    switch (image->format->BitsPerPixel)
    {
    case 24:
        internalFormat = GL_RGB;
        if (image->format->Rmask == 0x000000FF) {
            format = GL_RGB;
        } else {
            format = GL_BGR;
        }
        hasAlpha = false;
    break;
    case 32:
        internalFormat = GL_RGBA;
        if (image->format->Rmask == 0x000000FF) {
            format = GL_RGBA;
        } else {
            format = GL_BGRA;
        }
        hasAlpha = true;
    break;
    default:
        fatalError("Program_Error", "Format of \'" + file + "\' isn't supported.");
    }

    glGenTextures(1, &ID);
    glBindTexture(GL_TEXTURE_2D, ID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT);

    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, image->pixels);
    if (genMipmaps) {
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    SDL_FreeSurface(image);
}
