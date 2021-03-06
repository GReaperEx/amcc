#include "GameEngine.h"

#include <iostream>

#include <glm/glm.hpp>
#include <SDL2/SDL_image.h>

void GameEngine::updateAll()
{
    Uint32 curUpdate = SDL_GetTicks();
    float deltaT = (curUpdate - lastUpdate)/1000.0f ;

    if (cameraEnabled) {
        camera.update(deltaT);
    }

    lastUpdate = curUpdate;
}

void GameEngine::renderAll()
{
    // Calculating the view-projection matrix
    glm::mat4 v = camera.genViewMatrix();
    glm::mat4 p = camera.genProjectionMatrix();
    glm::mat4 vp = p*v;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Draw stuff here
    curWorld.renderChunks(g_ShaderManager, vp);
    curWorld.renderOutline(g_ShaderManager, vp);

    SDL_GL_SwapWindow(mainWindow);
}

void fatalError(const std::string& prefix, const std::string& message);

void GameEngine::initState(const std::string& wndName, int wndWidth, int wndHeight, bool fullscreen)
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

    if ((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) != IMG_INIT_PNG) {
        fatalError("IMG_Init: ", IMG_GetError());
    }

    glGenVertexArrays(1, &vtxArrayID);
    glBindVertexArray(vtxArrayID);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    if (!g_ShaderManager.addShader("default", "shaders/default.vert", "shaders/default.frag") ||
        !g_ShaderManager.addShader("simple", "shaders/simple.vert", "shaders/simple.frag")) {
        fatalError("Program_Error: ", "Failed to load required shaders.");
    }

    camera.setAspectRatio(windowWidth/(float)windowHeight);
    camera.setPosition(glm::vec3(0, 150, 0));
    camera.setFarClipDistance(16*16);
    cameraEnabled = true;
    SDL_SetRelativeMouseMode(SDL_TRUE);

    lastUpdate = SDL_GetTicks();

    // TODO: The seed given by the user should be passed instead
    noiseGens.push_back(NoiseGenerator(123));
    noiseGens.push_back(NoiseGenerator(123*2));

    curWorld.init(g_TextureManager, noiseGens, &camera);
}

void GameEngine::clearState()
{
    glDeleteVertexArrays(1, &vtxArrayID);
    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(mainWindow);
    SDL_Quit();
}

bool GameEngine::handleEvent(const SDL_Event& event)
{
    static int curLightLevel = 15;

    if (!cameraEnabled || !camera.handleEvent(event)) {
        switch (event.type)
        {
        case SDL_KEYDOWN:
            if (event.key.keysym.sym == SDLK_ESCAPE) {
                cameraEnabled = !cameraEnabled;
                if (cameraEnabled) {
                    SDL_SetRelativeMouseMode(SDL_TRUE);
                } else {
                    SDL_SetRelativeMouseMode(SDL_FALSE);
                }
            } else if (event.key.keysym.sym == SDLK_F1) {
                if (curLightLevel == 0) {
                    curLightLevel = 15;
                } else {
                    --curLightLevel;
                }
                curWorld.changeSunlight(curLightLevel);
            }
        break;
        case SDL_MOUSEBUTTONDOWN:
            if (event.button.button == SDL_BUTTON_LEFT) {
                Chunk::BlockDetails lookBlock;
                if (curWorld.traceRayToBlock(lookBlock, camera.getPosition(), camera.getLookVector())) {
                    lookBlock.id = 0;
                    curWorld.replaceBlock(lookBlock, true);
                }
            } else if (event.button.button == SDL_BUTTON_RIGHT) {
                Chunk::BlockDetails lookBlock;
                if (curWorld.traceRayToBlock(lookBlock, camera.getPosition(), camera.getLookVector())) {
                    curWorld.addLightSource(lookBlock.position + glm::vec3(0, 1, 0), 15);
                }
            } else if (event.button.button == SDL_BUTTON_MIDDLE) {
                Chunk::BlockDetails lookBlock;
                if (curWorld.traceRayToBlock(lookBlock, camera.getPosition(), camera.getLookVector())) {
                    curWorld.remLightSource(lookBlock.position + glm::vec3(0, 1, 0));
                }
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
