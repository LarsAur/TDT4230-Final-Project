#pragma once

#include <vector>
#include <glm/vec3.hpp>

class Node
{
    protected:
        glm::vec3 mPosition;
        Node *mParent;
        std::vector<Node*> children;

    public:
        Node()
        {
        }

        void setPosition(glm::vec3 position)
        {
            mPosition = position;
        }

        void addChild(Node *child)
        {
            children.push_back(child);
            child->mParent = this;
        }

};