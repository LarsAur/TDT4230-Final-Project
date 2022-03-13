#pragma once

#include <glad/glad.h>
#include <vector>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <glm/gtx/string_cast.hpp>

class Node
{
    protected:
        Node *mParent;
        std::vector<Node*> children;
        glm::vec3 mPosition;
        glm::vec3 mRotation;
        
        glm::mat4 mGlobalTransform;
    public:
        Node()
        {
            setPosition(glm::vec3(0,0,0));
            setRotation(glm::vec3(0,0,0));
            Node *mParent = nullptr;
        }

        void setPosition(glm::vec3 position)
        {
            mPosition = position;
        }
        
        void setRotation(glm::vec3 rotation)
        {
            mRotation = rotation;
        }

        void rotate(glm::vec3 rotation)
        {
            mRotation += rotation;
        }

        void translate(glm::vec3 translation)
        {
            mPosition += translation;
        }

        glm::mat4 getTransformMatrix()
        {
            return mGlobalTransform;
        }

        void addChild(Node *child)
        {
            children.push_back(child);
            child->mParent = this;
        }

        void updateTransforms(glm::mat4 transformMatrix = glm::identity<glm::mat4>())
        {
            transformMatrix = glm::translate(transformMatrix, mPosition);
            transformMatrix = glm::rotate(transformMatrix, mRotation.x, glm::vec3(1, 0, 0));
            transformMatrix = glm::rotate(transformMatrix, mRotation.y, glm::vec3(0, 1, 0));
            transformMatrix = glm::rotate(transformMatrix, mRotation.z, glm::vec3(0, 0, 1));

            mGlobalTransform = transformMatrix;

            for(Node *child : children)
            {
                child->updateTransforms(transformMatrix);
            }
        }
};