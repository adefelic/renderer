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
#include <array>
#include <map>

const TGAColor WHITE = TGAColor(255, 255, 255, 255);
const TGAColor BLUE  = TGAColor(  0,   0, 255, 255);
const TGAColor GREEN = TGAColor(  0, 255,   0, 255);
const TGAColor RED   = TGAColor(255,   0,   0, 255);
const TGAColor TEAL  = TGAColor( 22, 255, 255, 255);

const auto WIDTH  = 800;
const auto HEIGHT = 800;
const auto AREA = WIDTH*HEIGHT;

// convert from world coordinates to screen coordinates
// add 1 to each point to make all numbers positive, then scale by dimension
Vec3f convert_to_screen_coords(Vec3f point) {
	int c = 4; // camera's distance from the origin in the positive z direction

	// project onto the plane z=1
	double x = point.x / (1 - point.z/c);
	double y = point.y / (1 - point.z/c);
	double z = point.z / (1 - point.z/c);


	int scale = std::sqrt(AREA);
	return Vec3f(
		(x+1.0)*scale/2,
		(y+1.0)*scale/2,
		(z+1.0)*scale/2
	);
}

// calculate the RGBA color for triangle abc, according to light
TGAColor get_illumination(Face &face) {

	// vector describing directional light source
	// this is totally broken
	auto light_source = Vec3f(0, 0, 1);

	// TODO: I don't think i'm calculating normals well
	auto n = Vec3f(face.get_normal());

	// now find the angle between the directional light and the normal (theta from here forwards)
	auto cos_theta = dot_product(n, light_source) / vector_magnitude(n, Vec3f(0,0,0)) * vector_magnitude(light_source, Vec3f(0,0,0));

	int brightness = 255 * cos_theta;
	return TGAColor(brightness, brightness, brightness, 255);
}

// rasterize the triangle described by vertices a b c onto the passed TGAImage
void draw_face(const std::vector<Vec3f> &v, const std::vector<Vec2f> &vt, TGAColor face_color, TGAImage &image, std::array<double, AREA> &zbuffer, TGAImage &texture) {

	auto a = convert_to_screen_coords(v[0]);
	auto b = convert_to_screen_coords(v[1]);
	auto c = convert_to_screen_coords(v[2]);
	auto at = vt[0];
	auto bt = vt[1];
	auto ct = vt[2];

	// of the triangle's corner vertices, find the maxes and mins of x and y.
	// those describe the bounding box
	std::vector<float> x_extrema = {a.x, b.x, c.x};
	std::vector<float> y_extrema = {a.y, b.y, c.y};

	// this is where we go from floating point to integral
	int x_max = std::round(*max_element(x_extrema.begin(), x_extrema.end()));
	int x_min = std::round(*min_element(x_extrema.begin(), x_extrema.end()));
	int y_max = std::round(*max_element(y_extrema.begin(), y_extrema.end()));
	int y_min = std::round(*min_element(y_extrema.begin(), y_extrema.end()));

	// iterate over each point in the bounding box
	for (int x = x_min; x < x_max + 1; ++x) {
		// bounds check
		if (x < 0 || x >= WIDTH) continue;
		for (int y = y_min; y < y_max + 1; ++y) {
			// bounds check
			if (y < 0 || y >= HEIGHT ) continue;

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
					// shade
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

// draw a model to an image
void draw_model(Model &m, TGAImage &texture, TGAImage &image) {

	// init z buffer
  std::array<double, AREA> zbuffer{};
  zbuffer.fill(0);

	// for each face
	// TODO: Models, as declared in model.h, do not have an iterable face collection :(
	for (auto i = 0; i < m.nfaces(); ++i) {
		auto face = m.get_face(i);
		auto illumination = get_illumination(*face);
		if (illumination.r <= 0) continue;
		draw_face(face->get_vertices(), face->get_texture_coordinates(), illumination, image, zbuffer, texture);
	}
}

// render an image
int main(int argc, char* argv[]) {

	// load model
	// TODO: this boilerplate is not ideal, i should rewrite it
	auto model = Model("./african_head.obj");

	// load texture
	auto texture = TGAImage();
	texture.read_tga_file("african_head_diffuse.tga");
	texture.flip_vertically();

	// initialize output image
	auto image = TGAImage(WIDTH, HEIGHT, TGAImage::RGB);
	// fill the image with a background color because the glare on my screen is fierce
	for (auto i = 0; i < WIDTH; ++i) {
		for (auto j = 0; j < HEIGHT; ++j) {
			image.set(i, j, TGAColor(200, 200, 200, 255));
		}
	}

	// draw model to image
	draw_model(model, texture, image);

	// write image to file
	image.flip_vertically();
	image.write_tga_file("output.tga");
	return 0;
}
