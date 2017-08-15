#include <glm/glm.hpp>

#include <string>
#include <iostream>

#include "CGameEngine.h"
#include "CTextureManager.h"
#include "CShaderManager.h"

int main()
{
    CGameEngine myEngine("MyMC", 800, 600, false);

    while (myEngine.handleInput()) {
        myEngine.updateAll();
        myEngine.renderAll();

        SDL_Delay(30);
    }

    return 0;
}
