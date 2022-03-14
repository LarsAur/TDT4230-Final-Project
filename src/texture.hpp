#pragma once

#include <string>
#include <glad/glad.h>
#include <SOIL/SOIL.h>

class Texture
{
    private:
        unsigned int textureID;
    public:
        Texture(std::string path)
        {
            glGenTextures(1, &textureID);
            glBindTexture(GL_TEXTURE_2D, textureID);
            int width, height;
            unsigned char* imageData = SOIL_load_image(path.c_str(), &width, &height, 0, SOIL_LOAD_RGBA);

            glTexImage2D(
                GL_TEXTURE_2D,
                0,
                GL_RGBA,
                width,
                height,
                0,
                GL_RGBA,
                GL_UNSIGNED_BYTE,
                imageData
            );

            SOIL_free_image_data(imageData);

            glGenerateMipmap(GL_TEXTURE_2D);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }

        void bind(unsigned int textureUnitIndex)
        {
            glBindTextureUnit(textureUnitIndex, textureID);
        }
};