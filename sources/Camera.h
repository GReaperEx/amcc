#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <SDL2/SDL.h>

#include "IRefCounted.h"
#include "utils3d.h"

class Camera : public IRefCounted
{
public:
    Camera()
    : position(0), lookVector(0, 0, -1), upVector(0, 1, 0), fieldOfView(70), aspectRatio(4/3.0f), nearClipDistance(0.01f), farClipDistance(1000)
    {
        keepMoving[0] = false;
        keepMoving[1] = false;
        keepMoving[2] = false;
        keepMoving[3] = false;
        keepMoving[4] = false;
        keepMoving[5] = false;
    }

    Camera(const glm::vec3& pos, const glm::vec3& look, const glm::vec3& up, float FoV, float aspect, float near, float far)
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

    const glm::mat4 genMirrorviewMatrix() const {
        return glm::inverse(genViewMatrix());
    }

    const glm::mat4 genProjectionMatrix() const {
        return glm::perspective(fieldOfView, aspectRatio, nearClipDistance, farClipDistance);
    }

    const utils3d::Frustum genFrustum() const {
        return utils3d::Frustum(genMirrorviewMatrix(), fieldOfView, aspectRatio, nearClipDistance, farClipDistance);
    }

    bool handleEvent(const SDL_Event& event);
    void update(float dT);

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

#endif // CAMERA_H
