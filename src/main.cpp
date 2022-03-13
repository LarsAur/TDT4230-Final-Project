#include <glad/glad.h>
#include <glfw/glfw3.h>
#include <iostream>

#include <shader.hpp>
#include <window.hpp>
#include <camera.hpp>
#include <mesh.hpp>
#include <node.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

int main()
{
    Window window(1000, 1000, "Portal");

    Camera fpCamera(&window, glm::vec3(0), glm::vec3(0), M_PI / 2, 0.001f, 150.0f);

    Shader shader;
    shader.attach("../shaders/test.vert");
    shader.attach("../shaders/test.frag");
    shader.link();
    shader.activate();

    Node root;
    Cube cube(glm::vec3(100.0f, 100.0f, 100.0f), true);
    ObjMesh chamber("../res/models/map.obj", 10.0f);
    ObjMesh portalGun("../res/models/PortalGun.obj", 0.01f);
    
    root.addChild(&fpCamera);
    root.addChild(&cube);
    root.addChild(&chamber);
    root.addChild(&portalGun);

    chamber.generateVertexData(&shader);
    cube.generateVertexData(&shader);
    portalGun.generateVertexData(&shader);

    portalGun.setPosition(glm::vec3(0.007f, -0.005f, -0.01f));
    portalGun.setRotation(glm::vec3(0, -0.3f, 0.1f));

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
	glEnable(GL_MULTISAMPLE);

    window.disableCursor();

    float t = 0;

    while (!window.shouldClose())
    {
        t += 0.015f;
        portalGun.setPosition(glm::vec3(0.007f, -0.005f + 0.0005f * sin(t), -0.01f));

        window.updateInput();
        
        glm::mat4 view = fpCamera.getViewMatrix();
        glm::mat4 proj = fpCamera.getPerspectiveMatrix();

        int uViewLoc = shader.getUniformLocation("view");
        int uProjLoc = shader.getUniformLocation("proj");
        int uModlLoc = shader.getUniformLocation("model");

        fpCamera.rotateClamp(glm::vec3(-window.getMouseDelta().y / 500, -window.getMouseDelta().x / 500, 0.0f));
        fpCamera.cameraTranslate(glm::vec3(
            (window.isKeyDown(GLFW_KEY_D) - window.isKeyDown(GLFW_KEY_A)) / 5.0f,
            (window.isKeyDown(GLFW_KEY_SPACE) - window.isKeyDown(GLFW_KEY_LEFT_SHIFT)) / 5.0f,
            (window.isKeyDown(GLFW_KEY_W) - window.isKeyDown(GLFW_KEY_S)) / 5.0f
        ));

        glUniformMatrix4fv(uProjLoc, 1, GL_FALSE, glm::value_ptr(proj));

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        root.updateTransforms();

        glUniformMatrix4fv(uViewLoc, 1, GL_FALSE, glm::value_ptr(glm::identity<glm::mat4>()));
        glUniformMatrix4fv(uModlLoc, 1, GL_FALSE, glm::value_ptr(portalGun.getTransformMatrix()));
        portalGun.render();
        
        glUniformMatrix4fv(uViewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(uModlLoc, 1, GL_FALSE, glm::value_ptr(cube.getTransformMatrix()));
        //cube.render();

        glUniformMatrix4fv(uModlLoc, 1, GL_FALSE, glm::value_ptr(chamber.getTransformMatrix()));
        chamber.render();

        window.swapBuffers();
        
        if(window.isKeyDown(GLFW_KEY_ESCAPE))
            window.close();
    }

    window.destroy();
    glfwTerminate();
}
