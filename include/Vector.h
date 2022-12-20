#ifndef VECTOR_H
#define VECTOR_H

#include <math.h>
#include "Geometry.h"

vec3d Vector_Add(vec3d &v1, vec3d &v2);
vec3d Vector_Sub(vec3d &v1, vec3d &v2);
vec3d Vector_Mul(vec3d &v1, float k);
vec3d Vector_Div(vec3d &v1, float k);
float Vector_DotProduct(vec3d &v1, vec3d &v2);
float Vector_Length(vec3d &v);
vec3d Vector_Normalise(vec3d &v);
vec3d Vector_CrossProduct(vec3d &v1, vec3d &v2);
vec3d Vector_IntersectPlane(vec3d &plane_p, vec3d &plane_n, vec3d &lineStart, vec3d &lineEnd);

#endif // VECTOR_H