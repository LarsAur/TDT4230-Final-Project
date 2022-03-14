#include <glad/glad.h>
#include <glfw/glfw3.h>
#include <iostream>
#include <shader.hpp>
#include <window.hpp>
#include <camera.hpp>
#include <mesh.hpp>
#include <node.hpp>
#include <texture.hpp>
#include <portal.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

int main()
{
    Window window(1000, 1000, "Portal");

    Camera fpFarCamera(&window, glm::vec3(0), glm::vec3(0), M_PI / 2, 0.1f, 200.0f);
    Camera fpNearCamera(&window, glm::vec3(0), glm::vec3(0), M_PI / 2, 0.001f, 1.0f);

    Shader shader;
    shader.attach("../shaders/test.vert");
    shader.attach("../shaders/test.frag");
    shader.link();
    shader.activate();

    Node root;
    Cube cube(glm::vec3(1.0f, 1.0f, 1.0f), false);
    //ObjMesh chamber("../res/models/map.obj", 10.0f);
    Cube chamber(glm::vec3(30, 20, 10), true);
    ObjMesh portalGun("../res/models/PortalGun.obj", 0.01f);

    Portal portal(glm::vec2(5, 10));
    Portal portal2(glm::vec2(5, 10));
    
    root.addChild(&fpFarCamera);
    root.addChild(&fpNearCamera);
    root.addChild(&cube);
    root.addChild(&chamber);
    root.addChild(&portalGun);
    root.addChild(&portal);
    root.addChild(&portal2);

    Texture portalGunAlbedo("../res/textures/portalgun_col.jpg");
    Texture wall("../res/textures/wall.png");
    Texture portalTexture("../res/textures/blue_portal.png");
    Texture rubix("../res/textures/rubix.jpg");
    portalGun.albedo = &portalGunAlbedo;
    cube.albedo = &rubix;
    chamber.albedo = &wall;
    portal.albedo = &portalTexture;
    portal2.albedo = &portalTexture;

    chamber.generateVertexData(&shader);
    cube.generateVertexData(&shader);
    portalGun.generateVertexData(&shader);
    portal.generateVertexData(&shader);
    portal2.generateVertexData(&shader);

    portalGun.setPosition(glm::vec3(0.007f, -0.005f, -0.01f));
    portalGun.setRotation(glm::vec3(0, -0.3f, 0.1f));

    portal.translate(glm::vec3(0, 0, -4.5f));
    portal2.translate(glm::vec3(10, 0, -4.5f));
    cube.translate(glm::vec3(0, 0, -2));

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_STENCIL_TEST);
    glEnable(GL_CULL_FACE);
	glEnable(GL_MULTISAMPLE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    window.disableCursor();

    float t = 0;

    while (!window.shouldClose())
    {
        t += 0.015f;
        portalGun.setPosition(glm::vec3(0.007f, -0.005f + 0.0005f * sin(t), -0.01f));
        portal.setPosition(glm::vec3(0, 2 * sin(t), -4.5f));

        window.updateInput();
        
        glm::mat4 view = fpFarCamera.getViewMatrix();
        glm::mat4 proj = fpFarCamera.getPerspectiveMatrix();

        int uViewLoc = shader.getUniformLocation("view");
        int uProjLoc = shader.getUniformLocation("proj");

        fpFarCamera.rotateClamp(glm::vec3(-window.getMouseDelta().y / 500, -window.getMouseDelta().x / 500, 0.0f));
        fpFarCamera.cameraTranslate(glm::vec3(
            (window.isKeyDown(GLFW_KEY_D) - window.isKeyDown(GLFW_KEY_A)) / 5.0f,
            (window.isKeyDown(GLFW_KEY_SPACE) - window.isKeyDown(GLFW_KEY_LEFT_SHIFT)) / 5.0f,
            (window.isKeyDown(GLFW_KEY_W) - window.isKeyDown(GLFW_KEY_S)) / 5.0f
        ));


        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        root.updateTransforms();

        glDisable(GL_STENCIL_TEST);
        glUniformMatrix4fv(uProjLoc, 1, GL_FALSE, glm::value_ptr(fpNearCamera.getPerspectiveMatrix()));
        glUniformMatrix4fv(uViewLoc, 1, GL_FALSE, glm::value_ptr(glm::identity<glm::mat4>()));
        portalGun.render();
    
        glEnable(GL_STENCIL_TEST);
        glUniformMatrix4fv(uProjLoc, 1, GL_FALSE, glm::value_ptr(proj));
        
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
        glDepthMask(GL_FALSE);
        glStencilFunc(GL_EQUAL, 1, 0xff);
        glStencilOp(GL_INCR, GL_KEEP, GL_KEEP);
        glClear(GL_STENCIL_BUFFER_BIT);

            glUniformMatrix4fv(uViewLoc, 1, GL_FALSE, glm::value_ptr(view));
            portal.render();

        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glDepthMask(GL_TRUE);
        glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
        glStencilFunc(GL_LEQUAL, 1, 0xFF);
        // Render all 

        glUniformMatrix4fv(uViewLoc, 1, GL_FALSE, glm::value_ptr(portal.getViewMatrix(view, &portal2)));
        cube.render();
        chamber.render();
        // Render all 

        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
        glDepthMask(GL_FALSE);
        glStencilFunc(GL_EQUAL, 1, 0xff);
        glStencilOp(GL_INCR, GL_KEEP, GL_KEEP);
        glClear(GL_STENCIL_BUFFER_BIT);

            glUniformMatrix4fv(uViewLoc, 1, GL_FALSE, glm::value_ptr(view));
            portal2.render();

        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glDepthMask(GL_TRUE);
        glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
        glStencilFunc(GL_LEQUAL, 1, 0xFF);

        glUniformMatrix4fv(uViewLoc, 1, GL_FALSE, glm::value_ptr(portal2.getViewMatrix(view, &portal)));
        cube.render();
        chamber.render();
        
        glDisable(GL_STENCIL_TEST);

        glUniformMatrix4fv(uViewLoc, 1, GL_FALSE, glm::value_ptr(view));
        cube.render();
        portal2.render();
        portal.render();
        chamber.render();

        

        // Render all 



        window.swapBuffers();
        
        if(window.isKeyDown(GLFW_KEY_ESCAPE))
            window.close();
    }

    window.destroy();
    glfwTerminate();
}
