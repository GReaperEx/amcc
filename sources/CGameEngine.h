#ifndef C_GAME_ENGINE_H
#define C_GAME_ENGINE_H

#include <GL/glew.h>
#include <GL/gl.h>

#include <SDL2/SDL.h>

#include <string>

#include "CShaderManager.h"
#include "CTextureManager.h"
#include "CCamera.h"

#include "CTestQuad.h"

class CGameEngine
{
public:
    CGameEngine(const std::string& wndName, int wndWidth, int wndHeight, bool fullscreen) {
        initState(wndName, wndWidth, wndHeight, fullscreen);
    }

    ~CGameEngine() {
        clearState();
    }

    void updateAll();
    void renderAll();

    // Returns false if the game was asked to terminate
    bool handleInput() {
        SDL_Event event;
        bool run = true;

        while (run && SDL_PollEvent(&event)) {
            run = handleEvent(event);
        }

        return run;
    }

private:
    void initState(const std::string& wndName, int wndWidth, int wndHeight, bool fullscreen);
    void clearState();

    // Returns false if an event requests termination
    bool handleEvent(const SDL_Event& event);

private:
    std::string windowName;
    int windowWidth;
    int windowHeight;
    bool fullscreenEnabled;
    bool vsyncEnabled;

    SDL_Window* mainWindow;
    SDL_GLContext context;

    GLuint vtxArrayID;

    CShaderManager shaderManager;
    CTextureManager textureManager;

    CCamera camera;
    bool cameraEnabled;

    Uint32 lastUpdate;

    CTestQuad myQuad;
};

#endif // C_GAME_ENGINE_H
