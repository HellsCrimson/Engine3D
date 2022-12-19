#ifndef GEOM_H
#define GEOM_H

struct vec3d
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float w = 1.0f;
};

struct triangle
{
    vec3d p[3];
    wchar_t sym;
    short col;
};

struct mat4x4
{
    float m[4][4] = {0};
};

#endif // GEOM_H