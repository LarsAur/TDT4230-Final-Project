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
        // Print the fps and average frametime every 10 seconds
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
    gamedata.camera = new Camera(*gamedata.window, glm::vec3(0), M_PI / 2, 0.01f, 200.0f);

    // Create all meshes
    gamedata.turret = new ObjMesh("../res/models/turret.obj", 0.1f);
    gamedata.player = new Cube(glm::vec3(1.0f, 1.0f, 1.0f), false);
    gamedata.portals[0] = new Portal(glm::vec2(5, 10), glm::vec3(0.36f, 0.58f, 1.0f));
    gamedata.portals[1] = new Portal(glm::vec2(5, 10), glm::vec3(1.0f, 0.5f, 0.05f));
    gamedata.cubes.push_back(new Cube(glm::vec3(120, 60, 60), true));   // The room
    gamedata.cubes.push_back(new Cube(glm::vec3(20, 20, 60), false));   // The Long bar
    gamedata.cubes.push_back(new Cube(glm::vec3(40, 20, 20), false));   // The center cube
    gamedata.cubes.push_back(new Cube(glm::vec3(20, 60, 20), false));   // 45 degree corner
    gamedata.cubes.push_back(new Cube(glm::vec3(20, 60, 20), false));   // 45 degree corner

    // Generate vertexdata for all meshes
    gamedata.turret->generateVertexData(*gamedata.shader);
    gamedata.portals[0]->generateVertexData(*gamedata.shader);
    gamedata.portals[1]->generateVertexData(*gamedata.shader);
    gamedata.player->generateVertexData(*gamedata.shader);

    // Create a root node and connect the scene elements as a tree
    gamedata.root = new Node();
    gamedata.root->addChild(*gamedata.camera);
    gamedata.root->addChild(*gamedata.portals[0]);
    gamedata.root->addChild(*gamedata.portals[1]);
    gamedata.root->addChild(*gamedata.turret);
    gamedata.camera->addChild(*gamedata.player);
    
    gamedata.root->addChild(*gamedata.cubes[0]);
    for(size_t i = 1; i < gamedata.cubes.size(); i++)
    {
        gamedata.cubes[0]->addChild(*gamedata.cubes[i]);
    }

    // Create all lights
    gamedata.lights[0] = new Light(glm::vec3(0, 0, 0.5), glm::vec3(0.36, 0.58, 1.0));
    gamedata.lights[1] = new Light(glm::vec3(0, 0, 0.5), glm::vec3(1.0, 0.5, 0.05));
    gamedata.lights[2] = new Light(glm::vec3(-30,5,0), glm::vec3(0.5, 0.5, 0.5));
    gamedata.lights[3] = new Light(glm::vec3(0,5,0), glm::vec3(0.5, 0.5, 0.5));
    gamedata.lights[4] = new Light(glm::vec3(30,5,0), glm::vec3(0.5, 0.5, 0.5));
    
    // Add lights to the portals
    gamedata.portals[0]->addChild(*gamedata.lights[0]);
    gamedata.portals[1]->addChild(*gamedata.lights[1]);

    // Load all textures and genereate a perlin noise texture
    gamedata.wallTexture = new Texture("../res/textures/wall.png", LINEAR);
    gamedata.rubixTexture = new Texture("../res/textures/rubix.png", NEAREST);
    gamedata.turretTexture = new Texture("../res/textures/turret.bmp", LINEAR);
    gamedata.noiseTexture = new Texture(255, 255, 4, 50, 34877u);
    gamedata.noiseTexture->bind(NOISE_TEXTURE_BINDING);

    for(Cube *cube : gamedata.cubes)
    {
        cube->generateVertexData(*gamedata.shader);
        cube->albedo = gamedata.wallTexture;
    }

    // Assign all initial textures
    gamedata.player->albedo = gamedata.rubixTexture;
    gamedata.turret->albedo = gamedata.turretTexture;
    
    // Initial position of the portals
    gamedata.portals[0]->setPosition(glm::vec3(-8, -24, -9.8f));
    gamedata.portals[1]->setPosition(glm::vec3(8, -24, -9.8f));

    // Place the cubes to create the scene
    gamedata.cubes[1]->setPosition(glm::vec3(50, -20, 0));
    gamedata.cubes[2]->setPosition(glm::vec3(0, -20, -20));
    gamedata.cubes[3]->setPosition(glm::vec3(-60, 0, 30));
    gamedata.cubes[4]->setPosition(glm::vec3(-60, 0, -30));
    gamedata.cubes[3]->rotate(glm::vec3(0, 1, 0), M_PI / 4);
    gamedata.cubes[4]->rotate(glm::vec3(0, 1, 0), M_PI / 4);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glEnable(GL_BLEND);

    gamedata.window->disableCursor();
}

void update(gamedata_st &gamedata)
{
    // Update the global position of all the nodes in the scene
    gamedata.root->updateTransforms();

    // Rotate and bob the turret up and down
    double time = gamedata.window->getTime();
    gamedata.turret->rotate(glm::vec3(0, 1, 0), 0.01f);
    gamedata.turret->setPosition(glm::vec3(0, -25 + sin(time), 0));

    // Update the inputs of the window. E.g check if buttons were pressed this frame
    gamedata.window->updateInput();

    // Direct the camera using the mouse
    gamedata.camera->direct(
        -gamedata.window->getMouseDelta().x / 500,
        -gamedata.window->getMouseDelta().y / 500
    );
    
    // Get the camera translation in world space based on keyboard input
    glm::vec3 camTranslation = gamedata.camera->getCameraTranslation(glm::vec3(
        (gamedata.window->isKeyDown(GLFW_KEY_D) - gamedata.window->isKeyDown(GLFW_KEY_A)) / 5.0f,
        (gamedata.window->isKeyDown(GLFW_KEY_SPACE) - gamedata.window->isKeyDown(GLFW_KEY_LEFT_SHIFT)) / 5.0f,
        (gamedata.window->isKeyDown(GLFW_KEY_W) - gamedata.window->isKeyDown(GLFW_KEY_S)) / 5.0f    
    ));

    // Move the camera using the translation
    gamedata.camera->translate(camTranslation);

    // Check if the camera would pass through the portal and move it if it does
    // If the camera passes through the portal, it cannot pass through the other portal
    if(!gamedata.portals[0]->passthrough(*gamedata.camera, camTranslation, *gamedata.portals[1]))
    {
        gamedata.portals[1]->passthrough(*gamedata.camera, camTranslation, *gamedata.portals[0]);
    }

    // If the mouse buttons are pressed, attempt to place a portal
    if(
        gamedata.window->isMouseButtonPressed(GLFW_MOUSE_BUTTON_1) || 
        gamedata.window->isMouseButtonPressed(GLFW_MOUSE_BUTTON_2)
    )
    {
        placePortals(gamedata);
    }

    // Exit the game by pressing escape
    if (gamedata.window->isKeyDown(GLFW_KEY_ESCAPE))
        gamedata.window->close();
}

// Attempt to place a portal by casting two rays intersecting with the cubes in the scene
void placePortals(gamedata_st &gamedata)
{
    #define MAX_DIST 1000.0f // Max distance for the rays
    float dist = MAX_DIST;
    glm::vec3 pos, normal, up;  // Positions, normal and up vector of the wall that is hit
    for(Cube *cube : gamedata.cubes)
    {
        // Cast two rays, one from the camara and one from some distance upward in viewspace
        // This is to find the upward vector along surfaces which have a non-zero z-value in the normalvector
        glm::vec3 intersectNormal1, intersectNormal2;
        glm::vec3 intersection1, intersection2;
        if(
            cube->isColliding(gamedata.camera->getGlobalPosition(), MAX_DIST * gamedata.camera->get3DLookingVector(), intersectNormal1, intersection1) && 
            cube->isColliding(gamedata.camera->getGlobalPosition() + gamedata.camera->getUpVector(), MAX_DIST * gamedata.camera->get3DLookingVector(), intersectNormal2, intersection2) &&
            intersectNormal1 == intersectNormal2 // Check if the normals are equal, to not place portals in corners
        )
        {
            // Find the closest intersection point
            float tmpDist = glm::distance(gamedata.camera->getGlobalPosition(), intersection1);
            if(dist > tmpDist)
            {
                dist = tmpDist;
                normal = intersectNormal1;
                pos = intersection1;
                up = intersection2 - intersection1;
            }
        }
    }

    // Place the portal corresponding to the mouse button pressed
    if(dist != MAX_DIST)
    {
        Portal *portal = gamedata.window->isMouseButtonDown(GLFW_MOUSE_BUTTON_1) ? gamedata.portals[0] : gamedata.portals[1];
        portal->place(pos + normal * 0.2f, normal, up);
    }
}

void render(gamedata_st &gamedata)
{
    // Send the time to the fragmentshader
    int uTimeLoc = gamedata.shader->getUniformLocation("u_time");
    glUniform1f(uTimeLoc, (float) gamedata.window->getTime());

    // Update all lights in the fragment shader
    for(int i = 0; i < N_LIGHTS; i++)
    {
        gamedata.lights[i]->updateUniform(*gamedata.shader);
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // Render the recursive portals
    glm::mat4 view = gamedata.camera->getViewMatrix();
    glm::mat4 proj = gamedata.camera->getPerspectiveMatrix();
    renderRecursivePortals(gamedata, view, proj, 10);

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

    // Render the world inside portal 1. 
    // On depth 0, this is the world outside the portals
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
        // Create the stencil for portal 1 by incrementing the stencil buffer
        glStencilFunc(GL_EQUAL, depth, 0xff);
        glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);

        glUniformMatrix4fv(uViewLoc, 1, GL_FALSE, glm::value_ptr(p1View));
        glUniformMatrix4fv(uProjLoc, 1, GL_FALSE, glm::value_ptr(p1Proj));

        p1->render();

        // Create the stencil for portal 2 by decrementing the stencil buffer
        glStencilFunc(GL_EQUAL, (uint8_t)-depth, 0xff);
        glStencilOp(GL_KEEP, GL_KEEP, GL_DECR_WRAP);

        glUniformMatrix4fv(uViewLoc, 1, GL_FALSE, glm::value_ptr(p2View));
        glUniformMatrix4fv(uProjLoc, 1, GL_FALSE, glm::value_ptr(p2Proj));

        p2->render();

        // Create the view from the destination portal given the view used then looking into it
        glm::mat4 nextP1View = p1->getViewMatrix(p1View, p2);
        glm::mat4 nextP2View = p2->getViewMatrix(p2View, p1);
        // Create the oblique projections using the standard projection, assumes that the input has a standard view-frustum
        glm::mat4 nextP1Proj = p2->getObliqueProjection(proj, nextP1View);
        glm::mat4 nextP2Proj = p1->getObliqueProjection(proj, nextP2View);

        // Clearing the depth buffer is necessary because the objects inside the portal can have
        // both a lower and higher depth value, because the near plane is moved
        // By clearing the depth buffer, it is ensured that everything inside the portals are rendered
        // The stencil buffer ensures that the fragments outside of the portal are not overwritten.
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
    // Send the camera position, view, and projection
    // This is required to do every render, because the camera position would be different
    // depending on the state of the recursion.
    int uCameraLoc = gamedata.shader->getUniformLocation("u_camera_position");
    int uViewLoc = gamedata.shader->getUniformLocation("view");
    int uProjLoc = gamedata.shader->getUniformLocation("proj");
    
    glm::vec3 camPosition = glm::vec3(glm::column(glm::inverse(view), 3));
    glUniform3fv(uCameraLoc, 1, glm::value_ptr(camPosition));
    glUniformMatrix4fv(uViewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(uProjLoc, 1, GL_FALSE, glm::value_ptr(proj));

    // Render all scene elements
    for(Cube *cube : gamedata.cubes)
    {
        cube->render();
    }
    gamedata.player->render();
    gamedata.turret->render();
}

void destroy(gamedata_st &gamedata)
{
    // Destroy all meshes
    gamedata.portals[0]->destroy();
    gamedata.portals[1]->destroy();
    gamedata.player->destroy();
    gamedata.turret->destroy();
    for(Cube *cube : gamedata.cubes)
    {
        cube->destroy();
    }

    // Destory all textures
    gamedata.turretTexture->destroy();
    gamedata.noiseTexture->destroy();
    gamedata.rubixTexture->destroy();
    gamedata.wallTexture->destroy();

    gamedata.window->destroy();
    glfwTerminate();

    delete gamedata.window;
    delete gamedata.root;
    delete gamedata.turret;
    delete gamedata.player;
    delete gamedata.portals[0];
    delete gamedata.portals[1];
    delete gamedata.wallTexture;
    delete gamedata.rubixTexture;
    delete gamedata.turretTexture;
    delete gamedata.noiseTexture;
    delete gamedata.camera;
    delete gamedata.shader;

    for(int i = 0; i < N_LIGHTS; i++)
    {
        delete gamedata.lights[i];
    }

    for(Cube *cube : gamedata.cubes)
    {
        delete cube;
    }
}