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
                * glm::rotate(glm::mat4(1.0), glm::radians(180.0f), glm::vec3(0.0,1.0,0.0)) // Rotate the camara to look out
                * glm::inverse(destPortal->getTransformMatrix());
        }

        glm::vec3 getNormal()
        {
            return glm::vec3(
                sin(mRotation.y) * sin(mRotation.x),
                cos(mRotation.x),
                cos(mRotation.y) * sin(mRotation.x)
            );
        }

        /* Source: http://www.terathon.com/lengyel/Lengyel-Oblique.pdf */
        glm::mat4 getObliqueProjection(glm::mat4 proj, glm::mat4 view)
        {
            glm::vec3 clipNormal = -getNormal();
            float cw = glm::dot(clipNormal, mPosition);
            glm::vec4 clipPlane = glm::inverse(glm::transpose(view)) * glm::vec4(clipNormal, cw);
            //glm::mat4 invProj = glm::inverse(proj);
            
            if(clipPlane.w > 0)
            {
                //printf("X\n");
                return proj;
            }
                //printf("#\n");

            glm::vec4 q = glm::vec4(
                glm::sign(clipPlane.x),
                glm::sign(clipPlane.y),
                1.0f,
                1.0f
            );

            glm::vec4 c = clipPlane * 2.0f / glm::dot(clipPlane, q);
            glm::mat4 obliqueProj = glm::row(proj, 2, c - glm::row(proj, 3));

            return obliqueProj;
        }
};