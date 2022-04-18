#pragma once

#include <algorithm>
#include <window.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/gtx/euler_angles.hpp> 
#include <glm/gtc/quaternion.hpp>
#include <node.hpp>

#include <glm/gtx/string_cast.hpp>

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

class Camera : public Node
{
private:
    Window *mWindow;
    float mfov;
    float mNear;
    float mFar;
    float mYaw;
    float mPitch;

public:
    Camera(Window &window, glm::vec3 position, float fov, float near, float far)
    {
        setPosition(position);
        mfov = fov;
        mNear = near;
        mFar = far;
        mWindow = &window;
        mYaw = 0;
        mPitch = 0;
    }

    glm::mat4 getPerspectiveMatrix()
    {
        // Handle the possibility of having 0 and inf aspect ratio
        float aspect = 1;
        float w = mWindow->getWidth();
        float h = mWindow->getHeight();
        if(w != 0 && h != 0)
        {
            aspect = w / h;
        }
        return glm::perspective(mfov, aspect, mNear, mFar);
    }

    glm::mat4 getViewMatrix()
    {
        glm::mat4 rot = glm::mat4_cast(glm::conjugate(getOrientation()));
        glm::mat4 pos = glm::translate(glm::mat4(1.0f), -getPosition());
        return rot * pos;
    }

    // Translates the camera based on the viewing direction 
    // translation = [right, up, forward] in view space
    void translateCamera(glm::vec3 translation)
    {
        glm::vec3 upVector = glm::vec3(0, 1, 0);
        glm::vec3 forwardVector = get2DLookingVector();
        glm::vec3 rightVector = glm::cross(forwardVector, upVector);

        mPosition += translation[0] * rightVector + 
                    translation[1] * upVector +
                    translation[2] * forwardVector;
    }

    // Returns the world space translation vector based on the viewing direction
    // translation = [right, up, forward] in view space 
    glm::vec3 getCameraTranslation(glm::vec3 translation)
    {
        glm::vec3 upVector = glm::vec3(0, 1, 0);
        glm::vec3 forwardVector = get2DLookingVector();
        glm::vec3 rightVector = glm::cross(forwardVector, upVector);

        return translation[0] * rightVector + 
               translation[1] * upVector +
               translation[2] * forwardVector;
    }

    // Directs the viewing direction of the camera by delta yaw and delta pitch
    // The pitch will be clamped between - PI / 2 and PI / 2
    void direct(float dYaw, float dPitch)
    {
        mYaw += dYaw;
        mPitch += dPitch;
        mPitch = std::min(std::max((double) mPitch, -M_PI / 2 + 0.1f), M_PI / 2 - 0.1f);

        setOrientation(glm::fquat(1,0,0,0));
        rotate(glm::vec3(0,1,0), mYaw);
        rotate(glm::vec3(1,0,0), mPitch);
    }

    // Get the forward direction which the camera is facing in world space
    glm::vec3 get3DLookingVector()
    {
        return glm::vec3(0, 0, -1) * glm::mat3_cast(glm::conjugate(getOrientation()));
    }

    // Get the up direction of the camera in world space
    glm::vec3 getUpVector()
    {
        return glm::vec3(0, 1, 0) * glm::mat3_cast(glm::conjugate(getOrientation()));
    }

    // Get the normalized looking vector in the xz-plane (y = 0)
    glm::vec3 get2DLookingVector()
    {
        glm::vec3 look = get3DLookingVector();
        look.y = 0;
        return glm::normalize(look);
    }
};