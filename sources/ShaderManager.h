#ifndef SHADER_MANAGER_H
#define SHADER_MANAGER_H

#include <string>
#include <GL/glew.h>
#include <GL/gl.h>

#include <map>

class ShaderManager
{
public:
    ShaderManager() { currentShader = 0; }
    ~ShaderManager();

    bool addShader(const std::string& name, const std::string& vert_path, const std::string& frag_path);
    void remShader(const std::string& name);
    bool use(const std::string& shaderName);

    GLuint getUniformLocation(const std::string& name);

private:
    GLuint load(const std::string& vert_path, const std::string& frag_path);

    std::map<std::string, GLuint> loadedShaders;
    GLuint currentShader;
    std::map<std::string, GLuint> fetchedUniforms;
};

extern ShaderManager g_ShaderManager;

#endif // SHADER_MANAGER_H
