#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

#include <camera.hpp>
#include <mesh.hpp>

class Portal : public Plane
{
    public:
        Portal(glm::vec2 dimensions) : Plane(dimensions)
        {

        }

        // Looking out out of the destination portal, with the same view as into this portal
        glm::mat4 getViewMatrix(glm::mat4 cameraViewMatrix, Portal *destPortal)
        {
            return (cameraViewMatrix * getTransformMatrix()) 
                * glm::rotate(glm::mat4(1.0), glm::radians(180.0f), glm::vec3(0.0,1.0,0.0)) // Rotate the camara to look out
                * glm::inverse(destPortal->getTransformMatrix());
        }
};