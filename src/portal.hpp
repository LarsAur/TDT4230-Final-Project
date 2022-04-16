#pragma once

#include <glad/glad.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_access.hpp>

#include <camera.hpp>
#include <mesh.hpp>

class Portal : public Circle
{
private:
    glm::vec3 mColor;

public:
    Portal(glm::vec2 dimensions, glm::vec3 color) : Circle(dimensions, 100)
    {
        mColor = color;
    }

    void render()
    {
        int uIsPortalLoc = mShader->getUniformLocation("u_is_portal");
        int uPortalColorLoc = mShader->getUniformLocation("u_portal_color");
        glUniform1i(uIsPortalLoc, 1);
        glUniform3fv(uPortalColorLoc, 1, glm::value_ptr(mColor));
        Circle::render();
        glUniform1i(uIsPortalLoc, 0);
    }

    // Takes a node, the translation of the node and destination portal as input parameters.
    // If the node is passing through this portal, the object is teleported to the destination portal with the correct offsets
    bool passthrough(Camera &node, glm::vec3 translation, Portal &destination)
    {
        float normalDotDir = glm::dot(getNormal(), translation);
        if(normalDotDir < 0) // Use less than 0 because we only want to enter the portal from the front
        {
            float t = glm::dot(getNormal(), node.getPosition() - getPosition()) / -normalDotDir;
            if(t < 1 && t > 0)
            {
                glm::mat3 rotation = glm::mat3_cast(glm::conjugate(getOrientation()));
                
                glm::vec3 up = glm::vec3(0, 1, 0) * rotation;
                glm::vec3 right = glm::vec3(1, 0, 0) * rotation;
                glm::vec3 norm = glm::vec3(0, 0, 1) * rotation;

                // Find the linear combination coefficients (a,b) of the vectors u and v in the portal plane.
                // a * u + b * v = transform * t
                glm::mat3 inv = glm::inverse(glm::mat3(up, right, norm));
                glm::vec3 ab = inv * (node.getPosition() + translation * t - getPosition());
                // Check if a and b are within the oval of the portal 
                if(pow(2 * ab[0] / mDimensions.y, 2) + pow(2 * ab[1] / mDimensions.x, 2) <= 1)
                {
                    // First calculate the position relative to the src portal
                    // Rotate it into the xy-plane, and filp the x-axis
                    // At last, rotate into the plane of the destination portal
                    glm::vec3 srcDeltaPos = (node.getGlobalPosition() + translation * t) - getGlobalPosition();
                    glm::vec3 neutralDeltaPos = srcDeltaPos * glm::inverse(rotation);
                    neutralDeltaPos.x *= -1;
                    glm::vec3 destDeltaPos = neutralDeltaPos * glm::mat3_cast(glm::conjugate(destination.getOrientation()));

                    // Calulate the yaw rotation (around the y axis)
                    glm::vec3 srcNormal = getNormal();
                    srcNormal.y = 0;
                    srcNormal = glm::normalize(srcNormal);
                    float srcYaw = (float) atan2(srcNormal.x, srcNormal.z);
                    glm::vec3 destNormal = destination.getNormal();
                    destNormal.y = 0;
                    destNormal = glm::normalize(destNormal);
                    float destYaw = (float) atan2(destNormal.x, destNormal.z);
                    float deltaYaw = destYaw - srcYaw + M_PI;
                    
                    if(glm::length(srcNormal) == 0 || glm::length(destNormal) == 0)
                    {
                        deltaYaw = 0;
                    }

                    // Calculate delta pitch
                    srcNormal = getNormal();
                    destNormal = destination.getNormal();
                    float deltaPitch = (float) asin(srcNormal.y) - asin(destNormal.y) ;

                    node.setPosition(destination.getPosition() + destDeltaPos);
                    node.direct(deltaYaw, deltaPitch);
                    return true;
                }
            }
        }

        return false;
    }

    // Place the portal given a normal vector, up vector and position
    void place(glm::vec3 targetPosition, glm::vec3 targetNormal, glm::vec3 targetUp)
    {
        targetNormal = glm::normalize(targetNormal);
        targetUp = glm::normalize(targetUp);

        setOrientation(glm::fquat(1,0,0,0));
        setPosition(targetPosition);
        
        // Align the normal of the wall to the normal of the portal
        glm::vec3 normal = getNormal();
        float dot = glm::dot(normal, targetNormal);
        float angle = acos(dot);
        glm::vec3 axis = glm::cross(normal, targetNormal);

        if(dot > 0.99)
        {
            angle = 0;
            axis = glm::vec3(1,0,0);
        }
        else if(dot < -0.99)
        {
            angle = M_PI;
            axis = targetUp;
        }
        rotate(axis, angle);

        // Align the up direction of the portal, to the up vector defined by the ray-casts
        glm::vec3 up = getUp();
        dot = glm::dot(up, targetUp);
        angle = acos(dot);
        axis = glm::cross(up, targetUp) * getOrientation();

        if(dot > 0.99)
        {
            angle = 0;
            axis = glm::vec3(1,0,0);
        }
        else if(dot < -0.99)
        {
            angle = M_PI;
            axis = targetNormal;
        }
        rotate(axis, angle);
    }

    // Looking out out of the destination portal, with the same view as into this portal
    glm::mat4 getViewMatrix(glm::mat4 viewMatrix, Portal *destPortal)
    {
        return viewMatrix * getTransformMatrix() * glm::rotate(glm::identity<glm::mat4>(), glm::radians(180.0f), glm::vec3(0.0, 1.0, 0.0)) // Rotate the camara to look out
               * glm::inverse(destPortal->getTransformMatrix());
    }

    glm::vec3 getNormal()
    {
        return glm::vec3(0, 0, 1) * glm::mat3_cast(glm::conjugate(getOrientation()));
    }

    glm::vec3 getUp()
    {
        return glm::vec3(0, 1, 0) * glm::mat3_cast(glm::conjugate(getOrientation()));
    }

    /* Source: http://www.terathon.com/lengyel/Lengyel-Oblique.pdf */
    glm::mat4 getObliqueProjection(glm::mat4 proj, glm::mat4 view)
    {
        glm::vec3 normal = getNormal();
        float d = -glm::dot(normal, mPosition);
        glm::vec4 clipPlane = glm::inverse(glm::transpose(view)) * glm::vec4(normal, d);
            
        if (clipPlane.w > 0.0f)
        {
            return proj;
        }

        float *ptr = glm::value_ptr(proj);
        glm::vec4 q = glm::vec4(
            (glm::sign(clipPlane.x) + ptr[8]) / ptr[0],
            (glm::sign(clipPlane.y) + ptr[9]) / ptr[5],
            -1.0f,
            (1.0f + ptr[10]) / ptr[14]);

        glm::vec4 c = clipPlane * 2.0f / glm::dot(clipPlane, q);
        glm::mat4 obliqueProj = glm::row(proj, 2, c - glm::row(proj, 3));

        return obliqueProj;
    }
};