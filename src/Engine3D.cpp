#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include "geom.h"

struct mesh
{
    std::vector<triangle> tris;
};

class Engine3D : public olc::PixelGameEngine
{
public:
    Engine3D()
    {
        sAppName = "3D Engine";
    }

private:
    mesh meshCube;
    mat4x4 matProj;

    float fTheta;

    void MultiplyMatrixVector(vec3d &i, vec3d &o, mat4x4 &m)
    {
        o.x = i.x * m.m[0][0] + i.y * m.m[1][0] + i.z * m.m[2][0] + m.m[3][0];
        o.y = i.x * m.m[0][1] + i.y * m.m[1][1] + i.z * m.m[2][1] + m.m[3][1];
        o.z = i.x * m.m[0][2] + i.y * m.m[1][2] + i.z * m.m[2][2] + m.m[3][2];
        float w = i.x * m.m[0][3] + i.y * m.m[1][3] + i.z * m.m[2][3] + m.m[3][3];

        if (w != 0.0f)
        {
            o.x /= w;
            o.y /= w;
            o.z /= w;
        }
    }

public:
    bool OnUserCreate() override
    {
        meshCube.tris = {

            // SOUTH
            {0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f},
            {0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f},

            // EAST
            {1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f},
            {1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f},

            // NORTH
            {1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f},
            {1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f},

            // WEST
            {0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f},
            {0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f},

            // TOP
            {0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f},
            {0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f},

            // BOTTOM
            {1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f},
            {1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f},

        };

        float fNear = 0.1f;
        float fFar = 1000.0f;
        float fFov = 90.0f;
        float fAspectRatio = (float)ScreenHeight() / (float)ScreenWidth();
        float fFovRad = 1.0f / tanf(fFov * 0.5f / 180.0f * 3.14159f);

        matProj.m[0][0] = fAspectRatio * fFovRad;
        matProj.m[1][1] = fFovRad;
        matProj.m[2][2] = fFar / (fFar - fNear);
        matProj.m[3][2] = (-fFar * fNear) / (fFar - fNear);
        matProj.m[2][3] = 1.0f;
        matProj.m[3][3] = 0.0f;

        return true;
    }

    bool OnUserUpdate(float fElapsedTime) override
    {
        Clear(olc::BLACK);

        mat4x4 matRotZ;
        mat4x4 matRotX;
        fTheta += 1.0f * fElapsedTime;

        matRotZ.m[0][0] = cosf(fTheta);
        matRotZ.m[0][1] = sinf(fTheta);
        matRotZ.m[1][0] = -sinf(fTheta);
        matRotZ.m[1][1] = cosf(fTheta);
        matRotZ.m[2][2] = 1;
        matRotZ.m[3][3] = 1;

        matRotX.m[0][0] = 1;
        matRotX.m[1][1] = cosf(fTheta * 0.5f);
        matRotX.m[1][2] = sinf(fTheta * 0.5f);
        matRotX.m[2][1] = -sinf(fTheta * 0.5f);
        matRotX.m[2][2] = cosf(fTheta * 0.5f);
        matRotX.m[3][3] = 1;

        for (auto tri : meshCube.tris)
        {
            triangle triProjected;
            triangle triTranslated;
            triangle triRotatedZ;
            triangle triRotatedZX;

            for (int i = 0; i < 3; i++)
            {
                MultiplyMatrixVector(tri.p[i], triRotatedZ.p[i], matRotZ);
            }

            for (int i = 0; i < 3; i++)
            {
                MultiplyMatrixVector(triRotatedZ.p[i], triRotatedZX.p[i], matRotX);
            }

            triTranslated = triRotatedZX;

            for (int i = 0; i < 3; i++)
            {
                triTranslated.p[i].z = triRotatedZX.p[i].z + 3.0f;
            }

            for (int i = 0; i < 3; i++)
            {
                MultiplyMatrixVector(triTranslated.p[i], triProjected.p[i], matProj);
            }

            for (int i = 0; i < 3; i++)
            {
                triProjected.p[i].x += 1.0f;
                triProjected.p[i].y += 1.0f;
            }

            for (int i = 0; i < 3; i++)
            {
                triProjected.p[i].x *= 0.5f * (float)ScreenWidth();
                triProjected.p[i].y *= 0.5f * (float)ScreenHeight();
            }

            DrawTriangle(triProjected.p[0].x, triProjected.p[0].y,
                         triProjected.p[1].x, triProjected.p[1].y,
                         triProjected.p[2].x, triProjected.p[2].y,
                         olc::WHITE);
        }

        return true;
    }
};

int main(void)
{
    Engine3D engine;
    if (engine.Construct(256, 240, 4, 4))
        engine.Start();
    else
        printf("Failed to start engine!\n");
    return 0;
}