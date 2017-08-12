#ifndef C_CAMERA_H
#define C_CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <SDL2/SDL.h>

#include "IRefCounted.h"

class CCamera : public IRefCounted
{
public:
    CCamera()
    : position(0), lookVector(0, 0, -1), upVector(0, 1, 0), fieldOfView(70), aspectRatio(4/3.0f), nearClipDistance(0.001f), farClipDistance(1000)
    {
        keepMoving[0] = false;
        keepMoving[1] = false;
        keepMoving[2] = false;
        keepMoving[3] = false;
        keepMoving[4] = false;
        keepMoving[5] = false;
    }

    CCamera(const glm::vec3& pos, const glm::vec3& look, const glm::vec3& up, float FoV, float aspect, float near, float far)
    : position(pos), lookVector(look), upVector(up), fieldOfView(FoV), aspectRatio(aspect), nearClipDistance(near), farClipDistance(far)
    {
        keepMoving[0] = false;
        keepMoving[1] = false;
        keepMoving[2] = false;
        keepMoving[3] = false;
        keepMoving[4] = false;
        keepMoving[5] = false;
    }

    const glm::vec3& getPosition() const {
        return position;
    }
    void setPosition(const glm::vec3& newPos) {
        position = newPos;
    }

    const glm::vec3& getLookVector() const {
        return lookVector;
    }
    void setLookVector(const glm::vec3& newLook) {
        lookVector = newLook;
    }

    const glm::vec3& getUpVector() const {
        return upVector;
    }
    void setUpVector(const glm::vec3& newUp) {
        upVector = newUp;
    }

    float getFieldOfView() const {
        return fieldOfView;
    }
    void setFieldOfView(float newFoV) {
        fieldOfView = newFoV;
    }

    float getAspectRatio() const {
        return aspectRatio;
    }
    void setAspectRatio(float newAspect) {
        aspectRatio = newAspect;
    }

    float getNearClipDistance() const {
        return nearClipDistance;
    }
    void setNearClipDistance(float newNear) {
        nearClipDistance = newNear;
    }

    float getFarClipDistance() const {
        return farClipDistance;
    }
    void setFarClipDistance(float newFar) {
        farClipDistance = newFar;
    }

    const glm::mat4 genViewMatrix() const {
        return glm::lookAt(position, position+lookVector, upVector);
    }

    const glm::mat4 genProjectionMatrix() const {
        return glm::perspective(fieldOfView, aspectRatio, nearClipDistance, farClipDistance);
    }

    bool handleEvent(const SDL_Event& event) {
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

    void update(float dT) {
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
            direction -= lookVector;
        } else if (keepMoving[5]) {
            direction += lookVector;
        }
        if (direction != glm::vec3(0, 0, 0)) {
            direction = glm::normalize(direction);
        }

        position += direction*dT*10.0f;
    }

private:
    glm::vec3 position;

    glm::vec3 lookVector;
    glm::vec3 upVector;

    float fieldOfView;
    float aspectRatio;
    float nearClipDistance;
    float farClipDistance;

    bool keepMoving[6]; // Right, Left, Up, Down, Back, Forward
};

#endif // C_CAMERA_H
