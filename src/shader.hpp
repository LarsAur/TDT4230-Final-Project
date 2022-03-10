#pragma once

#include <glad/glad.h>

#include <fstream>
#include <memory>

class Shader {
    private:
        unsigned int mProgramID;
        
        int shaderTypeFromPath(std::string path)
        {
            int dotIndex = path.rfind(".");
            std::string ext = path.substr(dotIndex + 1);

            if(ext == "vert") return GL_VERTEX_SHADER;
            if(ext == "frag") return GL_FRAGMENT_SHADER;
            
            return -1;
        }

    public:
        Shader() 
        {
            mProgramID = glCreateProgram();
        }

        void activate() 
        {
            glUseProgram(mProgramID);
        }

        unsigned int getProgramID()
        {
            return mProgramID;
        }

        void destroy()
        {
            glDeleteProgram(mProgramID);
        }

        void attach(std::string path)
        {
            // Load shader from file
            std::ifstream fileStream;
            fileStream.open(path.c_str());
            if(!fileStream)
            {
                std::cerr << "Error: Could not load " << path << std::endl;
            }

            std::string shaderCode = std::string(
                std::istreambuf_iterator<char>(fileStream), 
                std::istreambuf_iterator<char>()
            );

            const char *shaderCodePtr = shaderCode.c_str();

            // Find the shader type from the file extention
            int shaderType = shaderTypeFromPath(path);
            if(shaderType == -1)
            {
                std::cerr << "Error: " << path << " has no corresponding shader type" << std::endl;
                return;
            }

            // Create the shader and compile it
            int shader = glCreateShader(shaderType);
            glShaderSource(shader, 1, &shaderCodePtr, nullptr);
            glCompileShader(shader);
            
            // Display the error message if any
            int compileStatus;
            glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);
            if (!compileStatus)
            {
                int length;
                glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
                std::unique_ptr<char[]> buffer(new char[length]);
                glGetShaderInfoLog(shader, length, nullptr, buffer.get());
                std::cerr << path << std::endl;
                std::cerr << buffer.get() << std::endl;
                glDeleteShader(shader);
                return;
            }

            // Attach and free the shader
            glAttachShader(mProgramID, shader);
            glDeleteShader(shader);
        }

        void link()
        {
            glLinkProgram(mProgramID);

            // Display errors
            int linkStatus;
            glGetProgramiv(mProgramID, GL_LINK_STATUS, &linkStatus);
            if (!linkStatus)
            {
                int length;
                glGetProgramiv(mProgramID, GL_INFO_LOG_LENGTH, &length);
                std::unique_ptr<char[]> buffer(new char[length]);
                glGetProgramInfoLog(mProgramID, length, nullptr, buffer.get());
                std::cerr << buffer.get() << std::endl;
            }
        }

        int getUniformLocation(const char* name)
        {
            return glGetUniformLocation(mProgramID, name);
        }
};