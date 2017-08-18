#ifndef UTILS_3D_H
#define UTILS_3D_H

#include <glm/glm.hpp>

namespace utils3d
{

struct AABBox
{
    glm::vec3 minVec;
    glm::vec3 maxVec;

    AABBox(): minVec(0), maxVec(0) {}

    AABBox(const glm::vec3& minV, const glm::vec3& maxV)
    : minVec(minV), maxVec(maxV)
    {}

    void addPoint(const glm::vec3& newPoint) {
        if (newPoint.x > maxVec.x) {
            maxVec.x = newPoint.x;
        } else if (newPoint.x < minVec.x) {
            minVec.x = newPoint.x;
        }
        if (newPoint.y > maxVec.y) {
            maxVec.y = newPoint.y;
        } else if (newPoint.y < minVec.y) {
            minVec.y = newPoint.y;
        }
        if (newPoint.z > maxVec.z) {
            maxVec.z = newPoint.z;
        } else if (newPoint.z < minVec.z) {
            minVec.z = newPoint.z;
        }
    }
};

struct Plane
{
    glm::vec3 normal;
    float D;

    Plane() {}

    Plane(const glm::vec3& nml, float d)
    : normal(nml), D(d)
    {}

    Plane(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c) {
        glm::vec3 AB = b - a;
        glm::vec3 AC = c - a;

        normal = glm::normalize(glm::cross(AB, AC));
        D = -glm::dot(normal, a);
    }

    float getDistance(const glm::vec3& point) const {
        return glm::dot(normal, point) + D;
    }

    const glm::vec3 getRayIntersection(const glm::vec3& rayPos, const glm::vec3& rayDir) const {
        float cosTheta = glm::dot(rayDir, normal);
        if (glm::abs(cosTheta) == 0.0f) {
            return rayPos;
        }
        return rayPos + rayDir*(getDistance(rayPos)/cosTheta);
    }
};

struct Frustum
{
    Plane sides[6];

    Frustum(const glm::mat4& viewMatrix, float fieldOfView, float aspectRatio, float near, float far) {
        float cosTheta_2 = glm::cos(glm::radians(fieldOfView*0.5f));
        float sinTheta_2 = glm::sin(glm::radians(fieldOfView*0.5f));

        float nearWidth = (near/cosTheta_2)*sinTheta_2;
        float farWidth = (far/cosTheta_2)*sinTheta_2;

        float nearHeight = nearWidth/aspectRatio;
        float farHeight = farWidth/aspectRatio;

        glm::vec3 points[2][4] = {
            { glm::vec3(-nearWidth, -nearHeight, -near), glm::vec3(nearWidth, -nearHeight, -near),
              glm::vec3(nearWidth, nearHeight, -near), glm::vec3(-nearWidth, nearHeight, -near) },
            { glm::vec3(-farWidth, -farHeight, -far), glm::vec3(farWidth, -farHeight, -far),
              glm::vec3(farWidth, farHeight, -far), glm::vec3(-farWidth, farHeight, -far) }
        };

        for (int i = 0; i < 2; ++i) {
            for (int j = 0; j < 4; ++j) {
                points[i][j] = glm::vec3(viewMatrix*glm::vec4(points[i][j], 1.0f));
            }
        }

        sides[0] = Plane(points[0][1], points[1][1], points[1][2]); // Right plane
        sides[1] = Plane(points[1][0], points[0][0], points[0][3]); // Left plane
        sides[2] = Plane(points[0][3], points[0][2], points[1][2]); // Top plane
        sides[3] = Plane(points[1][0], points[1][1], points[0][1]); // Bottom plane
        sides[4] = Plane(points[0][0], points[0][1], points[0][2]); // Back plane
        sides[5] = Plane(points[1][1], points[1][0], points[1][3]); // Front plane
    }
};

bool AABBcollision(const AABBox& boxA, const AABBox& boxB);
bool FrustumAABBcollision(const Frustum& frustum, const AABBox& box);
bool RayAABBcollision(const glm::vec3& rayPos, const glm::vec3& rayDir_inverted, const AABBox& box);

}

#endif // UTILS_3D_H
