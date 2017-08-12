#ifndef C_TEXTURE_MANAGER_H
#define C_TEXTURE_MANAGER_H

#include <map>
#include <string>

#include "CTexture.h"

class CTextureManager
{
public:
    CTextureManager() {}
    ~CTextureManager() {
        for (auto it = textureMap.begin(); it != textureMap.end(); ++it) {
            it->second->drop();
        }
    }

    void addTexture(const std::string& path, bool genMipmaps = true,
                                            GLenum magFilter = GL_LINEAR,
                                            GLenum minFilter = GL_LINEAR_MIPMAP_LINEAR,
                                            GLenum wrapS = GL_REPEAT,
                                            GLenum wrapT = GL_REPEAT) {
        textureMap[path] = new CTexture(path, genMipmaps, magFilter, minFilter, wrapS, wrapT);
    }

    void remTexture(const std::string& path) {
        textureMap[path]->drop();
        textureMap.erase(path);
    }

    CTexture* getTexture(const std::string& path) {
        CTexture *ptr = textureMap[path];
        ptr->grab();
        return ptr;
    }

private:
    std::map<std::string, CTexture*> textureMap;
};

#endif // C_TEXTURE_MANAGER_H
