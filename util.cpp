#include "util.h"

// find the magnitude of the vector ab
// assumes R3
double vector_magnitude(Vec3f a, Vec3f b) {
	return sqrt( pow(b.x-a.x, 2) + pow(b.y-a.y, 2) + pow(b.z-a.z, 2) );
}

Vec3f cross_product(Vec3f a, Vec3f b) {
	return Vec3f(
		a.y*b.z - a.z*b.y,
		a.z*b.x - a.x*b.z,
		a.x*b.y - a.y*b.x
	);
}

// assumes we're in R3
double dot_product(Vec3f a, Vec3f b) {
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

/*
 * returns barycentric coordinates ( u, v, (1-u-v) ) of point P in triangle v0v1v2
 *
 */
Vec3f barycentric(Vec2i p, Vec3f v0, Vec3f v1, Vec3f v2) {

	// we could pre-compute the first two values of each vector
	// a = [(v2.x-v0.x) (v2.x-v1.x) (p.x-v2.x)]
	// X
	// b = [(v2.y-v0.y) (v2.y-v1.y) (p.y-v2.y)]
	// =
	// c
	Vec3f a(v2.x-v0.x, v2.x-v1.x, p.x-v2.x);
	Vec3f b(v2.y-v0.y, v2.y-v1.y, p.y-v2.y);
	Vec3f c(cross_product(a, b));

	// the cross product vec = (u, v, 1), so now i have to transform it so that z actually equals 1
	// let's return the vector: ( u, v, (1-u-v) )
	return Vec3f( c.x/c.z, c.y/c.z, (1 - c.x/c.z - c.y/c.z));
}