#pragma once

#include <shader.hpp>
#include <window.hpp>
#include <camera.hpp>
#include <mesh.hpp>
#include <node.hpp>
#include <texture.hpp>
#include <portal.hpp>

typedef struct gamedata_st 
{
    Window *window;

    Node *root;

    Mesh *cube;
    Mesh *chamber;
    Mesh *portalGun;
    Portal *portals[2];

    Texture *portalGunAlbedo;
    Texture *wall;
    Texture *bluePortalTexture;
    Texture *orangeportalTexture;
    Texture *rubix;
    Texture *portalStencil;

    Camera *nearCamera;
    Camera *farCamera;

    Shader *shader;

    int timestep;
} gamedata_st;