#include <glm/glm.hpp>

#include <string>
#include <iostream>

#include "GameEngine.h"
#include "TextureManager.h"
#include "ShaderManager.h"

int main(int argc, char* argv[])
{
    GameEngine myEngine("MyMC", 800, 600, false);

    while (myEngine.handleInput()) {
        myEngine.updateAll();
        myEngine.renderAll();

        SDL_Delay(30);
    }

    return 0;
}
