#include <glad/glad.h>
#ifdef _WIN64
    #include <glfw/glfw3.h>
#endif

#ifdef __unix__
    #include <GLFW/glfw3.h>
#endif

#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <iostream>

#include <shader.hpp>
#include <window.hpp>
#include <camera.hpp>
#include <mesh.hpp>
#include <node.hpp>
#include <texture.hpp>
#include <portal.hpp>
#include <game.hpp>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void init(gamedata_st &gamedata);
void update(gamedata_st &gamedata);
void render(gamedata_st &gamedata);
void renderWorld(gamedata_st &gamedata, glm::mat4 view, glm::mat4 proj);
void renderRecursivePortals(gamedata_st &gamedata, glm::mat4 view, glm::mat4 proj, int maxDepth, int depth);
void destroy(gamedata_st &gamedata);

int main()
{
    gamedata_st gamedata;

    init(gamedata);

    while (!gamedata.window->shouldClose())
    {
        render(gamedata);
        update(gamedata);
    }

    destroy(gamedata);
}

void init(gamedata_st &gamedata)
{
    // Create the window
    gamedata.window = new Window(900, 900, "Portal Demo");

    // Load the shader
    gamedata.shader = new Shader();
    gamedata.shader->attach("../shaders/test.vert");
    gamedata.shader->attach("../shaders/test.frag");
    gamedata.shader->link();
    gamedata.shader->activate();

    gamedata.farCamera = new Camera(*gamedata.window, glm::vec3(0), glm::vec3(0), M_PI / 2, 0.1f, 200.0f);
    gamedata.nearCamera = new Camera(*gamedata.window, glm::vec3(0), glm::vec3(0), M_PI / 2, 0.001f, 1.0f);

    gamedata.cube = new Cube(glm::vec3(1.0f, 1.0f, 1.0f), false);
    gamedata.chamber = new Cube(glm::vec3(30, 30, 30), true);
    gamedata.portalGun = new ObjMesh("../res/models/PortalGun.obj", 0.01f);

    gamedata.portals[0] = new Portal(glm::vec2(5, 10));
    gamedata.portals[1] = new Portal(glm::vec2(5, 10));

    gamedata.chamber->generateVertexData(*gamedata.shader);
    gamedata.cube->generateVertexData(*gamedata.shader);
    gamedata.portalGun->generateVertexData(*gamedata.shader);
    gamedata.portals[0]->generateVertexData(*gamedata.shader);
    gamedata.portals[1]->generateVertexData(*gamedata.shader);

    gamedata.root = new Node();
    gamedata.root->addChild(*gamedata.farCamera);
    gamedata.root->addChild(*gamedata.nearCamera);
    gamedata.root->addChild(*gamedata.cube);
    gamedata.root->addChild(*gamedata.chamber);
    gamedata.root->addChild(*gamedata.portalGun);
    gamedata.root->addChild(*gamedata.portals[0]);
    gamedata.root->addChild(*gamedata.portals[1]);

    // Load all textures
    gamedata.portalGunAlbedo = new Texture("../res/textures/portalgun_col.jpg");
    gamedata.wall = new Texture("../res/textures/wall.png");
    gamedata.bluePortalTexture = new Texture("../res/textures/blue_portal.png");
    gamedata.orangeportalTexture = new Texture("../res/textures/orange_portal.png");
    gamedata.rubix = new Texture("../res/textures/rubix.jpg");
    gamedata.portalStencil = new Texture("..res/textures/portal_stencil.png");

    // Assign all initial textures
    gamedata.portalGun->albedo = gamedata.portalGunAlbedo;
    gamedata.cube->albedo = gamedata.rubix;
    gamedata.chamber->albedo = gamedata.wall;
    gamedata.portals[0]->albedo = gamedata.bluePortalTexture;
    gamedata.portals[1]->albedo = gamedata.orangeportalTexture;

    gamedata.portalGun->setPosition(glm::vec3(0.007f, -0.005f, -0.01f));
    gamedata.portalGun->setRotation(glm::vec3(0, -0.3f, 0.1f));

    gamedata.portals[0]->translate(glm::vec3(-10, 0, 0));
    gamedata.portals[0]->rotate(glm::vec3(0, M_PI / 2, 0));

    gamedata.portals[1]->translate(glm::vec3(10, 0, 0));
    gamedata.portals[1]->rotate(glm::vec3(0, -M_PI / 2, 0));
    gamedata.cube->translate(glm::vec3(0, 0, -2));

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_MULTISAMPLE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    gamedata.window->disableCursor();
}

void update(gamedata_st &gamedata)
{
    double time = gamedata.window->getTime();
    gamedata.portalGun->setPosition(glm::vec3(0.007f, -0.005f + 0.0005f * sin(time / 10.0f), -0.01f));
    gamedata.cube->rotate(glm::vec3(0, 0.01f, 0));

    gamedata.window->updateInput();

    if (gamedata.window->isKeyDown(GLFW_KEY_ESCAPE))
        gamedata.window->close();
}

void render(game_st &gamedata)
{
    glm::mat4 view = gamedata.farCamera->getViewMatrix();
    glm::mat4 proj = gamedata.farCamera->getPerspectiveMatrix();

    int uViewLoc = gamedata.shader->getUniformLocation("view");
    int uProjLoc = gamedata.shader->getUniformLocation("proj");

    gamedata.farCamera->rotateClamp(glm::vec3(
        -gamedata.window->getMouseDelta().y / 500,
        -gamedata.window->getMouseDelta().x / 500,
        0.0f));

    gamedata.farCamera->cameraTranslate(glm::vec3(
        (gamedata.window->isKeyDown(GLFW_KEY_D) - gamedata.window->isKeyDown(GLFW_KEY_A)) / 5.0f,
        (gamedata.window->isKeyDown(GLFW_KEY_SPACE) - gamedata.window->isKeyDown(GLFW_KEY_LEFT_SHIFT)) / 5.0f,
        (gamedata.window->isKeyDown(GLFW_KEY_W) - gamedata.window->isKeyDown(GLFW_KEY_S)) / 5.0f));

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    gamedata.root->updateTransforms();

    glUniformMatrix4fv(uProjLoc, 1, GL_FALSE, glm::value_ptr(gamedata.farCamera->getPerspectiveMatrix()));
    glUniformMatrix4fv(uViewLoc, 1, GL_FALSE, glm::value_ptr(view));
    
    renderRecursivePortals(gamedata, view, proj, 5, 0);

    glUniformMatrix4fv(uProjLoc, 1, GL_FALSE, glm::value_ptr(proj));
    glUniformMatrix4fv(uViewLoc, 1, GL_FALSE, glm::value_ptr(view));
    /* glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glDepthMask(GL_TRUE);
    gamedata.portals[0]->render();
    gamedata.portals[1]->render(); */

    /* glDisable(GL_DEPTH_TEST);
    glUniformMatrix4fv(uProjLoc, 1, GL_FALSE, glm::value_ptr(gamedata.nearCamera->getPerspectiveMatrix()));
    glUniformMatrix4fv(uViewLoc, 1, GL_FALSE, glm::value_ptr(glm::identity<glm::mat4>()));
    gamedata.portalGun->render();
 */
    // Render all
    gamedata.window->swapBuffers();
}

void renderRecursivePortals(gamedata_st &gamedata, glm::mat4 view, glm::mat4 proj, int maxDepth, int depth)
{
    int uViewLoc = gamedata.shader->getUniformLocation("view");
    int uProjLoc = gamedata.shader->getUniformLocation("proj");

    for (int i = 0; i < 2; i++)
    {
        Portal *src = gamedata.portals[i];
        Portal *dest = gamedata.portals[(i + 1) % 2];

        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
        glDepthMask(GL_FALSE);
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_STENCIL_TEST);
        glStencilFunc(GL_NOTEQUAL, depth, 0xff);
        glStencilOp(GL_INCR, GL_KEEP, GL_KEEP);
        glStencilMask(0xff);

        glUniformMatrix4fv(uViewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(uProjLoc, 1, GL_FALSE, glm::value_ptr(proj));

        src->render();

        glm::mat4 destView = src->getViewMatrix(view, dest);

        if (depth == maxDepth)
        {
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            glDepthMask(GL_TRUE);
            glClear(GL_DEPTH_BUFFER_BIT);
            glEnable(GL_DEPTH_TEST);
            glEnable(GL_STENCIL_TEST);

            glStencilMask(0x00);
            glStencilFunc(GL_EQUAL, maxDepth + 1, 0xff);

            renderWorld(gamedata, destView, src->getObliqueProjection(proj, view));
        }
        else
        {
            renderRecursivePortals(gamedata, destView, src->getObliqueProjection(proj, destView), maxDepth, depth + 1);
        }

        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
        glDepthMask(GL_FALSE);

        glEnable(GL_STENCIL_TEST);
        glStencilMask(0xff);
        glStencilFunc(GL_NOTEQUAL, depth + 1, 0xff);

        glStencilOp(GL_DECR, GL_KEEP, GL_KEEP);

        glUniformMatrix4fv(uViewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(uProjLoc, 1, GL_FALSE, glm::value_ptr(proj));
        src->render();
    }

    glDisable(GL_STENCIL_TEST);
    glStencilMask(0x00);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_ALWAYS);
    glClear(GL_DEPTH_BUFFER_BIT);

    for (int i = 0; i < 2; i++)
    {
        glUniformMatrix4fv(uProjLoc, 1, GL_FALSE, glm::value_ptr(proj));
        glUniformMatrix4fv(uViewLoc, 1, GL_FALSE, glm::value_ptr(view));
        gamedata.portals[i]->render();
    }

    // Reset the depth function to the default
    glDepthFunc(GL_LESS);

    // Enable stencil test and disable writing to stencil buffer
    glEnable(GL_STENCIL_TEST);
    glStencilMask(0x00);

    // Draw at stencil >= recursionlevel
    // which is at the current level or higher (more to the inside)
    // This basically prevents drawing on the outside of this level.
    glStencilFunc(GL_LEQUAL, depth, 0xFF);

    // Enable color and depth drawing again
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glDepthMask(GL_TRUE);

    // And enable the depth test
    glEnable(GL_DEPTH_TEST);

    // Draw scene objects normally, only at recursionLevel
    renderWorld(gamedata, view, proj);
}

void renderWorld(gamedata_st &gamedata, glm::mat4 view, glm::mat4 proj)
{
    int uViewLoc = gamedata.shader->getUniformLocation("view");
    int uProjLoc = gamedata.shader->getUniformLocation("proj");

    glUniformMatrix4fv(uViewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(uProjLoc, 1, GL_FALSE, glm::value_ptr(proj));

    gamedata.cube->render();
    gamedata.chamber->render();
}

void destroy(gamedata_st &gamedata)
{
    gamedata.window->destroy();
    glfwTerminate();
}