/**
 * a simple renderer
 * - adam defelice
 *
 * from lessons at https://github.com/ssloy/tinyrenderer
 *
 */

#include "tgaimage.h"
#include "model.h"
#include "util.h"
#include <cmath>
#include <iostream>
#include <vector>
#include <map>

const TGAColor WHITE = TGAColor(255, 255, 255, 255);
const TGAColor BLUE  = TGAColor(  0,   0, 255, 255);
const TGAColor GREEN = TGAColor(  0, 255,   0, 255);
const TGAColor RED   = TGAColor(255,   0,   0, 255);
const TGAColor TEAL  = TGAColor( 22, 255, 255, 255);

const int SCALE  = 800;
const int WIDTH  = 800;
const int HEIGHT = 800;

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

// convert from world coordinates to screen coordinates
// add 1 to each point to make all numbers positive, then scale by dimension
Vec3f normalize(Vec3f triangle) {
	return Vec3f(
		(triangle.x+1.0)*SCALE/2,
		(triangle.y+1.0)*SCALE/2,
		(triangle.z+1.0)*SCALE/2
	);
}

// calculate the RGBA color for triangle abc, according to light
TGAColor get_color(Vec3f a, Vec3f b, Vec3f c) {

	// vector describing directional light source
	// this is kinda broken
	Vec3f light_source(0, 0, 1);

	// gotta find the angle between the light source and a normal of the triangle
	Vec3f n = get_normal(a, b, c);

	// now find the angle between the directional light and the normal (theta from here forwards)
	double cos_theta = dot_product(n, light_source) / vector_magnitude(n, Vec3f(0,0,0)) * vector_magnitude(light_source, Vec3f(0,0,0));

	int brightness = 255 * cos_theta;
	int alpha;
	// triangles that aren't getting hit by light get an alpha of 0
	(brightness < 0) ? alpha = 0 : alpha = 255;

	return TGAColor(brightness, brightness, brightness, alpha);
}

// rasterize the triangle described by vertices a b c onto the passed TGAImage
void draw_triangle(Vec3f v[], Vec3f vt[], TGAColor face_color, TGAImage &image, double *zbuffer, TGAImage &texture) {

	Vec3f a = v[0];
	Vec3f b = v[1];
	Vec3f c = v[2];
	Vec3f at = vt[0]; // .x = u, .y = v, .z = 0
	Vec3f bt = vt[1];
	Vec3f ct = vt[2];

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
	if (face_color.a <= 0) return;

	// iterate over each point in the bounding box
	for (int x = x_min; x < x_max + 1; ++x) {
		for (int y = y_min; y < y_max + 1; ++y) {
			Vec2i point(x, y);
			// find the barycentric weight point P within the triangle
			Vec3f barycentric_weights = barycentric(point, a, b, c);
			// if any of those barycentric weights is less 0, then the point isn't in the triangle
			if (!(barycentric_weights.x < 0 || barycentric_weights.y < 0 || barycentric_weights.z < 0)) {
				// draw the point if it's in the triangle & is of the lowest z value we've encountered
				// (lazily) get the cartesian z coordinate for point (x, y) from the barycentric weights we calculated
				double z = 0;
				z += a.z * barycentric_weights.x; // u
				z += b.z * barycentric_weights.y; // v
				z += c.z * barycentric_weights.z; // w


				// okay
				// we need to find a the point in (at, bt, ct) that corresponds with (a, b, c)
				// we have: a, b, c, point p, at.uv, bt.uv, ct.uv

				// the point p is now encoded as three barycentric weights
				// use our point p to find the correct part the texture
				double x_t = 0;
				x_t += at.x * barycentric_weights.x;
				x_t += bt.x * barycentric_weights.y;
				x_t += ct.x * barycentric_weights.z;
				double y_t = 0;
				y_t += at.y * barycentric_weights.x;
				y_t += bt.y * barycentric_weights.y;
				y_t += ct.y * barycentric_weights.z;

				// check in with our z buffer
				if (zbuffer[x + y * WIDTH] < z) {
					// add our new highest z value
					zbuffer[x + y * WIDTH] = z;
					// map a color from the texture to the pixel we're drawing
					TGAColor tex_color = texture.get(
						x_t * (texture.get_width()),
						y_t * (texture.get_height())
					);
					// THROW SHADE *snap* *snap* *snap*
					TGAColor pixel_color(
						(int)tex_color.r * (int)face_color.r / 255.0,
						(int)tex_color.g * (int)face_color.r / 255.0,
						(int)tex_color.b * (int)face_color.r / 255.0,
						255
					);
					// draw
					image.set(x, y, pixel_color);
				}
			}
		}
	}
}


/**
 *
 */
void draw_object(Model &m, TGAImage &image) {

	// init our world z buffer
	// it'll be using screen coordinates
  double *zbuffer = new double[WIDTH * HEIGHT];
	for (int i = 0; i < WIDTH * HEIGHT; i++) {
		zbuffer[i] = 0;
	}

	// load texture .tga
	TGAImage texture;
	texture.read_tga_file("african_head_diffuse.tga");
	texture.flip_vertically();

	// for each of the object's faces (each triangle)
	for (int i = 0; i < m.nfaces(); ++i) {

		// get the positional and texture vertices of the triangle
		std::vector<int> face_v = m.face_v(i);   // 3 3d position vertices (x, y, z)
		std::vector<int> face_vt = m.face_vt(i); // 3 2d texture vertices
		Vec3f position_vertices[3];
		Vec3f texture_vertices[3];
		for (int i = 0; i < 3; ++i) {
			position_vertices[i] = Vec3f(
				m.vert(face_v[i]).x,
				m.vert(face_v[i]).y,
				m.vert(face_v[i]).z
			);
			texture_vertices[i] = Vec3f(
				m.vert_t(face_vt[i]).x,
				m.vert_t(face_vt[i]).y,
				m.vert_t(face_vt[i]).z
			);
		}

		// get the face's color from lighting
		TGAColor face_color = get_color(position_vertices[0], position_vertices[1], position_vertices[2]);

		// normalize vertices to screen coordinates and draw
		for (int i = 0; i < 3; ++i) {
			position_vertices[i] = normalize(position_vertices[i]);
		}
		draw_triangle(position_vertices, texture_vertices, face_color, image, zbuffer, texture);
	}

	delete[] zbuffer;
}

int main(int argc, char* argv[]) {
	// construct output image
	TGAImage scene(WIDTH, HEIGHT, TGAImage::RGB);

	// add a background because the glare on my screen is fierce
	for (int i = 0; i < WIDTH; ++i) {
		for (int j = 0; j < HEIGHT; ++j) {
			scene.set(i, j, TGAColor(200, 200, 200, 255));
		}
	}

	// load model .obj
	Model model("./african_head.obj");

	// draw points
	draw_object(model, scene);
	scene.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	scene.write_tga_file("output.tga");
	return 0;
}
