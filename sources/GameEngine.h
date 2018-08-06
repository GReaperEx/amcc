#ifndef GAME_ENGINE_H
#define GAME_ENGINE_H

#include <GL/glew.h>
#include <GL/gl.h>

#include <SDL2/SDL.h>

#include <string>

#include "ShaderManager.h"
#include "TextureManager.h"
#include "Camera.h"

#include "World.h"
#include "NoiseGenerator.h"

class GameEngine
{
public:
    GameEngine(const std::string& wndName, int wndWidth, int wndHeight, bool fullscreen) {
        initState(wndName, wndWidth, wndHeight, fullscreen);
    }

    ~GameEngine() {
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

    Camera camera;
    bool cameraEnabled;

    Uint32 lastUpdate;

    World curWorld;
    std::vector<NoiseGenerator> noiseGens;
};

#endif // GAME_ENGINE_H
