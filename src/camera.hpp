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
    float mnear;
    float mfar;

    float yaw, pitch;

public:
    Camera(Window &window, glm::vec3 position, float fov, float near, float far)
    {
        setPosition(position);
        mfov = fov;
        mnear = near;
        mfar = far;
        mWindow = &window;
        yaw = 0;
        pitch = 0;
    }

    glm::mat4 getPerspectiveMatrix()
    {
        float aspect = mWindow->getWidth() / (float) mWindow->getHeight();
        return glm::perspective(mfov, aspect, mnear, mfar);
    }

    glm::mat4 getViewMatrix()
    {
        glm::mat4 rot = glm::mat4_cast(glm::conjugate(getOrientation()));
        glm::mat4 pos = glm::translate(glm::mat4(1.0f), -getPosition());
        return rot * pos;
    }

    void cameraTranslate(glm::vec3 translation)
    {
        glm::vec3 upVector = glm::vec3(0, 1, 0);
        glm::vec3 forwardVector = get2DLookingVector();
        glm::vec3 rightVector = glm::cross(forwardVector, upVector);

        mPosition += translation[0] * rightVector + 
                    translation[1] * upVector +
                    translation[2] * forwardVector;
    }

    // Returns a vector in world space based upon the translation given as (right, up, forward)
    glm::vec3 getFirstPersonTranslation(glm::vec3 translation)
    {
        glm::vec3 upVector = glm::vec3(0, 1, 0);
        glm::vec3 forwardVector = get2DLookingVector();
        glm::vec3 rightVector = glm::cross(forwardVector, upVector);

        return translation[0] * rightVector + 
               translation[1] * upVector +
               translation[2] * forwardVector;
    }

    void direct(float dYaw, float dPitch)
    {
        yaw += dYaw;
        pitch += dPitch;
        pitch = std::min(std::max((double) pitch, -M_PI / 2 + 0.1f), M_PI / 2 - 0.1f);

        setOrientation(glm::fquat(1,0,0,0));
        rotate(glm::vec3(0,1,0), yaw);
        rotate(glm::vec3(1,0,0), pitch);
    }

    // TODO: For some reason the x component is inverted
    glm::vec3 get3DLookingVector()
    {
        return glm::vec3(0, 0, -1) * glm::mat3_cast(glm::conjugate(getOrientation()));
    }

    glm::vec3 getUpVector()
    {
        return glm::vec3(0, 1, 0) * glm::mat3_cast(glm::conjugate(getOrientation()));
    }

    // The y (up) component is set to zero
    glm::vec3 get2DLookingVector()
    {
        glm::vec3 look = get3DLookingVector();
        look.y = 0;
        return glm::normalize(look);
    }
};