#pragma once

#include <glad/glad.h>
#include <string>
#include <iostream>
#include <PerlinNoise.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

typedef enum filter_e
{
    LINEAR,
    NEAREST
} filter_e;

class Texture
{
private:
    unsigned int textureID;

public:
    Texture(std::string path, filter_e filter)
    {
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);
        int width, height, channels;
        unsigned char *imageData = stbi_load(path.c_str(), &width, &height, &channels, 4);

        printf("Loaded: %s using %d channels\n", path.c_str(), channels);

        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RGBA,
            width,
            height,
            0,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            imageData);

        stbi_image_free(imageData);

        switch (filter)
        {
        case LINEAR:
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            break;
        case NEAREST:
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            break;
        }

        glGenerateMipmap(GL_TEXTURE_2D);
    }

    Texture(unsigned int width, unsigned int height, unsigned int octaves, float frequency, unsigned long seed)
    {
        const siv::PerlinNoise perlin{seed};
        unsigned char *imageData = (unsigned char *)malloc(sizeof(char) * width * height);

        const double invFreq = 1.0f / frequency;

        for (unsigned int i = 0; i < height; i++)
        {
            for (unsigned int j = 0; j < width; j++)
            {
                imageData[i * width + j] = (unsigned char) (255 * perlin.octave2D_01(i * invFreq, j * invFreq, octaves));
            }
        }

        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);

        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_R8,
            width,
            height,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            imageData);

        free(imageData);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    void bind(unsigned int textureUnitIndex)
    {
        glBindTextureUnit(textureUnitIndex, textureID);
    }

    void destroy()
    {
        glDeleteTextures(1, &textureID);
    }
};