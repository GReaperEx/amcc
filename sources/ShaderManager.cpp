#include "ShaderManager.h"

#include <cassert>
#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

ShaderManager::~ShaderManager()
{
    for (auto shader : loadedShaders) {
        glDeleteProgram(shader.second);
    }
}

bool ShaderManager::addShader(const std::string& name, const std::string& vert_path, const std::string& frag_path)
{
    remShader(name);

    GLuint newShader = load(vert_path, frag_path);
    if (newShader == 0) {
        return false;
    }

    loadedShaders[name] = newShader;
    return true;
}

void ShaderManager::remShader(const std::string& name)
{
    auto shader = loadedShaders.find(name);

    if (shader != loadedShaders.end()) {
        GLuint shaderID = shader->second;
        loadedShaders.erase(shader);
        if (shaderID == currentShader) {
            fetchedUniforms.clear();
            currentShader = loadedShaders.begin()->second;
        }
        glDeleteProgram(shaderID);
    }
}

bool ShaderManager::use(const std::string& shaderName)
{
    auto shader = loadedShaders.find(shaderName);
    if (shader == loadedShaders.end()) {
        return false;
    }

    GLuint selectedShader = shader->second;

    glUseProgram(selectedShader);
    if (currentShader != selectedShader) {
        fetchedUniforms.clear();
    }
    currentShader = selectedShader;

    return true;
}

GLuint ShaderManager::getUniformLocation(const std::string& name)
{
    GLuint location = 0;

    auto entry = fetchedUniforms.find(name);
    if (entry != fetchedUniforms.end()) {
        location = entry->second;
    } else {
        location = glGetUniformLocation(currentShader, name.c_str());
        fetchedUniforms[name] = location;
    }

    return location;
}

GLuint ShaderManager::load(const std::string& vert_path, const std::string& frag_path)
{
	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vert_path, std::ios::in);
	if(VertexShaderStream.is_open()) {
		std::string Line = "";
		while(getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	} else {
	    cerr << "Unable to open \'" << vert_path << "\'." << endl;
		return 0;
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(frag_path, std::ios::in);
	if(FragmentShaderStream.is_open()){
		std::string Line = "";
		while(getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	} else {
	    cerr << "Unable to open \'" << frag_path << "\'." << endl;
		return 0;
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 1 ){
		std::vector<char> VertexShaderErrorMessage(InfoLogLength+1);
		glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
		cerr << vert_path << " : " << &VertexShaderErrorMessage[0] << endl;
		return 0;
	}

	// Compile Fragment Shader
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 1 ){
		std::vector<char> FragmentShaderErrorMessage(InfoLogLength+1);
		glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
		cerr << frag_path << " : " << &FragmentShaderErrorMessage[0] << endl;
		return 0;
	}

	// Link the program
    GLuint ID = glCreateProgram();
	glAttachShader(ID, VertexShaderID);
	glAttachShader(ID, FragmentShaderID);
	glLinkProgram(ID);

	// Check the program
	glGetProgramiv(ID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 1 ){
		std::vector<char> ProgramErrorMessage(InfoLogLength+1);
		glGetProgramInfoLog(ID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		cerr << &ProgramErrorMessage[0] << endl;

		glDetachShader(ID, VertexShaderID);
        glDetachShader(ID, FragmentShaderID);
        glDeleteProgram(ID);
		return 0;
	}

	glDetachShader(ID, VertexShaderID);
	glDetachShader(ID, FragmentShaderID);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ID;
}

ShaderManager g_ShaderManager;
