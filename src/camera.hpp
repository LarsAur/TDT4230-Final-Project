#pragma once

#include <algorithm>
#include <window.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
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


public:
    Camera(Window &window, glm::vec3 position, glm::vec3 rotation, float fov, float near, float far)
    {
        setPosition(position);
        setRotation(rotation);
        mfov = fov;
        mnear = near;
        mfar = far;
        mWindow = &window;
    }

    void setNear(float near)
    {
        
    }

    glm::mat4 getPerspectiveMatrix()
    {
        float aspect = mWindow->getWidth() / (float) mWindow->getHeight();
        return glm::perspective(mfov, aspect, mnear, mfar);
    }

    glm::mat4 getViewMatrix()
    {
        glm::vec3 upVector = glm::vec3(0, 1, 0);
        glm::vec3 pitchVector = glm::vec3(1, 0, 0);
        glm::mat4 camera = glm::identity<glm::mat4>();
        
        camera = glm::translate(camera, mPosition);
        camera = glm::rotate(camera, mRotation.y, upVector);
        camera = glm::rotate(camera, mRotation.x, pitchVector);
        
        glm::mat4 view = glm::inverse(camera);
        return view;
    }

    void cameraTranslate(glm::vec3 translation)
    {
        glm::vec3 upVector = glm::vec3(0, 1, 0);
        glm::vec3 forwardVector = get2DLookingVector();
        glm::vec3 rightVector = glm::cross(forwardVector, upVector);

        mPosition+= translation[0] * rightVector + 
                    translation[1] * upVector +
                    translation[2] * forwardVector;
    }

    void rotateClamp(glm::vec3 rotation)
    {
        mRotation += rotation;
        mRotation.x = std::min(std::max((double) mRotation.x, -M_PI / 2 + 0.1f), M_PI / 2 - 0.1f);
    }

    glm::vec3 get3DLookingVector()
    {
        return glm::vec3(
            sin(mRotation.y) * sin(mRotation.x),
            cos(mRotation.x),
            cos(mRotation.y) * sin(mRotation.x)
        );
    }

    // The y (up) component is set to zero
    glm::vec3 get2DLookingVector()
    {
        return -glm::vec3(
            sin(mRotation.y),
            0,
            cos(mRotation.y)
        );
    }
};