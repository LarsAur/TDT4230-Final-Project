#pragma once

#include <glad/glad.h>
#include <vector>
#include <glm/vec3.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtx/string_cast.hpp>

class Node
{
    protected:
        std::vector<Node*> children;
        glm::vec3 mPosition;
        glm::fquat mOrientation;
        
        glm::mat4 mGlobalTransform;
    public:
        Node()
        {
            setPosition(glm::vec3(0,0,0));
            setOrientation(glm::fquat(1,0,0,0));
        }

        void setPosition(glm::vec3 position)
        {
            mPosition = position;
        }

        glm::vec3 getPosition()
        {
            return mPosition;
        }
        
        void setOrientation(glm::fquat orientation)
        {
            mOrientation = orientation;
        }

        glm::fquat getOrientation()
        {
            return mOrientation;
        }

        void rotate(glm::vec3 axis, float angle)
        {
            mOrientation = glm::rotate(mOrientation, angle, axis);
        }

        void rotate(glm::vec3 eulerAngles)
        {
            glm::fquat rot = glm::fquat(1, 0, 0, 0);
            rot = glm::rotate(rot, eulerAngles.y, glm::vec3(0,1,0));
            rot = glm::rotate(rot, eulerAngles.x, glm::vec3(1,0,0));
            rot = glm::rotate(rot, eulerAngles.z, glm::vec3(0,0,1));

            mOrientation = mOrientation * rot;
        }

        void translate(glm::vec3 translation)
        {
            mPosition += translation;
        }

        glm::mat4 getTransformMatrix()
        {
            return mGlobalTransform;
        }

        void addChild(Node &child)
        {
            children.push_back(&child);
        }

        void updateTransforms(glm::mat4 transformMatrix = glm::identity<glm::mat4>())
        {
            transformMatrix = glm::translate(transformMatrix, mPosition) * (glm::mat4) mOrientation;

            mGlobalTransform = transformMatrix;

            for(Node *child : children)
            {
                child->updateTransforms(transformMatrix);
            }
        }

        glm::vec3 getGlobalPosition()
        {
            return glm::column(mGlobalTransform, 3);
        }

        glm::mat3 getGlobalOrientationMatrix()
        {
            return glm::mat3(mGlobalTransform);
        }
        
};