/**
 * a simple renderer
 * - adam defelice
 *
 * from lessons at https://github.com/ssloy/tinyrenderer
 *
 */

#include "tgaimage.h"
#include "model.h"
#include <cmath>
#include <iostream>
#include <vector>
#include <map>

const TGAColor WHITE = TGAColor(255, 255, 255, 255);
const TGAColor BLUE  = TGAColor(  0,   0, 255, 255);
const TGAColor GREEN = TGAColor(  0, 255,   0, 255);
const TGAColor RED   = TGAColor(255,   0,   0, 255);
const TGAColor TEAL  = TGAColor( 22, 255, 255, 255);

const int SCALE  = 1024;
const int WIDTH  = 1024;
const int HEIGHT = 1024;

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

/*
 * returns barycentric coordinates (u, v, 1) of point P in triangle v0v1v2
 * this is memory leak, better delete that vec when you're done
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
	// let's return the vector: ( (1-u-v), u, v )
	return Vec3f( (1 - c.x/c.z - c.y/c.z), c.x/c.z, c.y/c.z);
}

// convert from world coordinates to screen coordinates
// add 1 to each point to make all numbers positive, then scale by dimension
Vec3f normalize(Vec3f triangle) {
	return Vec3f(
		(triangle.x+1.0)*SCALE/2,
		(triangle.y+1.0)*SCALE/2,
		(triangle.z+1.0)*SCALE/2
	);
}

// calculate the RGBA color for triangle abc
TGAColor get_color(Vec3f a, Vec3f b, Vec3f c) {

	// vector describing directional light source
	Vec3f light_source(0, 0, 1);

	// gotta find the angle between the light source and a normal of the triangle
	Vec3f n = get_normal(a, b, c);

	// now find the angle between the directional light and the normal (theta from here forwards)
	double cos_theta = dot_product(n, light_source) / vector_magnitude(n, Vec3f(0,0,0)) * vector_magnitude(light_source, Vec3f(0,0,0));

	double brightness = 255 * cos_theta;
	double alpha;
	// triangles that aren't getting hit by light get an alpha of 0
	(brightness < 0) ? alpha = 0 : alpha = 255;

	return TGAColor(brightness, brightness, brightness, alpha);
}

// draw the triangle described by vertices a b c onto the passed TGAImage
void draw_triangle(Vec3f a, Vec3f b, Vec3f c, TGAColor color, TGAImage &image, double *zbuffer) {

	Vec3f vertices[3] = {a, b, c};
	// of the triangle's corner vertices, find the maxes and mins of x and y.
	// those describe the bounding box
	std::vector<float> x_extrema = {a.x, b.x, c.x};
	std::vector<float> y_extrema = {a.y, b.y, c.y};

	// this is turning our floats into ints. not sure if i want to do this here?
	int x_max = std::round(*max_element(x_extrema.begin(), x_extrema.end()));
	int x_min = std::round(*min_element(x_extrema.begin(), x_extrema.end()));
	int y_max = std::round(*max_element(y_extrema.begin(), y_extrema.end()));
	int y_min = std::round(*min_element(y_extrema.begin(), y_extrema.end()));

	// if there is no light, we don't need to draw the triangle at all
	if (color.a <= 0) return;

	// iterate over each point in the bounding box
	for (int x = x_min; x < x_max + 1; ++x) {
		for (int y = y_min; y < y_max + 1; ++y) {
			Vec2i point(x, y);
			// find the barycentric weight of each point in the bounding box
			Vec3f barycentric_weights = barycentric(point, a, b, c);
			// if any of those barycentric weights is less 0, then the point isn't in the triangle
			if (!(barycentric_weights.x < 0 || barycentric_weights.y < 0 || barycentric_weights.z < 0)) {
				// draw the point if it's in the triangle & is of the lowest z value we've encountered
				// so lazy
				double z = 0;
				z += vertices[0].z * barycentric_weights.x;
				z += vertices[1].z * barycentric_weights.y;
				z += vertices[2].z * barycentric_weights.z;
				if (zbuffer[x + y * WIDTH] < z) {
					zbuffer[x + y * WIDTH] = z;
					image.set(x, y, color);
				}
			}
		}
	}
}

void draw_object(Model &m, TGAImage &image) {

	// init our z buffer
	// it'll be using screen coordinates
  double *zbuffer = new double[WIDTH * HEIGHT];
	for (int i=0; i<WIDTH*HEIGHT; i++) {
		zbuffer[i] = 0;
	}

	// for each of the object's faces (each triangle)
	for (int i = 0; i < m.nfaces(); ++i) {

		// a face is a vector of ints, copy it and SWIPE ITS VECS
		std::vector<int> face = m.face(i);

		// store raw triangle vertices in an array
		Vec3f triangle[3];
		for (int i = 0; i < 3; ++i) {
			triangle[i] = Vec3f(
				m.vert(face[i]).x,
				m.vert(face[i]).y,
				m.vert(face[i]).z
			);
		}
		// get the face's color
		TGAColor face_color = get_color(triangle[0], triangle[1], triangle[2]);

		// normalize vertices to screen coordinates and draw
		for (int i = 0; i < 3; ++i) {
			triangle[i] = normalize(triangle[i]);
		}
		draw_triangle(triangle[0], triangle[1], triangle[2], face_color, image, zbuffer);
	}

	delete[] zbuffer;
}

int main(int argc, char* argv[]) {
	// construct output image
	TGAImage scene(WIDTH, HEIGHT, TGAImage::RGB);
	// load model
	Model model("./african_head.obj");
	// draw points
	draw_object(model, scene);
	scene.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	scene.write_tga_file("output.tga");
	return 0;
}