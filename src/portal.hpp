#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_access.hpp>

#include <camera.hpp>
#include <mesh.hpp>

class Portal : public Circle
{
    public:
        Portal(glm::vec2 dimensions) : Circle(dimensions, 100)
        {

        }

        // Looking out out of the destination portal, with the same view as into this portal
        glm::mat4 getViewMatrix(glm::mat4 cameraViewMatrix, Portal *destPortal)
        {
            return (cameraViewMatrix * getTransformMatrix()) 
                * glm::rotate(glm::identity<glm::mat4>(), glm::radians(180.0f), glm::vec3(0.0,1.0,0.0)) // Rotate the camara to look out
                * glm::inverse(destPortal->getTransformMatrix());
        }

        glm::vec3 getNormal()
        {
            glm::mat4 corrected = glm::rotate(mGlobalTransform, glm::radians(90.0f), glm::vec3(1, 0, 0));
            return glm::mat3(glm::transpose(glm::inverse(corrected))) * glm::vec3(0, 1, 0);
        }

        /* Source: http://www.terathon.com/lengyel/Lengyel-Oblique.pdf */
        glm::mat4 getObliqueProjection(glm::mat4 proj, glm::mat4 view)
        {
            float dist = glm::length(mPosition);
            glm::vec4 clipPlane = glm::inverse(glm::transpose(view)) * glm::vec4(getNormal(), dist);
            
            if(clipPlane.w > 0.0f)
            {  
                return proj;
            }

            float *ptr = glm::value_ptr(proj);
            glm::vec4 q = glm::vec4(
                (glm::sign(clipPlane.x) + ptr[8]) / ptr[0],
                (glm::sign(clipPlane.y) + ptr[9]) / ptr[5],
                -1.0f,
                (1.0f + ptr[10]) / ptr[14]
            );

            glm::vec4 c = clipPlane * 2.0f / glm::dot(clipPlane, q);
            glm::mat4 obliqueProj = glm::row(proj, 2, c - glm::row(proj, 3));

            return obliqueProj;
        }
};