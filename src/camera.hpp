#pragma once

#include <algorithm>
#include <window.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <node.hpp>


#include <glm/gtx/string_cast.hpp>


class Camera : private Node
{
private:
    Window *mWindow;
    glm::vec3 mAngles;
    float mfov;
    float mnear;
    float mfar;

public:
    Camera(Window *window, glm::vec3 position, glm::vec3 angles, float fov, float near, float far)
    {
        setPosition(position);
        mAngles = angles;
        mfov = fov;
        mnear = near;
        mfar = far;
        mWindow = window;
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
        
        camera = glm::rotate(camera, mAngles[0], upVector);
        camera = glm::rotate(camera, mAngles[1], pitchVector);
        camera = glm::translate(camera, mPosition);
        
        glm::mat4 view = glm::inverse(camera);
        return view;
    }

    void translate(glm::vec3 deltaPosition)
    {
        mPosition += deltaPosition;
    }

    void rotate(glm::vec3 deltaAngles)
    {
        mAngles += deltaAngles;
    }

    void rotateClamp(glm::vec3 deltaAngles)
    {
        mAngles += deltaAngles;
        mAngles[1] = std::min(std::max((double) mAngles[1], -M_PI / 2 + 0.1f), M_PI / 2 - 0.1f);
    }
};