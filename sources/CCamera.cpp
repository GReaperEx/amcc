#include "CCamera.h"

bool CCamera::handleEvent(const SDL_Event& event)
{
    switch (event.type)
    {
    case SDL_KEYDOWN:
        switch (event.key.keysym.sym)
        {
        case SDLK_d:
            keepMoving[0] = true;
        break;
        case SDLK_a:
            keepMoving[1] = true;
        break;
        case SDLK_SPACE:
            keepMoving[2] = true;
        break;
        case SDLK_LSHIFT:
            keepMoving[3] = true;
        break;
        case SDLK_s:
            keepMoving[4] = true;
        break;
        case SDLK_w:
            keepMoving[5] = true;
        break;
        default:
            return false;
        }
    break;
    case SDL_KEYUP:
        switch (event.key.keysym.sym)
        {
        case SDLK_d:
            keepMoving[0] = false;
        break;
        case SDLK_a:
            keepMoving[1] = false;
        break;
        case SDLK_SPACE:
            keepMoving[2] = false;
        break;
        case SDLK_LSHIFT:
            keepMoving[3] = false;
        break;
        case SDLK_s:
            keepMoving[4] = false;
        break;
        case SDLK_w:
            keepMoving[5] = false;
        break;
        default:
            return false;
        }
    break;
    case SDL_MOUSEMOTION:
    {
        glm::mat4 rotateX = glm::rotate(glm::mat4(1.0f), glm::radians(-(float)event.motion.yrel/4), glm::cross(getLookVector(), getUpVector()));
        glm::mat4 rotateY = glm::rotate(glm::mat4(1.0f), glm::radians(-(float)event.motion.xrel/4), getUpVector());

        setLookVector(glm::vec3(rotateY*rotateX*glm::vec4(getLookVector(), 0)));
    }
    break;
    default:
        return false;
    }

    return true;
}

void CCamera::update(float dT)
{
    glm::vec3 direction(0, 0, 0);
    if (keepMoving[0]) {
        direction += glm::cross(lookVector, upVector);
    } else if (keepMoving[1]) {
        direction -= glm::cross(lookVector, upVector);;
    }
    if (keepMoving[2]) {
        direction += upVector;
    } else if (keepMoving[3]) {
        direction -= upVector;
    }
    if (keepMoving[4]) {
        direction -= glm::vec3(lookVector.x, 0.0f, lookVector.z);
    } else if (keepMoving[5]) {
        direction += glm::vec3(lookVector.x, 0.0f, lookVector.z);
    }
    if (direction != glm::vec3(0, 0, 0)) {
        direction = glm::normalize(direction);
    }

    position += direction*dT*50.0f;
}
