#define OLC_PGE_APPLICATION

#include <fstream>
#include <sstream>
#include <algorithm>

#include "olcConsoleGameEngineSDL.h"
#include "geom.h"

bool wireframe = false;

struct mesh
{
    std::vector<triangle> tris;

    bool LoadFromObjectFile(std::string sFilename)
    {
        std::ifstream f(sFilename);
        if (!f.is_open())
            return false;

        std::vector<vec3d> verts;

        while (!f.eof())
        {
            char line[128];
            f.getline(line, 128);

            std::stringstream s;
            s << line;

            char junk;

            if (line[0] == 'v')
            {
                vec3d v;
                s >> junk >> v.x >> v.y >> v.z;
                verts.push_back(v);
            }

            if (line[0] == 'f')
            {
                int f[3];
                s >> junk >> f[0] >> f[1] >> f[2];
                tris.push_back({verts[f[0] - 1], verts[f[1] - 1], verts[f[2] - 1]});
            }
        }

        return true;
    }
};

class Engine3D : public olcConsoleGameEngine
{
public:
    Engine3D()
    {
        m_sAppName = L"3D Engine";
    }

private:
    mesh meshCube;
    mat4x4 matProj;

    vec3d vCamera;

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

    CHAR_INFO GetColour(float lum)
    {
        short bg_col, fg_col;
        wchar_t sym;
        int pixel_bw = (int)(13.0f * lum);
        switch (pixel_bw)
        {
        case 0:
            bg_col = BG_BLACK;
            fg_col = FG_BLACK;
            break;
        case 1:
            bg_col = BG_BLACK;
            fg_col = FG_DARK_GREY;
            break;
        case 2:
            bg_col = BG_BLACK;
            fg_col = FG_DARK_GREY;
            break;
        case 3:
            bg_col = BG_BLACK;
            fg_col = FG_DARK_GREY;
            break;
        case 4:
            bg_col = BG_BLACK;
            fg_col = FG_DARK_GREY;
            break;
        case 5:
            bg_col = BG_DARK_GREY;
            fg_col = FG_GREY;
            break;
        case 6:
            bg_col = BG_DARK_GREY;
            fg_col = FG_GREY;
            break;
        case 7:
            bg_col = BG_DARK_GREY;
            fg_col = FG_GREY;
            break;
        case 8:
            bg_col = BG_DARK_GREY;
            fg_col = FG_GREY;
            break;
        case 9:
            bg_col = BG_GREY;
            fg_col = FG_WHITE;
            break;
        case 10:
            bg_col = BG_GREY;
            fg_col = FG_WHITE;
            break;
        case 11:
            bg_col = BG_GREY;
            fg_col = FG_WHITE;
            break;
        case 12:
            bg_col = BG_GREY;
            fg_col = FG_WHITE;
            break;
        default:
            bg_col = BG_BLACK;
            fg_col = FG_BLACK;
        }

        CHAR_INFO c;
        c.colour = bg_col | fg_col;
        c.glyph = PIXEL_SOLID;
        return c;
    }

public:
    bool OnUserCreate() override
    {
        meshCube.LoadFromObjectFile("objects/SpaceShip.obj");

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
        Fill(0, 0, ScreenWidth(), ScreenHeight(), PIXEL_SOLID, FG_BLACK);

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

        std::vector<triangle> vecTrianglesToRaster;

        for (auto tri : meshCube.tris)
        {
            triangle triProjected;
            triangle triTranslated;
            triangle triRotatedZ;
            triangle triRotatedZX;

            // Rotate in Z-Axis
            for (int i = 0; i < 3; i++)
            {
                MultiplyMatrixVector(tri.p[i], triRotatedZ.p[i], matRotZ);
            }

            // Rotate in X-Axis
            for (int i = 0; i < 3; i++)
            {
                MultiplyMatrixVector(triRotatedZ.p[i], triRotatedZX.p[i], matRotX);
            }

            // Offset into the screen
            triTranslated = triRotatedZX;
            for (int i = 0; i < 3; i++)
            {
                triTranslated.p[i].z = triRotatedZX.p[i].z + 8.0f;
            }

            vec3d normal;
            vec3d line1;
            vec3d line2;
            line1.x = triTranslated.p[1].x - triTranslated.p[0].x;
            line1.y = triTranslated.p[1].y - triTranslated.p[0].y;
            line1.z = triTranslated.p[1].z - triTranslated.p[0].z;

            line2.x = triTranslated.p[2].x - triTranslated.p[0].x;
            line2.y = triTranslated.p[2].y - triTranslated.p[0].y;
            line2.z = triTranslated.p[2].z - triTranslated.p[0].z;

            normal.x = line1.y * line2.z - line1.z * line2.y;
            normal.y = line1.z * line2.x - line1.x * line2.z;
            normal.z = line1.x * line2.y - line1.y * line2.x;

            float l = sqrtf(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
            normal.x /= l;
            normal.y /= l;
            normal.z /= l;

            float dotProduct = normal.x * (triTranslated.p[0].x - vCamera.x) +
                               normal.y * (triTranslated.p[0].y - vCamera.y) +
                               normal.z * (triTranslated.p[0].z - vCamera.z);

            if (dotProduct < 0)
            {
                // Illumination
                vec3d light_direction = {0.0f, 0.0f, -1.0f};
                float l = sqrtf(light_direction.x * light_direction.x + light_direction.y * light_direction.y + light_direction.z * light_direction.z);
                light_direction.x /= l;
                light_direction.y /= l;
                light_direction.z /= l;

                // How similar is normal to light direction (dot product)
                float dp = normal.x * light_direction.x + normal.y * light_direction.y + normal.z * light_direction.z;

                CHAR_INFO c = GetColour(dp);
                triTranslated.col = c.colour;
                triTranslated.sym = c.glyph;

                // Project triangles from 3D --> 2D
                for (int i = 0; i < 3; i++)
                {
                    MultiplyMatrixVector(triTranslated.p[i], triProjected.p[i], matProj);
                }
                triProjected.col = triTranslated.col;
                triProjected.sym = triTranslated.sym;

                // Scale into view
                for (int i = 0; i < 3; i++)
                {
                    triProjected.p[i].x += 1.0f;
                    triProjected.p[i].y += 1.0f;
                }

                // Still scale into view
                for (int i = 0; i < 3; i++)
                {
                    triProjected.p[i].x *= 0.5f * (float)ScreenWidth();
                    triProjected.p[i].y *= 0.5f * (float)ScreenHeight();
                }

                // Store triangle for sorting
                vecTrianglesToRaster.push_back(triProjected);
            }
        }

        // Sort triangles from back to front
        sort(vecTrianglesToRaster.begin(), vecTrianglesToRaster.end(), [](triangle &t1, triangle &t2)
             {
                 float z1 = (t1.p[0].z + t1.p[1].z + t1.p[2].z) / 3.0f;
                 float z2 = (t2.p[0].z + t2.p[1].z + t2.p[2].z) / 3.0f;
                 return z1 > z2; });

        for (auto &triProjected : vecTrianglesToRaster)
        {
            // Rasterize triangle
            FillTriangle(triProjected.p[0].x, triProjected.p[0].y,
                         triProjected.p[1].x, triProjected.p[1].y,
                         triProjected.p[2].x, triProjected.p[2].y,
                            triProjected.sym, triProjected.col);
            
            if (wireframe)
            {
                DrawTriangle(triProjected.p[0].x, triProjected.p[0].y,
                         triProjected.p[1].x, triProjected.p[1].y,
                         triProjected.p[2].x, triProjected.p[2].y,
                            PIXEL_SOLID, FG_BLACK);
            }
        }

        return true;
    }
};

int main(int argc, char **argv)
{
    if (argc > 1)
    {
        if (strcmp(argv[1], "-w") == 0 || strcmp(argv[1], "-wireframe") == 0)
        {
            wireframe = true;
        }
        else
        {
            printf("Usage: %s [-w | -wireframe]\n", argv[0]);
        }
    }
    
    Engine3D engine;
    if (engine.ConstructConsole(256, 240, 4, 4))
        engine.Start();
    else
        printf("Failed to start engine!\n");
    return 0;
}