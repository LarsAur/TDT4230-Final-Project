#pragma once

#include <shader.hpp>
#include <window.hpp>
#include <camera.hpp>
#include <mesh.hpp>
#include <node.hpp>
#include <texture.hpp>
#include <portal.hpp>
#include <light.hpp>

#define N_LIGHTS 5

#define ALBEDO_TEXTURE_BINDING 0
#define NOISE_TEXTURE_BINDING 1

typedef struct gamedata_st 
{
    Window *window;

    Node *root;
    ObjMesh *turret;
    Mesh *player;
    Portal *portals[2];

    std::vector<Cube*> cubes;

    Texture *wallTexture;
    Texture *rubixTexture;
    Texture *turretTexture;
    Texture *noiseTexture;

    Camera *camera;

    Shader *shader;

    Light *lights[N_LIGHTS];
} gamedata_st;