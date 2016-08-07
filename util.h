#ifndef __UTIL_H__
#define __UTIL_H__

#include "geometry.h"

double vector_magnitude(Vec3f a, Vec3f b);
double dot_product(Vec3f a, Vec3f b);
Vec3f cross_product(Vec3f a, Vec3f b);
Vec3f get_normal(Vec3f a, Vec3f b, Vec3f c);

#endif //__UTIL_H__
