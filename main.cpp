/**
 * a simple renderer
 * - adam defelice
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

enum DRAW_MODE {WIRE, FILL};

// returns a vector of points on the heap
// this is a memory leak if you don't deallocate the returned vector :X
std::vector<Vec3i*> line(Vec2i v0, Vec2i v1, TGAImage &image, TGAColor color, bool draw) {

	int x0 = v0.x;
	int y0 = v0.y;
	int x1 = v1.x;
	int y1 = v1.y;

	// this vector will hold the points that make up the line. we'll be returning it.
	std::vector<Vec3i*> points;

	/* DETERMINE Y VALUES FOR EACH X */
	bool yTransposed = false;
	// if the rise is greater than the run, transpose
	// we do this before potentially swapping points so that we don't end up un-swapping them
	if (std::abs(y1 - y0) > std::abs(x1 - x0)) {
		std::swap(x0, y0);
		std::swap(x1, y1);
		yTransposed = true;
	}

	// ensure that (x0, y0) is the leftmost point (closest to zero on the positive side)
	if (std::abs(x0) > std::abs(x1)) {
		std::swap(x0, x1);
		std::swap(y0, y1);
	}

	int dy = y1 - y0;
	int dx = x1 - x0;
	float slope = (float)dy/(float)dx; // we're preserving the fraction here
	float error = 0.0;

	// iterate over each x value on the line
	int y = y0;
	//printf("x0: %d -> x1: %d\n", x0, x1);
	// draw the points, first point is (x0, y0)
	for (int x = x0; x <= x1; x++) {

		// handle transposition
		int x_to_draw;
		int y_to_draw;
		if (yTransposed) {
			x_to_draw = y;
			y_to_draw = x;
		} else {
			x_to_draw = x;
			y_to_draw = y;
		}

		// record the point, maybe draw
		Vec3i *point = new Vec3i(x_to_draw, y_to_draw, 0);
		points.push_back(point);
		if (draw) image.set(x_to_draw, y_to_draw, color);

		// add overflow for the next point
		// TODO? the offset can only ever deliver 1 y of error per x
		error += slope;
		if (error >= 1.0) {
			error--;
			y++;
		} else if (error <= -1.0) {
			error++;
			y--;
		}
	}

	printf("\n");
	return points;
}

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
float normalize(float f) {
	return (f+1.0)*SCALE/2;
}

// calculate the RGBA color for triangle abc
TGAColor get_color(Vec3f a, Vec3f b, Vec3f c, TGAImage &image) {
	// unit vector describing directional light
	Vec3f light_source(0, 0, 1);
	// gotta find the angle between the light source and a normal of the triangle
	// find the triangle's normal, originating at point a
	Vec3f n = get_normal(a, b, c);

	// now find the angle between the directional light and the normal
	// we should make the normal into a unit vector
	Vec3f unit_normal(
		n.x - a.x,
		n.y - a.y,
		n.z - a.z
	);
	// here we get the cosine of theta
	double cos_theta = dot_product(unit_normal, light_source) / vector_magnitude(unit_normal, Vec3f(0,0,0)) * vector_magnitude(light_source, Vec3f(0,0,0));
	// we're finding the square root just to make the color shifts a little bit less extreme
	double brightness = 255 * sqrt(cos_theta);

	// degenerate triangles are getting a brightness of 0
	if (brightness < 0) brightness = 0;

	return TGAColor(brightness, brightness, brightness, 255);
}

// draw the triangle described by vertices a b c onto the passed TGAImage
void draw_triangle(Vec3f a, Vec3f b, Vec3f c, TGAImage &image) {

	// of the triangle's corner vertices, find the maxes and mins of x and y.
	// those describe the bounding box
	std::vector<float> x_extrema = {a.x, b.x, c.x};
	std::vector<float> y_extrema = {a.y, b.y, c.y};

	// this is turning our floats into ints. not sure if i want to do this here?
	int x_max = std::round(*max_element(x_extrema.begin(), x_extrema.end()));
	int x_min = std::round(*min_element(x_extrema.begin(), x_extrema.end()));
	int y_max = std::round(*max_element(y_extrema.begin(), y_extrema.end()));
	int y_min = std::round(*min_element(y_extrema.begin(), y_extrema.end()));

	TGAColor color = get_color(a, b, c, image);

	// if there is no light, we don't need to draw the triangle at all
	if (color.r <= 0) return;

	// iterate over each point in the bounding box
	for (int x = x_min; x < x_max + 1; ++x) {
		for (int y = y_min; y < y_max + 1; ++y) {
			Vec2i point(x, y);
			// find the barycentric weight of each point in the bounding box
			Vec3f u_v_1 = barycentric(point, a, b, c);
			// if any of those barycentric weights is less 0, then the point isn't in the triangle
			if (!(u_v_1.x < 0 || u_v_1.y < 0 || u_v_1.z < 0)) {
				// draw the point if it's in the triangle
				image.set(x, y, color);
			}
		}
	}
}

void draw_object(Model &m, TGAImage &image) {
	// for each of the object's faces (each triangle)
	for (int i = 0; i < m.nfaces(); ++i) {
		//TGAColor color(rand()%255, rand()%255, rand()%255, 255);

		// a face is a vector of ints, copy it and SWIPE ITS VECS
		std::vector<int> face = m.face(i);

		// normalize!
		// add 1 to each point to make all numbers positive, then scale by dimension
		Vec3f v0_n(
			normalize(m.vert(face[0]).x),
			normalize(m.vert(face[0]).y),
			normalize(m.vert(face[0]).z)
		);
		Vec3f v1_n(
			normalize(m.vert(face[1]).x),
			normalize(m.vert(face[1]).y),
			normalize(m.vert(face[1]).z)
		);
		Vec3f v2_n(
			normalize(m.vert(face[2]).x),
			normalize(m.vert(face[2]).y),
			normalize(m.vert(face[2]).z)
		);

		draw_triangle(v0_n, v1_n, v2_n, image);

	}
}

// [executable] wire
//   - draws wireframe instead of fill
int main(int argc, char* argv[]) {

	// parse args
	DRAW_MODE mode = FILL;
	if (argc > 1) {
		if (strncmp(argv[1], "wire", 4) == 0) mode = WIRE;
	}

	TGAImage image(WIDTH, HEIGHT, TGAImage::RGB);
	Model model("./african_head.obj");
	draw_object(model, image);
	//draw_triangle(Vec3f(1,0,1), Vec3f(3,0,3), Vec3f(4,0,4), image );
	image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	image.write_tga_file("output.tga");
	return 0;
}