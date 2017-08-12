#ifndef CSHADER_MANAGER_H
#define CSHADER_MANAGER_H

#include <string>
#include <GL/glew.h>
#include <GL/gl.h>

#include <map>

class CShaderManager
{
public:
    CShaderManager() {}
    ~CShaderManager();

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

#endif // CSHADER_MANAGER_H
