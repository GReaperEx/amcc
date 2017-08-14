#include "CGameEngine.h"

#include <iostream>

#include <glm/glm.hpp>

void CGameEngine::updateAll()
{
    Uint32 curUpdate = SDL_GetTicks();
    float deltaT = (curUpdate - lastUpdate)/1000.0f ;

    if (cameraEnabled) {
        camera.update(deltaT);
    }

    lastUpdate = curUpdate;
}

void CGameEngine::renderAll()
{
    // Calculating the view-projection matrix
    glm::mat4 v = camera.genViewMatrix();
    glm::mat4 p = camera.genProjectionMatrix();
    glm::mat4 vp = p*v;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Draw stuff here
    chunkManager.renderChunks(shaderManager, vp);

    SDL_GL_SwapWindow(mainWindow);
}

void fatalError(const std::string& prefix, const std::string& message);

void CGameEngine::initState(const std::string& wndName, int wndWidth, int wndHeight, bool fullscreen)
{
    windowName = wndName;
    windowWidth = wndWidth;
    windowHeight = wndHeight;
    fullscreenEnabled = fullscreen;
    vsyncEnabled = false;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0) {
        fatalError("SDL_Error: ", SDL_GetError());
    }

    Uint32 flags = SDL_WINDOW_OPENGL;
    if (fullscreenEnabled) {
        flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
    }

    mainWindow = SDL_CreateWindow(
        windowName.c_str(),
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        wndWidth, wndHeight,
        flags
    );
    if (!mainWindow) {
        fatalError("SDL_Error: ", SDL_GetError());
    }
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    context = SDL_GL_CreateContext(mainWindow);
    if (!context) {
        fatalError("SDL_Error: ", SDL_GetError());
    }

    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        fatalError("GLEW_Error: ", (const char*)glewGetString(err));
    }

    glGenVertexArrays(1, &vtxArrayID);
    glBindVertexArray(vtxArrayID);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    if (!shaderManager.addShader("default", "shaders/default.vert", "shaders/default.frag")) {
        fatalError("Program_Error: ", "Failed to load required shaders.");
    }

    camera.setAspectRatio(windowWidth/(float)windowHeight);
    camera.setPosition(glm::vec3(0, 150, 0));
    cameraEnabled = true;
    SDL_SetRelativeMouseMode(SDL_TRUE);

    lastUpdate = SDL_GetTicks();

    chunkManager.init(textureManager);
}

void CGameEngine::clearState()
{
    glDeleteVertexArrays(1, &vtxArrayID);
    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(mainWindow);
    SDL_Quit();
}

bool CGameEngine::handleEvent(const SDL_Event& event)
{
    if (!cameraEnabled || !camera.handleEvent(event)) {
        switch (event.type)
        {
        case SDL_KEYDOWN:
            if (event.key.keysym.sym == SDLK_ESCAPE) {
                return false;
            }
        break;
        case SDL_QUIT:
            return false;
        break;
        }
    }

    return true;
}


void fatalError(const std::string& prefix, const std::string& message)
{
    std::cerr << prefix << message << '\n' << std::endl;
    exit(1);
}