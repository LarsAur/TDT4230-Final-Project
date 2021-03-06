#pragma once

#include <glm/gtc/type_ptr.hpp>
#include <glm/vec3.hpp>
#include <shader.hpp>
#include <node.hpp>

static int id = 0;

class Light : public Node
{
    private:
        glm::vec3 mColor;
        int mID;
    public:
    Light(glm::vec3 position, glm::vec3 color)
    {
        mPosition = position;
        mColor = color;

        // Create an unique ID for each light, 
        // this ID is used as the index in the uniform array in the shader
        mID = id;
        id++;

        printf("Light ID: %d\n", mID);
    }

    glm::vec3 getColor()
    {
        return mColor;
    }

    // Send the color and position of the light to the fragment shader
    void updateUniform(Shader &shader)
    {
        int uPositionLoc = shader.getUniformLocation("u_light_positions") + mID;
        glUniform3fv(uPositionLoc, 1, glm::value_ptr(getGlobalPosition()));

        int uColorLoc = shader.getUniformLocation("u_light_colors") + mID;
        glUniform3fv(uColorLoc, 1, glm::value_ptr(getColor()));
    }
};