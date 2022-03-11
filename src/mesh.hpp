#pragma once

#include <shader.hpp>
#include <glad/glad.h>
#include <glm/vec3.hpp>
#include <vector>

class Mesh
{
private:
    template <class T>
    void generateAttribute(int location, int elementsPerEntry, std::vector<T> data, bool normalize)
    {
        unsigned int vbo;
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(T), data.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(location, elementsPerEntry, GL_FLOAT, normalize ? GL_TRUE : GL_FALSE, sizeof(T), 0);
        glEnableVertexAttribArray(location);
        vbos.push_back(vbo);
    }

public:
    unsigned int vao, ebo;
    std::vector<unsigned int> vbos;
    std::vector<unsigned int> indices;
    std::vector<glm::vec3> verticies;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec3> textureCoordinates;
    
    Mesh()
    {
        
    }

    void generateVertexData(Shader *shader)
    {
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        if(verticies.size() > 0)
            generateAttribute(shader->getAttributeLocation("position") , 3, verticies, false);

        if(normals.size() > 0)
            generateAttribute(shader->getAttributeLocation("normal"), 3, normals, false);

        if(textureCoordinates.size() > 0)
            generateAttribute(shader->getAttributeLocation("textureCoordinate"), 2, textureCoordinates, false);

        glGenBuffers(1, &ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
    }
};

class Cube : public Mesh{
    private:    
        int faces[6][4] = {
            {2,3,0,1}, // Bottom 
            {4,5,6,7}, // Top 
            {7,5,3,1}, // Right 
            {4,6,0,2}, // Left 
            {5,4,1,0}, // Back 
            {6,7,2,3}, // Front 
        };

    public:
        Cube(glm::vec3 dimensions, bool inside)
        {   
            verticies.resize(8, glm::vec3(0));
            for (int y = 0; y <= 1; y++)
            for (int z = 0; z <= 1; z++)
            for (int x = 0; x <= 1; x++) {
                verticies.at(x+y*4+z*2) = glm::vec3(x*2-1, y*2-1, z*2-1) * 0.5f * dimensions;
            }

            for(int face = 0; face < 6; face++)
            {
                if(inside)
                {

                }
                else
                {
                    indices.push_back(faces[face][0]);
                    indices.push_back(faces[face][1]);
                    indices.push_back(faces[face][3]);
                    indices.push_back(faces[face][0]);
                    indices.push_back(faces[face][2]);
                    indices.push_back(faces[face][3]);
                }
            }
        }
};