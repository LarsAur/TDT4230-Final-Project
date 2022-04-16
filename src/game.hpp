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

    Mesh *player;
    Mesh *portalGun;
    Portal *portals[2];

    std::vector<Cube*> cubes;

    Texture *portalGunAlbedo;
    Texture *wall;
    Texture *rubix;
    Texture *noise;

    Camera *nearCamera;
    Camera *farCamera;

    Shader *shader;

    Light *lights[N_LIGHTS];
} gamedata_st;