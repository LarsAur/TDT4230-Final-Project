#include <glad/glad.h>
#include <glfw/glfw3.h>
#include <iostream>

#include <shader.hpp>
#include <window.hpp>
#include <camera.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

float vertices[] = {
    -0.5f, -0.5f, 1.0f, // left
    0.5f, -0.5f, 1.0f,  // right
    0.0f, 0.5f, 1.0f    // top
};

int main()
{
    Window window(1000, 1000, "Portal");

    Camera fpCamera(&window, glm::vec3(0), glm::vec3(0), M_PI / 2, 0.1f, 5.0f);

    Shader shader;
    shader.attach("../shaders/test.vert");
    shader.attach("../shaders/test.frag");
    shader.link();
    shader.activate();

    unsigned int vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    int positionLocation = glGetAttribLocation(shader.getProgramID(), "position");

    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(positionLocation);

    window.disableCursor();

    while (!window.shouldClose())
    {
        window.updateInput();
        
        glm::mat4 view = fpCamera.getViewMatrix();
        glm::mat4 proj = fpCamera.getPerspectiveMatrix();

        int uViewLoc = shader.getUniformLocation("view");
        int uProjLoc = shader.getUniformLocation("proj");

        fpCamera.rotateClamp(glm::vec3(-window.getMouseDelta().x / 500, -window.getMouseDelta().y / 500, 0.0f));
        fpCamera.translate(glm::vec3(
            (window.isKeyDown(GLFW_KEY_D) - window.isKeyDown(GLFW_KEY_A)) / 500.0f,
            0.0f,
            (window.isKeyDown(GLFW_KEY_W) - window.isKeyDown(GLFW_KEY_S)) / 500.0f
        ));

        glUniformMatrix4fv(uViewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(uProjLoc, 1, GL_FALSE, glm::value_ptr(proj));

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        window.swapBuffers();
        
        if(window.isKeyDown(GLFW_KEY_ESCAPE))
            window.close();
    }

    window.destroy();
    glfwTerminate();
}
