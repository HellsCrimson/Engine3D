#define OLC_PGE_APPLICATION

#include <fstream>
#include <sstream>
#include <algorithm>

#include "olcConsoleGameEngineSDL.h"
#include "Vector.h"
#include "Matrix.h"
#include "Geometry.h"
#include "Utils.h"

#define DIMENSIONS 3

float timerWireframe = 0.0f;
float timerClipping = 0.0f;

bool wireframe = false;
bool showClipping = false;
std::vector<std::string> objectFiles;

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
	std::vector<mesh> meshes;
	mat4x4 matProj;

	vec3d vCamera;
	vec3d vLookDir;

	float fYaw;

	float fTheta;

public:
	bool OnUserCreate() override
	{
		for (auto file : objectFiles)
		{
			mesh mesh;
			mesh.LoadFromObjectFile(file);
			meshes.push_back(mesh);
		}

		matProj = Matrix_MakeProjection(90.0f, (float)ScreenHeight() / (float)ScreenWidth(), 0.1f, 1000.0f);

		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		if (timerWireframe > 0.0f)
			timerWireframe -= fElapsedTime;
		if (timerClipping > 0.0f)
			timerClipping -= fElapsedTime;
		
		// Handle User Input
		if (GetKey(VK_SPACE).bHeld)
			vCamera.y += 8.0f * fElapsedTime;

		if (GetKey(VK_LCONTROL).bHeld)
			vCamera.y -= 8.0f * fElapsedTime;

		// Up vector
		vec3d vUp = {0, 1, 0};
		vec3d vRight = Vector_CrossProduct(vLookDir, vUp);
		vRight = Vector_Normalise(vRight);
		if (GetKey(L'A').bHeld)
		{
			vec3d translated = Vector_Mul(vRight, 8.0f * fElapsedTime);
			vCamera = Vector_Sub(vCamera, translated);
		}
		if (GetKey(L'E').bHeld)
		{
			vec3d translated = Vector_Mul(vRight, 8.0f * fElapsedTime);
			vCamera = Vector_Add(vCamera, translated);
		}

		vec3d vForward;
		if (GetKey(VK_LSHIFT).bHeld)
			vForward = Vector_Mul(vLookDir, 16.0f * fElapsedTime);
		else
			vForward = Vector_Mul(vLookDir, 8.0f * fElapsedTime);

		if (GetKey(L'Z').bHeld)
			vCamera = Vector_Add(vCamera, vForward);

		if (GetKey(L'S').bHeld)
			vCamera = Vector_Sub(vCamera, vForward);

		if (GetKey(L'Q').bHeld)
			fYaw -= 2.0f * fElapsedTime;

		if (GetKey(L'D').bHeld)
			fYaw += 2.0f * fElapsedTime;
		
		if (GetKey(L'W').bHeld)
		{
			if (timerWireframe <= 0)
			{
				timerWireframe = 0.3f;
				wireframe = !wireframe;
			}
		}
		if (GetKey(L'C').bHeld)
		{
			if (timerClipping <= 0)
			{
				timerClipping = 0.3f;
				showClipping = !showClipping;
			}
		}

		Fill(0, 0, ScreenWidth(), ScreenHeight(), PIXEL_SOLID, FG_BLACK);

		mat4x4 matRotZ;
		mat4x4 matRotX;

		matRotZ = Matrix_MakeRotationZ(fTheta * 0.5f);
		matRotX = Matrix_MakeRotationX(fTheta);

		mat4x4 matTrans;
		matTrans = Matrix_MakeTranslation(0.0f, 0.0f, 16.0f);

		mat4x4 matWorld;
		matWorld = Matrix_MakeIdentity();
		matWorld = Matrix_MultiplyMatrix(matRotZ, matRotX);
		matWorld = Matrix_MultiplyMatrix(matWorld, matTrans);

		vec3d vTarget = {0, 0, 1};

		mat4x4 matCameraRot = Matrix_MakeRotationY(fYaw);
		vLookDir = Matrix_MultiplyVector(matCameraRot, vTarget);
		vTarget = Vector_Add(vCamera, vLookDir);

		mat4x4 matCamera = Matrix_PointAt(vCamera, vTarget, vUp);

		mat4x4 matView = Matrix_QuickInverse(matCamera);

		std::vector<triangle> vecTrianglesToRaster;

		for (auto mesh : meshes)
		{
		for (auto tri : mesh.tris)
		{
			triangle triProjected;
			triangle triTransformed;
			triangle triViewed;

			for (int i = 0; i < DIMENSIONS; i++)
			{
				triTransformed.p[i] = Matrix_MultiplyVector(matWorld, tri.p[i]);
			}

			vec3d normal;
			vec3d line1;
			vec3d line2;
			line1 = Vector_Sub(triTransformed.p[1], triTransformed.p[0]);
			line2 = Vector_Sub(triTransformed.p[2], triTransformed.p[0]);

			normal = Vector_CrossProduct(line1, line2);

			normal = Vector_Normalise(normal);

			vec3d vCameraRay = Vector_Sub(triTransformed.p[0], vCamera);

			if (Vector_DotProduct(normal, vCameraRay) < 0)
			{
				// Illumination
				vec3d light_direction = {0.0f, 1.0f, -1.0f};
				light_direction = Vector_Normalise(light_direction);

				// How similar is normal to light direction (dot product)
				float dp = std::max(0.1f, Vector_DotProduct(light_direction, normal));

				CHAR_INFO c = static_cast<CHAR_INFO>(GetColour(dp));
				triTransformed.col = c.colour;
				triTransformed.sym = c.glyph;

				// Convert World Space --> View Space
				for (int i = 0; i < DIMENSIONS; i++)
				{
					triViewed.p[i] = Matrix_MultiplyVector(matView, triTransformed.p[i]);
				}
				triViewed.sym = triTransformed.sym;
				triViewed.col = triTransformed.col;

				// Clip Viewed Triangle against near plane, this could form two additional
				// additional triangles.
				int nClippedTriangles = 0;
				triangle clipped[2];
				nClippedTriangles = Triangle_ClipAgainstPlane({0.0f, 0.0f, 0.1f}, {0.0f, 0.0f, 1.0f}, triViewed, clipped[0], clipped[1], showClipping);

				// We may end up with multiple triangles from the clip, so project as
				// required
				for (int n = 0; n < nClippedTriangles; n++)
				{
					// Project triangles from 3D --> 2D
					for (int i = 0; i < DIMENSIONS; i++)
					{
						triProjected.p[i] = Matrix_MultiplyVector(matProj, clipped[n].p[i]);
					}
					triProjected.col = clipped[n].col;
					triProjected.sym = clipped[n].sym;

					// Scale into view, we moved the normalising into cartesian space
					// out of the matrix.vector function from the previous videos, so
					// do this manually
					for (int i = 0; i < DIMENSIONS; i++)
					{
						triProjected.p[i] = Vector_Div(triProjected.p[i], triProjected.p[i].w);
					}

					// X/Y are inverted so put them back
					for (int i = 0; i < DIMENSIONS; i++)
					{
						triProjected.p[i].x *= -1.0f;
						triProjected.p[i].y *= -1.0f;
					}

					// Offset verts into visible normalised space
					vec3d vOffsetView = {1, 1, 0};
					for (int i = 0; i < DIMENSIONS; i++)
					{
						triProjected.p[i] = Vector_Add(triProjected.p[i], vOffsetView);
					}
					for (int i = 0; i < DIMENSIONS; i++)
					{
						triProjected.p[i].x *= 0.5f * (float)ScreenWidth();
						triProjected.p[i].y *= 0.5f * (float)ScreenHeight();
					}

					// Store triangle for sorting
					vecTrianglesToRaster.push_back(triProjected);
				}
			}
		}
		}

		// Sort triangles from back to front
		sort(vecTrianglesToRaster.begin(), vecTrianglesToRaster.end(), [](triangle &t1, triangle &t2)
			 {
			float z1 = (t1.p[0].z + t1.p[1].z + t1.p[2].z) / 3.0f;
			float z2 = (t2.p[0].z + t2.p[1].z + t2.p[2].z) / 3.0f;
			return z1 > z2; });

		// Clear Screen
		Fill(0, 0, ScreenWidth(), ScreenHeight(), PIXEL_SOLID, FG_BLACK);

		// Loop through all transformed, viewed, projected, and sorted triangles
		for (auto &triToRaster : vecTrianglesToRaster)
		{
			// Clip triangles against all four screen edges, this could yield
			// a bunch of triangles, so create a queue that we traverse to
			//  ensure we only test new triangles generated against planes
			triangle clipped[2];
			std::list<triangle> listTriangles;

			// Add initial triangle
			listTriangles.push_back(triToRaster);
			int nNewTriangles = 1;

			for (int p = 0; p < 4; p++)
			{
				int nTrisToAdd = 0;
				while (nNewTriangles > 0)
				{
					// Take triangle from front of queue
					triangle test = listTriangles.front();
					listTriangles.pop_front();
					nNewTriangles--;

					// Clip it against a plane. We only need to test each
					// subsequent plane, against subsequent new triangles
					// as all triangles after a plane clip are guaranteed
					// to lie on the inside of the plane. I like how this
					// comment is almost completely and utterly justified
					switch (p)
					{
					case 0:
						nTrisToAdd = Triangle_ClipAgainstPlane({0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, test, clipped[0], clipped[1], showClipping);
						break;
					case 1:
						nTrisToAdd = Triangle_ClipAgainstPlane({0.0f, (float)ScreenHeight() - 1, 0.0f}, {0.0f, -1.0f, 0.0f}, test, clipped[0], clipped[1], showClipping);
						break;
					case 2:
						nTrisToAdd = Triangle_ClipAgainstPlane({0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, test, clipped[0], clipped[1], showClipping);
						break;
					case 3:
						nTrisToAdd = Triangle_ClipAgainstPlane({(float)ScreenWidth() - 1, 0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}, test, clipped[0], clipped[1], showClipping);
						break;
					}

					// Clipping may yield a variable number of triangles, so
					// add these new ones to the back of the queue for subsequent
					// clipping against next planes
					for (int w = 0; w < nTrisToAdd; w++)
						listTriangles.push_back(clipped[w]);
				}
				nNewTriangles = listTriangles.size();
			}

			// Draw the transformed, viewed, clipped, projected, sorted, clipped triangles
			for (auto &t : listTriangles)
			{
				FillTriangle(t.p[0].x, t.p[0].y, t.p[1].x, t.p[1].y, t.p[2].x, t.p[2].y, t.sym, t.col);
				if (wireframe)
					DrawTriangle(t.p[0].x, t.p[0].y, t.p[1].x, t.p[1].y, t.p[2].x, t.p[2].y, PIXEL_SOLID, FG_BLACK);
			}
		}

		return true;
	}
};

int main(int argc, char **argv)
{
	if (argc > 1)
	{
		for (int i = 1; i < argc; i++)
		{
			if (strcmp(argv[i] + strlen(argv[i]) - 4, ".obj") == 0)
			{
				objectFiles.push_back(argv[i]);
			}
		}
	}
	if (objectFiles.size() == 0)
	{
		printf("No object files specified!\n");
		return 0;
	}

	Engine3D engine;
	if (engine.ConstructConsole(500, 400, 2, 2))
		engine.Start();
	else
		printf("Failed to start engine!\n");
	return 0;
}