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

    CTexture* getTexture(const std::string& path, bool genMipmaps = false,
                                            GLenum magFilter = GL_NEAREST,
                                            GLenum minFilter = GL_NEAREST,
                                            GLenum wrapS = GL_CLAMP_TO_EDGE,
                                            GLenum wrapT = GL_CLAMP_TO_EDGE) {
        auto it = textureMap.find(path);
        if (it != textureMap.end()) {
            it->second->grab();
            return it->second;
        }

        CTexture *newTexture = new CTexture(path, genMipmaps, magFilter, minFilter, wrapS, wrapT);
        textureMap[path] = newTexture;

        newTexture->grab();
        return newTexture;
    }

    void remTexture(const std::string& path) {
        auto it = textureMap.find(path);
        if (it != textureMap.end()) {
            it->second->drop();
            textureMap.erase(it);
        }
    }

private:
    std::map<std::string, CTexture*> textureMap;
};

#endif // C_TEXTURE_MANAGER_H
