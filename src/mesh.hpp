#pragma once

#include <algorithm>
#include <shader.hpp>
#include <node.hpp>
#include <texture.hpp>
#include <glad/glad.h>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <iostream>
#include <sstream>

class Mesh : public Node
{
protected:
    Shader *mShader = nullptr;

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
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> textureCoordinates;
    Texture *albedo = nullptr;

    void generateVertexData(Shader &shader)
    {
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        if (vertices.size() > 0)
            generateAttribute(shader.getAttributeLocation("position"), 3, vertices, false);

        if (normals.size() > 0)
            generateAttribute(shader.getAttributeLocation("normal"), 3, normals, false);

        if (textureCoordinates.size() > 0)
            generateAttribute(shader.getAttributeLocation("textureCoordinate"), 2, textureCoordinates, false);

        glGenBuffers(1, &ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

        mShader = &shader;
    }

    void render()
    {
        int uModlLoc = mShader->getUniformLocation("model");
        glUniformMatrix4fv(uModlLoc, 1, GL_FALSE, glm::value_ptr(getTransformMatrix()));
        glBindVertexArray(vao);

        if (albedo)
        {
            albedo->bind(0);
        }

        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, nullptr);
    }

    void destroy()
    {
        glDeleteBuffers(vbos.size(), vbos.data());
        glDeleteBuffers(1, &ebo);
        glDeleteBuffers(1, &vao);
    }
};

class ObjMesh : public Mesh
{
public:
    ObjMesh(std::string path, float scale)
    {
        // Load mesh from file
        std::ifstream fileStream;
        fileStream.open(path.c_str());
        if (!fileStream)
        {
            std::cerr << "Error: Could not load " << path << std::endl;
        }

        std::string line;
        std::vector<unsigned int> vertIdx, texIdx, normIdx;
        std::vector<glm::vec3> tmpVerts;
        std::vector<glm::vec3> tmpNorm;
        std::vector<glm::vec2> tmpTex;

        while (!fileStream.eof())
        {
            getline(fileStream, line);

            if (line[0] == 'v') // can be geometric vertex, texturecoord, normal or paramter space vertex
            {
                std::string type;
                float v1, v2, v3;
                std::istringstream iss(line);
                iss >> type >> v1 >> v2 >> v3;

                if (type == "v")
                {
                    tmpVerts.push_back(glm::vec3(v1, v2, v3) * scale);
                }
                else if (type == "vt")
                {
                    // Note we are flipping the texture coordinates, instead of flipping the texture
                    tmpTex.push_back(glm::vec2(v1, 1 - v2));
                }
                else if (type == "vn")
                {
                    tmpNorm.push_back(glm::vec3(v1, v2, v3));
                }
                else if (type == "vp")
                {
                    std::cout << "Paramter space vertex used" << std::endl;
                }
            }
            else if (line[0] == 'f') // Faces
            {
                std::string type;
                unsigned int vi1, vi2, vi3, ti1, ti2, ti3, ni1, ni2, ni3;
                std::ptrdiff_t separators = std::count(line.begin(), line.end(), '/');
                std::replace(line.begin(), line.end(), '/', ' ');
                std::istringstream iss(line);

                if (separators == 6)
                {
                    // TODO: This could also include the version without UVs
                    iss >> type >> vi1 >> ti1 >> ni1 >> vi2 >> ti2 >> ni2 >> vi3 >> ti3 >> ni3;
                    normIdx.push_back(ni1);
                    normIdx.push_back(ni2);
                    normIdx.push_back(ni3);
                }
                else if (separators == 3)
                {
                    iss >> type >> vi1 >> ti1 >> vi2 >> ti2 >> vi3 >> ti3;
                }

                vertIdx.push_back(vi1);
                vertIdx.push_back(vi2);
                vertIdx.push_back(vi3);
                texIdx.push_back(ti1);
                texIdx.push_back(ti2);
                texIdx.push_back(ti3);
            }
        }

        for (unsigned int i = 0; i < vertIdx.size(); i++)
        {
            int index = vertIdx[i];
            glm::vec3 vertex = tmpVerts[index - 1]; // .OBJ indexing starts at 1
            vertices.push_back(vertex);
            indices.push_back(i);
        }

        for (int i : texIdx)
        {
            glm::vec2 uv = tmpTex[i - 1]; // .OBJ indexing starts at 1
            textureCoordinates.push_back(uv);
        }

        for (int i : normIdx)
        {
            glm::vec3 normal = tmpNorm[i - 1]; // .OBJ indexing starts at 1
            normals.push_back(normal);
        }
    }
};

class Circle : public Mesh
{
protected:
    glm::vec2 mDimensions;

public:
    Circle(glm::vec2 dimensions, int triangles)
    {
        mDimensions = dimensions;

        for (int i = 0; i <= triangles; i++)
        {
            vertices.push_back(glm::vec3(
                dimensions.x * cos(i * M_PI * 2 / triangles) / 2,
                dimensions.y * sin(i * M_PI * 2 / triangles) / 2,
                0));

            normals.push_back(glm::vec3(
                0,
                0,
                1));

            textureCoordinates.push_back(glm::vec2(
                (cos(i * M_PI * 2 / triangles) + 1) / 2,
                (sin(i * M_PI * 2 / triangles) + 1) / 2));

            indices.push_back(i);
        }
    }

    void render()
    {
        int uModlLoc = mShader->getUniformLocation("model");
        glUniformMatrix4fv(uModlLoc, 1, GL_FALSE, glm::value_ptr(getTransformMatrix()));
        glBindVertexArray(vao);

        if (albedo)
        {
            albedo->bind(0);
        }

        // The circle is drawn using trianglefan
        glDrawElements(GL_TRIANGLE_FAN, indices.size(), GL_UNSIGNED_INT, nullptr);
    }

    glm::vec2 getDimensions()
    {
        return mDimensions;
    }
};

class Cube : public Mesh
{
private:
    int faces[6][4] = {
        {2, 3, 0, 1}, // Bottom
        {4, 5, 6, 7}, // Top
        {7, 5, 3, 1}, // Right
        {4, 6, 0, 2}, // Left
        {5, 4, 1, 0}, // Back
        {6, 7, 2, 3}, // Front
    };

    glm::vec3 norms[6] = {
        glm::vec3(0, 1, 0),
        glm::vec3(0, -1, 0),
        glm::vec3(-1, 0, 0),
        glm::vec3(1, 0, 0),
        glm::vec3(0, 0, 1),
        glm::vec3(0, 0, -1)};

    glm::vec3 mDimension;
    bool mInside;

public:
    Cube(glm::vec3 dimensions, bool inside)
    {
        mInside = inside;
        mDimension = dimensions;

        glm::vec3 verts[8];
        int idx[36];

        for (int y = 0; y <= 1; y++)
            for (int z = 0; z <= 1; z++)
                for (int x = 0; x <= 1; x++)
                {
                    verts[x + y * 4 + z * 2] = glm::vec3(x * 2 - 1, y * 2 - 1, z * 2 - 1) * 0.5f * dimensions;
                }

        glm::vec2 UVs[4] = {
            {0, 0},
            {0, 1},
            {1, 0},
            {1, 1},
        };

        for (int face = 0; face < 6; face++)
        {
            int offset = face * 6;
            if (inside)
            {
                idx[offset + 0] = faces[face][0];
                idx[offset + 1] = faces[face][1];
                idx[offset + 2] = faces[face][3];
                idx[offset + 3] = faces[face][0];
                idx[offset + 4] = faces[face][3];
                idx[offset + 5] = faces[face][2];
            }
            else
            {
                idx[offset + 0] = faces[face][0];
                idx[offset + 1] = faces[face][3];
                idx[offset + 2] = faces[face][1];
                idx[offset + 3] = faces[face][0];
                idx[offset + 4] = faces[face][2];
                idx[offset + 5] = faces[face][3];
            }

            for (int i = 0; i < 6; i++)
            {
                vertices.push_back(verts[idx[offset + i]]);
                normals.push_back(inside ? norms[face] : -norms[face]);
                indices.push_back(offset + i);
            }

            if (inside)
            {
                for (int i : {3, 1, 0, 3, 0, 2})
                {
                    textureCoordinates.push_back(UVs[i]);
                }
            }
            else
            {
                for (int i : {1, 2, 3, 1, 0, 2})
                {
                    textureCoordinates.push_back(UVs[i]);
                }
            }
        }
    }

    // Takes a position and a ray and checks if there is an intersection with the cube.
    // If there is an intersection, the outNormal is the normal of the surface of the cube
    // which is hit, the outIntersection is the position of the intersection. 
    // The return value is whether or not there was an intersection 
    bool isColliding(glm::vec3 position, glm::vec3 ray, glm::vec3 &outNormal, glm::vec3 &outIntersection)
    {
        const float epsilon = 0.001f;
        // Iterate the 6 faces of the cube
        // TODO: This could have an early return in some cases to make it more efficient
        for(int face = 0; face < 6; face++)
        {
            // Normal of the face after rotation
            glm::vec3 normal = getGlobalOrientationMatrix() * norms[face];

            // Calculate the center of the face
            glm::vec3 center = getGlobalPosition();
            glm::vec3 offset = 0.5f * normal * glm::length(mDimension * norms[face]);
            center = center + (mInside ? -offset : offset);

            float normalDotDir = glm::dot(normal, ray);
            if(normalDotDir < 0)
            {
                float t = glm::dot(normal, center - position) / normalDotDir;
                if(t < 1 && t > 0)
                {
                    glm::vec3 intersection = position + ray * t;
                    glm::vec3 localPosition = (intersection - getGlobalPosition()) * glm::inverse(getGlobalOrientationMatrix());
                    glm::vec3 absLocal = glm::abs(localPosition);

                    if(
                        absLocal.x < mDimension.x * 0.5f + epsilon &&
                        absLocal.y < mDimension.y * 0.5f + epsilon &&
                        absLocal.z < mDimension.z * 0.5f + epsilon
                    )
                    {
                        outNormal = normal;
                        outIntersection = intersection;
                        return true;
                    }
                }
            }
        }
        return false;
    }
};