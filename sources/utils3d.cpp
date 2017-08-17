#include "utils3d.h"

bool utils3d::frustumAABBcollision(const Frustum& frustum, const AABBox& box)
{
   glm::vec3 vmin, vmax;

   for(int i = 0; i < 6; ++i) {
      // X axis
      if(frustum.sides[i].normal.x > 0) {
         vmin.x = box.minVec.x;
         vmax.x = box.maxVec.x;
      } else {
         vmin.x = box.maxVec.x;
         vmax.x = box.minVec.x;
      }
      // Y axis
      if(frustum.sides[i].normal.y > 0) {
         vmin.y = box.minVec.y;
         vmax.y = box.maxVec.y;
      } else {
         vmin.y = box.maxVec.y;
         vmax.y = box.minVec.y;
      }
      // Z axis
      if(frustum.sides[i].normal.z > 0) {
         vmin.z = box.minVec.z;
         vmax.z = box.maxVec.z;
      } else {
         vmin.z = box.maxVec.z;
         vmax.z = box.minVec.z;
      }
      if(frustum.sides[i].getDistance(vmin) > 0) {
         return false;
      }
   }
   return true;
}

bool utils3d::rayAABBcollision(const glm::vec3& rayPos, const glm::vec3& rayDir_inverted, const AABBox& box)
{
    using glm::min;
    using glm::max;

    glm::vec3 minCheck = (box.minVec - rayPos)*rayDir_inverted;
    glm::vec3 maxCheck = (box.maxVec - rayPos)*rayDir_inverted;

    float tmin = max(max(min(minCheck.x, maxCheck.x), min(minCheck.y, maxCheck.y)), min(minCheck.z, maxCheck.z));
    float tmax = min(min(max(minCheck.x, maxCheck.x), max(minCheck.y, maxCheck.y)), max(minCheck.z, maxCheck.z));

    return tmax >= tmin && tmax >= 0;
}
