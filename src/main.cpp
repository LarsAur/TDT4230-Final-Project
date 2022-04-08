#include <glad/glad.h>
#ifdef _WIN64
#include <glfw/glfw3.h>
#endif

#ifdef __unix__
#include <GLFW/glfw3.h>
#endif

#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <iostream>

#include <game.hpp>
#include <shader.hpp>
#include <window.hpp>
#include <camera.hpp>
#include <mesh.hpp>
#include <node.hpp>
#include <texture.hpp>
#include <portal.hpp>
#include <light.hpp>

void init(gamedata_st &gamedata);
void update(gamedata_st &gamedata);
void render(gamedata_st &gamedata);
void renderWorld(gamedata_st &gamedata, glm::mat4 view, glm::mat4 proj);
void renderRecursivePortals(gamedata_st &gamedata, glm::mat4 view, glm::mat4 proj, int maxDepth);
void destroy(gamedata_st &gamedata);

int main()
{
    gamedata_st gamedata;

    init(gamedata);

    double prevTime = 0, time;
    int frames = 0;
    while (!gamedata.window->shouldClose())
    {
        // Print the fps every 10 seconds
        time = gamedata.window->getTime();
        if(time - prevTime >= 10.0)
        {
            printf("FPS: %f, (ms per frame: %f)\n", frames / (time - prevTime), (time - prevTime) / frames);
            frames = 0;
            prevTime = time;
        }

        // Run render and update loop
        update(gamedata);
        render(gamedata);
        frames++;
    }

    destroy(gamedata);
}

void init(gamedata_st &gamedata)
{
    // Create the window
    gamedata.window = new Window(900, 900, "Portal Demo");

    // Load the shader
    gamedata.shader = new Shader();
    gamedata.shader->attach("../shaders/shader.vert");
    gamedata.shader->attach("../shaders/shader.frag");
    gamedata.shader->link();
    gamedata.shader->activate();

    gamedata.farCamera = new Camera(*gamedata.window, glm::vec3(0), M_PI / 2, 0.001f, 150.0f);
    gamedata.nearCamera = new Camera(*gamedata.window, glm::vec3(0), M_PI / 2, 0.001f, 1.0f);

    gamedata.cube = new Cube(glm::vec3(5.0f, 5.0f, 5.0f), false);
    gamedata.chamber = new Cube(glm::vec3(30, 30, 30), true);
    gamedata.portalGun = new ObjMesh("../res/models/PortalGunNormals.obj", 0.01f);
    gamedata.player = new Cube(glm::vec3(1.0f, 1.0f, 1.0f), false);
    gamedata.portals[0] = new Portal(glm::vec2(5, 10), glm::vec3(0.36f, 0.58f, 1.0f));
    gamedata.portals[1] = new Portal(glm::vec2(5, 10), glm::vec3(1.0f, 0.5f, 0.05f));

    gamedata.chamber->generateVertexData(*gamedata.shader);
    gamedata.cube->generateVertexData(*gamedata.shader);
    gamedata.portalGun->generateVertexData(*gamedata.shader);
    gamedata.portals[0]->generateVertexData(*gamedata.shader);
    gamedata.portals[1]->generateVertexData(*gamedata.shader);
    gamedata.player->generateVertexData(*gamedata.shader);

    gamedata.root = new Node();
    gamedata.root->addChild(*gamedata.farCamera);
    gamedata.root->addChild(*gamedata.nearCamera);
    gamedata.root->addChild(*gamedata.cube);
    gamedata.root->addChild(*gamedata.chamber);
    gamedata.root->addChild(*gamedata.portalGun);
    gamedata.root->addChild(*gamedata.portals[0]);
    gamedata.root->addChild(*gamedata.portals[1]);
    gamedata.farCamera->addChild(*gamedata.player);

    // Add lights
    gamedata.lights[0] = new Light(glm::vec3(0,5,0), glm::vec3(0.3, 0.3, 0.3));
    gamedata.lights[1] = new Light(glm::vec3(0, 0, 0.5), glm::vec3(0.36, 0.58, 1.0));
    gamedata.lights[2] = new Light(glm::vec3(0, 0, 0.5), glm::vec3(1.0, 0.5, 0.05));
    gamedata.portals[0]->addChild(*gamedata.lights[1]);
    gamedata.portals[1]->addChild(*gamedata.lights[2]);

    // Load all textures
    gamedata.portalGunAlbedo = new Texture("../res/textures/portalgun_col.png", LINEAR);
    gamedata.wall = new Texture("../res/textures/wall.png", LINEAR);
    gamedata.rubix = new Texture("../res/textures/rubix.png", NEAREST);
    gamedata.noise = new Texture(255, 255, 4, 100, 34877u);
    gamedata.noise->bind(NOISE_TEXTURE_BINDING);

    // Assign all initial textures
    gamedata.portalGun->albedo = gamedata.portalGunAlbedo;
    gamedata.cube->albedo = gamedata.rubix;
    gamedata.chamber->albedo = gamedata.wall;
    gamedata.player->albedo = gamedata.rubix;

    gamedata.portalGun->setPosition(glm::vec3(0.007f, -0.005f, -0.01f));
    gamedata.portalGun->rotate(glm::vec3(0, 1.0f, 0), -0.3f);
    gamedata.portalGun->rotate(glm::vec3(0, 0, 1.0f), 0.1f);

    gamedata.portals[0]->translate(glm::vec3(-14.8, -10, 5));
    gamedata.portals[0]->rotate(glm::vec3(0, 1, 0), M_PI / 2);

    gamedata.portals[1]->translate(glm::vec3(14.8, -10, -5));
    gamedata.portals[1]->rotate(glm::vec3(0, 1, 0), -M_PI / 2);

    gamedata.cube->setPosition(glm::vec3(0,-12.5, 0));

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_MULTISAMPLE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    gamedata.window->disableCursor();
}

void update(gamedata_st &gamedata)
{
    gamedata.root->updateTransforms();
    double time = gamedata.window->getTime();
    gamedata.portalGun->setPosition(glm::vec3(0.007f, -0.005f + 0.0005f * sin(time / 10.0f), -0.01f));
    //gamedata.cube->rotate(glm::vec3(0, 0.01f, 0));

    gamedata.window->updateInput();

    gamedata.farCamera->direct(-gamedata.window->getMouseDelta().x / 500, -gamedata.window->getMouseDelta().y / 500);
    
    glm::vec3 camTranslation = gamedata.farCamera->getFirstPersonTranslation(glm::vec3(
        (gamedata.window->isKeyDown(GLFW_KEY_D) - gamedata.window->isKeyDown(GLFW_KEY_A)) / 5.0f,
        (gamedata.window->isKeyDown(GLFW_KEY_SPACE) - gamedata.window->isKeyDown(GLFW_KEY_LEFT_SHIFT)) / 5.0f,
        (gamedata.window->isKeyDown(GLFW_KEY_W) - gamedata.window->isKeyDown(GLFW_KEY_S)) / 5.0f)
    );

    // TODO: Only move the rest of the way out of the portal after going through
    gamedata.farCamera->translate(camTranslation);
    gamedata.portals[0]->passthrough(*gamedata.farCamera, camTranslation, *gamedata.portals[1]);
    gamedata.portals[1]->passthrough(*gamedata.farCamera, camTranslation, *gamedata.portals[0]);

    glm::vec3 intersectNormal1, intersectNormal2;
    glm::vec3 intersection1, intersection2;
    if(
        gamedata.window->isMouseButtonPressed(GLFW_MOUSE_BUTTON_1) && 
        gamedata.chamber->isColliding(gamedata.farCamera->getGlobalPosition(), 100.0f * gamedata.farCamera->get3DLookingVector(), intersectNormal1, intersection1) && 
        gamedata.chamber->isColliding(gamedata.farCamera->getGlobalPosition() + gamedata.farCamera->getUpVector() * gamedata.portals[0]->getDimensions().y * 0.5f, 100.0f * gamedata.farCamera->get3DLookingVector(), intersectNormal2, intersection2) &&
        intersectNormal1 == intersectNormal2
    )
    {
        gamedata.portals[0]->setOrientation(glm::fquat(1,0,0,0));
        gamedata.portals[0]->setPosition(intersection1 + intersectNormal1 * 0.2f);
        glm::vec3 wallUp = glm::normalize(intersection2 - intersection1);
        
        // Align the normal of the wall to the normal of the portal
        glm::vec3 portalNormal = gamedata.portals[0]->getNormal();
        float dot = glm::dot(portalNormal, intersectNormal1);
        float angle = acos(dot);
        glm::vec3 axis = glm::normalize(glm::cross(portalNormal, intersectNormal1));

        if(dot > 0.99)
        {
            angle = 0;
            axis = glm::vec3(1,0,0);
        }
        else if(dot < -0.99)
        {
            angle = M_PI;
            axis = wallUp;
        }
        
        gamedata.portals[0]->rotate(axis, angle);

        // Align the up direction of the portal, to the up vector defined by the ray-casts
        glm::vec3 portalUp = gamedata.portals[0]->getUp();
        dot = glm::dot(portalUp, wallUp);
        angle = acos(dot);
        axis = glm::cross(portalUp, wallUp) * gamedata.portals[0]->getOrientation();

        if(dot > 0.99)
        {
            angle = 0;
            axis = glm::vec3(1,0,0);
        }
        else if(dot < -0.99)
        {
            angle = M_PI;
            axis = intersectNormal1;
        }

        gamedata.portals[0]->rotate(axis, angle);
    }

    //gamedata.portals[0]->rotate(gamedata.portals[0]->getNormal(), 0.01f);

    if (gamedata.window->isKeyDown(GLFW_KEY_ESCAPE))
        gamedata.window->close();
}

void render(gamedata_st &gamedata)
{
    int uTimeLoc = gamedata.shader->getUniformLocation("u_time");
    glUniform1f(uTimeLoc, (float) gamedata.window->getTime());

    gamedata.lights[0]->updateUniform(*gamedata.shader);
    gamedata.lights[1]->updateUniform(*gamedata.shader);
    gamedata.lights[2]->updateUniform(*gamedata.shader);

    //int uViewLoc = gamedata.shader->getUniformLocation("view");
    //int uProjLoc = gamedata.shader->getUniformLocation("proj");

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glm::mat4 view = gamedata.farCamera->getViewMatrix();
    glm::mat4 proj = gamedata.farCamera->getPerspectiveMatrix();

    renderRecursivePortals(gamedata, view, proj, 10);

/*     glDisable(GL_DEPTH_TEST);
    glUniformMatrix4fv(uProjLoc, 1, GL_FALSE, glm::value_ptr(gamedata.nearCamera->getPerspectiveMatrix()));
    glUniformMatrix4fv(uViewLoc, 1, GL_FALSE, glm::value_ptr(glm::identity<glm::mat4>()));
    gamedata.portalGun->render();
    glEnable(GL_DEPTH_TEST);
 */
    gamedata.window->swapBuffers();
}

void recursivePortalHelper(gamedata_st gamedata, glm::mat4 proj, glm::mat4 p1View, glm::mat4 p1Proj, glm::mat4 p2View, glm::mat4 p2Proj, int maxDepth, int depth);

void renderRecursivePortals(gamedata_st &gamedata, glm::mat4 view, glm::mat4 proj, int maxDepth)
{
    glEnable(GL_STENCIL_TEST);
    glStencilMask(0xff);

    recursivePortalHelper(gamedata, proj, view, proj, view, proj, maxDepth, 0);

    glDisable(GL_STENCIL_TEST);
}

void recursivePortalHelper(gamedata_st gamedata, glm::mat4 proj, glm::mat4 p1View, glm::mat4 p1Proj, glm::mat4 p2View, glm::mat4 p2Proj, int maxDepth, int depth)
{
    int uViewLoc = gamedata.shader->getUniformLocation("view");
    int uProjLoc = gamedata.shader->getUniformLocation("proj");
    Portal *p1 = gamedata.portals[0];
    Portal *p2 = gamedata.portals[1];

    // Render the world inside portal 1 on depth 0, this is the current world
    glStencilFunc(GL_EQUAL, depth, 0xff);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    renderWorld(gamedata, p1View, p1Proj);

    if(depth > 0)
    {
        // Render the world inside portal 2
        glStencilFunc(GL_EQUAL, (uint8_t)-depth, 0xff);
        glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
        renderWorld(gamedata, p2View, p2Proj);
    }

    if (depth < maxDepth)
    {
        // We have to disable the color mask here when drawing the stencils.
        // This is due to the portal not rendering correcly because the oblique projection matrix
        // does not create a consitant depth value
        // This the rendering of the portals have to be done on the way "back out" using the normal projection matrix
        // This is also to get the correct transparency
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
        // Create stencil for portal 1
        glStencilFunc(GL_EQUAL, depth, 0xff);
        glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);

        glUniformMatrix4fv(uViewLoc, 1, GL_FALSE, glm::value_ptr(p1View));
        glUniformMatrix4fv(uProjLoc, 1, GL_FALSE, glm::value_ptr(p1Proj));

        p1->render();

        // Create stencil for portal 2
        glStencilFunc(GL_EQUAL, (uint8_t)-depth, 0xff);
        glStencilOp(GL_KEEP, GL_KEEP, GL_DECR_WRAP);

        glUniformMatrix4fv(uViewLoc, 1, GL_FALSE, glm::value_ptr(p2View));
        glUniformMatrix4fv(uProjLoc, 1, GL_FALSE, glm::value_ptr(p2Proj));

        p2->render();
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

        glm::mat4 nextP1View = p1->getViewMatrix(p1View, p2);
        glm::mat4 nextP2View = p2->getViewMatrix(p2View, p1);
        glm::mat4 nextP1Proj = p2->getObliqueProjection(proj, nextP1View);
        glm::mat4 nextP2Proj = p1->getObliqueProjection(proj, nextP2View);

        recursivePortalHelper(gamedata, proj, nextP1View, nextP1Proj, nextP2View, nextP2Proj, maxDepth, depth + 1);
    }
    else
    {
        glClear(GL_DEPTH_BUFFER_BIT);
    }

    // Draw the portals on the way back out
    glUniformMatrix4fv(uViewLoc, 1, GL_FALSE, glm::value_ptr(p1View));
    glUniformMatrix4fv(uProjLoc, 1, GL_FALSE, glm::value_ptr(proj));
    glStencilOp(GL_KEEP, GL_KEEP, GL_DECR);
    glStencilFunc(GL_EQUAL, depth + 1, 0xff);
    p1->render();

    glUniformMatrix4fv(uViewLoc, 1, GL_FALSE, glm::value_ptr(p2View));
    glUniformMatrix4fv(uProjLoc, 1, GL_FALSE, glm::value_ptr(proj));
    glStencilOp(GL_KEEP, GL_KEEP, GL_INCR_WRAP);
    glStencilFunc(GL_EQUAL, (uint8_t)(-depth - 1), 0xff);
    p2->render();
}

void renderWorld(gamedata_st &gamedata, glm::mat4 view, glm::mat4 proj)
{
    // Get the camera position and send it to the shader
    int uCameraLoc = gamedata.shader->getUniformLocation("u_camera_position");
    glm::vec3 camPosition = glm::vec3(glm::column(glm::inverse(view), 3));
    glUniform3fv(uCameraLoc, 1, glm::value_ptr(camPosition));

    int uViewLoc = gamedata.shader->getUniformLocation("view");
    int uProjLoc = gamedata.shader->getUniformLocation("proj");

    glUniformMatrix4fv(uViewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(uProjLoc, 1, GL_FALSE, glm::value_ptr(proj));

    gamedata.cube->render();
    gamedata.chamber->render();
    gamedata.player->render();
}

void destroy(gamedata_st &gamedata)
{
    // Destroy all meshes
    gamedata.portals[0]->destroy();
    gamedata.portals[1]->destroy();
    gamedata.chamber->destroy();
    gamedata.player->destroy();
    gamedata.cube->destroy();
    gamedata.portalGun->destroy();

    // Destory all textures
    gamedata.rubix->destroy();
    gamedata.portalGunAlbedo->destroy();
    gamedata.wall->destroy();

    gamedata.window->destroy();
    glfwTerminate();

    // Delete all pointers in the gamedata
    // TODO: This might be a bit too shady
    for(long long unsigned int i = 0; i < (sizeof(gamedata) / sizeof(void*)); i++)
    {
        void** start = (void**) &gamedata;
        delete *(start + i);
        *(start + i) = nullptr;
    }
}