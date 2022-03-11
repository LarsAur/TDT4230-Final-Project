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
    std::vector<int> indices;
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
            generateAttribute(
                shader->getAttributeLocation("position"),
                3,
                verticies,
                false
            );

        if(verticies.size() > 0)
            generateAttribute(
                shader->getAttributeLocation("normal"),
                3,
                normals,
                false
            );

        if(verticies.size() > 0)
            generateAttribute(
                shader->getAttributeLocation("textureCoordinate"),
                2,
                textureCoordinates,
                false
            );

        glGenBuffers(1, &ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
    }
};

class Cube : public Mesh{

    Cube(int size)
    {
        
    }
};