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

// returns normal vector originating at a
Vec3f get_normal(Vec3f a, Vec3f b, Vec3f c) {
	// derive two vectors from our three vertices
	// the vectors are arbitrary
	Vec3f ab(b.x-a.x, b.y-a.y, b.z-a.z);
	Vec3f ac(c.x-a.x, c.y-a.y, c.z-a.z);
	return cross_product(ab, ac);
}
