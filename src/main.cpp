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
void placePortals(gamedata_st &gamedata);
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

    // Create cameras
    gamedata.farCamera = new Camera(*gamedata.window, glm::vec3(0), M_PI / 2, 0.01f, 200.0f);
    gamedata.nearCamera = new Camera(*gamedata.window, glm::vec3(0), M_PI / 2, 0.1f, 1.0f);

    // Create all meshes
    gamedata.portalGun = new ObjMesh("../res/models/PortalGunNormals.obj", 0.01f);
    gamedata.player = new Cube(glm::vec3(1.0f, 1.0f, 1.0f), false);
    gamedata.portals[0] = new Portal(glm::vec2(5, 10), glm::vec3(0.36f, 0.58f, 1.0f));
    gamedata.portals[1] = new Portal(glm::vec2(5, 10), glm::vec3(1.0f, 0.5f, 0.05f));
    gamedata.cubes.push_back(new Cube(glm::vec3(120, 60, 60), true));   // The room
    gamedata.cubes.push_back(new Cube(glm::vec3(20, 20, 60), false));   // The Long bar
    gamedata.cubes.push_back(new Cube(glm::vec3(40, 20, 20), false));   // The center cube
    gamedata.cubes.push_back(new Cube(glm::vec3(20, 60, 20), false));   // 45 degree corner
    gamedata.cubes.push_back(new Cube(glm::vec3(20, 60, 20), false));   // 45 degree corner

    // Generate vertexdata for all meshes
    gamedata.portalGun->generateVertexData(*gamedata.shader);
    gamedata.portals[0]->generateVertexData(*gamedata.shader);
    gamedata.portals[1]->generateVertexData(*gamedata.shader);
    gamedata.player->generateVertexData(*gamedata.shader);

    gamedata.root = new Node();
    gamedata.root->addChild(*gamedata.farCamera);
    gamedata.root->addChild(*gamedata.nearCamera);
    gamedata.root->addChild(*gamedata.portalGun);
    gamedata.root->addChild(*gamedata.portals[0]);
    gamedata.root->addChild(*gamedata.portals[1]);
    gamedata.farCamera->addChild(*gamedata.player);
    
    gamedata.root->addChild(*gamedata.cubes[0]);
    for(size_t i = 1; i < gamedata.cubes.size(); i++)
    {
        gamedata.cubes[0]->addChild(*gamedata.cubes[i]);
    }

    // Add lights
    gamedata.lights[0] = new Light(glm::vec3(0, 0, 0.5), glm::vec3(0.36, 0.58, 1.0));
    gamedata.lights[1] = new Light(glm::vec3(0, 0, 0.5), glm::vec3(1.0, 0.5, 0.05));
    gamedata.lights[2] = new Light(glm::vec3(-30,5,0), glm::vec3(0.5, 0.5, 0.5));
    gamedata.lights[3] = new Light(glm::vec3(0,5,0), glm::vec3(0.5, 0.5, 0.5));
    gamedata.lights[4] = new Light(glm::vec3(30,5,0), glm::vec3(0.5, 0.5, 0.5));
    gamedata.portals[0]->addChild(*gamedata.lights[0]);
    gamedata.portals[1]->addChild(*gamedata.lights[1]);

    // Load all textures
    gamedata.portalGunAlbedo = new Texture("../res/textures/portalgun_col.png", LINEAR);
    gamedata.wall = new Texture("../res/textures/wall.png", LINEAR);
    gamedata.rubix = new Texture("../res/textures/rubix.png", NEAREST);
    gamedata.noise = new Texture(255, 255, 4, 100, 34877u);
    gamedata.noise->bind(NOISE_TEXTURE_BINDING);

    for(Cube *cube : gamedata.cubes)
    {
        cube->generateVertexData(*gamedata.shader);
        cube->albedo = gamedata.wall;
    }

    // Assign all initial textures
    gamedata.portalGun->albedo = gamedata.portalGunAlbedo;
    gamedata.player->albedo = gamedata.rubix;
    
    gamedata.portalGun->setPosition(glm::vec3(0.007f, -0.005f, -0.01f));
    gamedata.portalGun->rotate(glm::vec3(0, 1.0f, 0), -0.3f);
    gamedata.portalGun->rotate(glm::vec3(0, 0, 1.0f), 0.1f);

    // Place the portal such that they are out of the scene
    gamedata.portals[0]->setPosition(glm::vec3(0, 100, 0));
    gamedata.portals[1]->translate(glm::vec3(0, 100, 0));

    gamedata.cubes[1]->setPosition(glm::vec3(50, -20, 0));
    gamedata.cubes[2]->setPosition(glm::vec3(0, -20, -20));
    gamedata.cubes[3]->setPosition(glm::vec3(-60, 0, 30));
    gamedata.cubes[4]->setPosition(glm::vec3(-60, 0, -30));

    gamedata.cubes[3]->rotate(glm::vec3(0, 1, 0), M_PI / 4);
    gamedata.cubes[4]->rotate(glm::vec3(0, 1, 0), M_PI / 4);

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
    // If the object passes through the portal, it cannot pass through the other portal
    if(!gamedata.portals[0]->passthrough(*gamedata.farCamera, camTranslation, *gamedata.portals[1]))
    {
        gamedata.portals[1]->passthrough(*gamedata.farCamera, camTranslation, *gamedata.portals[0]);
    }

    if(
        gamedata.window->isMouseButtonPressed(GLFW_MOUSE_BUTTON_1) || 
        gamedata.window->isMouseButtonPressed(GLFW_MOUSE_BUTTON_2)
    )
    {
        placePortals(gamedata);
    }

    if (gamedata.window->isKeyDown(GLFW_KEY_ESCAPE))
        gamedata.window->close();
}

void placePortals(gamedata_st &gamedata)
{
    #define MAX_DIST 100000
    float dist = MAX_DIST;
    glm::vec3 pos, normal, up;
    for(Cube *cube : gamedata.cubes)
    {
        glm::vec3 intersectNormal1, intersectNormal2;
        glm::vec3 intersection1, intersection2;
        if(
            cube->isColliding(gamedata.farCamera->getGlobalPosition(), 150.0f * gamedata.farCamera->get3DLookingVector(), intersectNormal1, intersection1) && 
            cube->isColliding(gamedata.farCamera->getGlobalPosition() + gamedata.farCamera->getUpVector(), 150.0f * gamedata.farCamera->get3DLookingVector(), intersectNormal2, intersection2) &&
            intersectNormal1 == intersectNormal2
        )
        {
            float tmpDist = glm::distance(gamedata.farCamera->getGlobalPosition(), intersection1);
            if(dist > tmpDist)
            {
                dist = tmpDist;
                normal = intersectNormal1;
                pos = intersection1;
                up = intersection2 - intersection1;
            }
        }
    }

    if(dist != MAX_DIST)
    {
        Portal *portal = gamedata.window->isMouseButtonDown(GLFW_MOUSE_BUTTON_1) ? gamedata.portals[0] : gamedata.portals[1];
        portal->place(pos + normal * 0.2f, normal, up);
    }
}

void render(gamedata_st &gamedata)
{
    int uTimeLoc = gamedata.shader->getUniformLocation("u_time");
    glUniform1f(uTimeLoc, (float) gamedata.window->getTime());

    for(int i = 0; i < N_LIGHTS; i++)
    {
        gamedata.lights[i]->updateUniform(*gamedata.shader);

    }

    int uViewLoc = gamedata.shader->getUniformLocation("view");
    int uProjLoc = gamedata.shader->getUniformLocation("proj");

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glm::mat4 view = gamedata.farCamera->getViewMatrix();
    glm::mat4 proj = gamedata.farCamera->getPerspectiveMatrix();

    renderRecursivePortals(gamedata, view, proj, 10);

    glDisable(GL_DEPTH_TEST);
    glUniformMatrix4fv(uProjLoc, 1, GL_FALSE, glm::value_ptr(gamedata.nearCamera->getPerspectiveMatrix()));
    glUniformMatrix4fv(uViewLoc, 1, GL_FALSE, glm::value_ptr(glm::identity<glm::mat4>()));
    gamedata.portalGun->render();
    glEnable(GL_DEPTH_TEST);

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

        glClear(GL_DEPTH_BUFFER_BIT);
        recursivePortalHelper(gamedata, proj, nextP1View, nextP1Proj, nextP2View, nextP2Proj, maxDepth, depth + 1);
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

    for(Cube *cube : gamedata.cubes)
    {
        cube->render();
    }
    gamedata.player->render();
}

void destroy(gamedata_st &gamedata)
{
    // Destroy all meshes
    gamedata.portals[0]->destroy();
    gamedata.portals[1]->destroy();
    gamedata.player->destroy();
    gamedata.portalGun->destroy();
    for(Cube *cube : gamedata.cubes)
    {
        cube->destroy();
    }

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