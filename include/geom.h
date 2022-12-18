#ifndef GEOM_H
#define GEOM_H

struct vec3d
{
    float x, y, z;
};

struct triangle
{
    vec3d p[3];
};

struct mat4x4
{
    float m[4][4] = {0};
};

#endif // GEOM_H