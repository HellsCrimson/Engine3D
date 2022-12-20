#include "Utils.hpp"

int Triangle_ClipAgainstPlane(vec3d plane_p, vec3d plane_n, triangle &in_tri, triangle &out_tri1, triangle &out_tri2, bool showClipping)
{
    // Make sure plane normal is indeed normal
    plane_n = Vector_Normalise(plane_n);

    // Return signed shortest distance from point to plane, plane normal must be normalised
    auto dist = [&](vec3d &p)
    {
        vec3d n = Vector_Normalise(p);
        return (plane_n.x * p.x + plane_n.y * p.y + plane_n.z * p.z - Vector_DotProduct(plane_n, plane_p));
    };

    // Create two temporary storage arrays to classify points either side of plane
    // If distance sign is positive, point lies on "inside" of plane
    vec3d *inside_points[3];
    int nInsidePointCount = 0;
    vec3d *outside_points[3];
    int nOutsidePointCount = 0;

    // Get signed distance of each point in triangle to plane
    float d0 = dist(in_tri.p[0]);
    float d1 = dist(in_tri.p[1]);
    float d2 = dist(in_tri.p[2]);

    if (d0 >= 0)
    {
        inside_points[nInsidePointCount++] = &in_tri.p[0];
    }
    else
    {
        outside_points[nOutsidePointCount++] = &in_tri.p[0];
    }
    if (d1 >= 0)
    {
        inside_points[nInsidePointCount++] = &in_tri.p[1];
    }
    else
    {
        outside_points[nOutsidePointCount++] = &in_tri.p[1];
    }
    if (d2 >= 0)
    {
        inside_points[nInsidePointCount++] = &in_tri.p[2];
    }
    else
    {
        outside_points[nOutsidePointCount++] = &in_tri.p[2];
    }

    // Now classify triangle points, and break the input triangle into
    // smaller output triangles if required. There are four possible
    // outcomes...

    if (nInsidePointCount == 0)
    {
        // All points lie on the outside of plane, so clip whole triangle
        // It ceases to exist

        return 0; // No returned triangles are valid
    }

    if (nInsidePointCount == 3)
    {
        // All points lie on the inside of plane, so do nothing
        // and allow the triangle to simply pass through
        out_tri1 = in_tri;

        return 1; // Just the one returned original triangle is valid
    }

    if (nInsidePointCount == 1 && nOutsidePointCount == 2)
    {
        // Triangle should be clipped. As two points lie outside
        // the plane, the triangle simply becomes a smaller triangle

        // Copy appearance info to new triangle
        if (showClipping)
            out_tri1.col = FG_BLUE;
        else
            out_tri1.col = in_tri.col;
        out_tri1.sym = in_tri.sym;

        // The inside point is valid, so keep that...
        out_tri1.p[0] = *inside_points[0];

        // but the two new points are at the locations where the
        // original sides of the triangle (lines) intersect with the plane
        out_tri1.p[1] = Vector_IntersectPlane(plane_p, plane_n, *inside_points[0], *outside_points[0]);
        out_tri1.p[2] = Vector_IntersectPlane(plane_p, plane_n, *inside_points[0], *outside_points[1]);

        return 1; // Return the newly formed single triangle
    }

    if (nInsidePointCount == 2 && nOutsidePointCount == 1)
    {
        // Triangle should be clipped. As two points lie inside the plane,
        // the clipped triangle becomes a "quad". Fortunately, we can
        // represent a quad with two new triangles

        // Copy appearance info to new triangles
        if (showClipping)
            out_tri1.col = FG_GREEN;
        else
            out_tri1.col = in_tri.col;
        out_tri1.sym = in_tri.sym;

        if (showClipping)
            out_tri2.col = FG_RED;
        else
            out_tri2.col = in_tri.col;
        out_tri2.sym = in_tri.sym;

        // The first triangle consists of the two inside points and a new
        // point determined by the location where one side of the triangle
        // intersects with the plane
        out_tri1.p[0] = *inside_points[0];
        out_tri1.p[1] = *inside_points[1];
        out_tri1.p[2] = Vector_IntersectPlane(plane_p, plane_n, *inside_points[0], *outside_points[0]);

        // The second triangle is composed of one of he inside points, a
        // new point determined by the intersection of the other side of the
        // triangle and the plane, and the newly created point above
        out_tri2.p[0] = *inside_points[1];
        out_tri2.p[1] = out_tri1.p[2];
        out_tri2.p[2] = Vector_IntersectPlane(plane_p, plane_n, *inside_points[1], *outside_points[0]);

        return 2; // Return two newly formed triangles which form a quad
    }
    return 0;
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
        bg_col = BG_WHITE;
        fg_col = FG_WHITE;
    }

    CHAR_INFO c;
    c.colour = bg_col | fg_col;
    c.glyph = PIXEL_SOLID;
    return c;
}