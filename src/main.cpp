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
#include <array>
#include <map>

const auto WIDTH = 2048;
const auto HEIGHT = 2048;
const auto AREA = WIDTH * HEIGHT;

// convert from world coordinates to screen coordinates
// add 1 to each point to make all numbers positive, then scale by dimension
Vec3f convert_to_screen_coordinates(Vec3f point) {
	auto c = 4; // camera's distance from the origin in the positive z direction

	// project onto the plane z=1
	auto x = point.x / (1 - point.z / c);
	auto y = point.y / (1 - point.z / c);
	auto z = point.z / (1 - point.z / c);

	auto scale = std::sqrt(AREA);
	return Vec3f(
		float((x + 1.0) * scale / 2),
		float((y + 1.0) * scale / 2),
		float((z + 1.0) * scale / 2)
	);
}

// calculate the RGBA illumination for normal n, according to directional light
// BUG?: this operation ignores occlusion by other faces!
TGAColor get_illumination(Vec3f &normal, Vec3f &light_source) {

	auto n = normal.normalize();
	auto l = light_source.normalize();

	// now find the angle between the directional light and the normal (theta from here forwards)
	auto cos_theta = dot_product(n, l) / n.norm() * l.norm();

	// cos_theta will be between -1.0 and 1.0
	auto brightness = static_cast<unsigned char>(std::round(200 * ((1 + cos_theta) / 2)));
	return TGAColor(brightness, brightness, brightness, 255);
}

// rasterize the triangle described by vertices a b c onto the passed TGAImage
void draw_face(Face &face, TGAImage &image, std::unique_ptr<std::array<double, AREA>> &zbuffer, TGAImage &texture, Vec3f &light_source) {

	// (a, b, c) describes the position of the face's vertices
	auto a = convert_to_screen_coordinates(face.get_vertices()[0].get_position());
	auto b = convert_to_screen_coordinates(face.get_vertices()[1].get_position());
	auto c = convert_to_screen_coordinates(face.get_vertices()[2].get_position());

	// (at, bt, ct) describes the (u, v) position of each vertex's corresponding texel
	auto at = face.get_vertices()[0].get_texture_coordinates();
	auto bt = face.get_vertices()[1].get_texture_coordinates();
	auto ct = face.get_vertices()[2].get_texture_coordinates();

	// (an, bn, cn) describes the vertices normal's (if provided)
	// we don't both calculating our own
	auto an = face.get_vertices()[0].get_normal();
	auto bn = face.get_vertices()[1].get_normal();
	auto cn = face.get_vertices()[2].get_normal();

	// of the triangle's corner vertices, find the maxes and mins of x and y.
	// those describe the bounding box
	std::vector<float> x_extrema = {a.x, b.x, c.x};
	std::vector<float> y_extrema = {a.y, b.y, c.y};

	// this is where we go from floating point to integral,
	// we find the integral bounding box that wraps the floating point vertices
	auto x_max = static_cast<int>(std::round(*max_element(x_extrema.begin(), x_extrema.end())));
	auto x_min = static_cast<int>(std::round(*min_element(x_extrema.begin(), x_extrema.end())));
	auto y_max = static_cast<int>(std::round(*max_element(y_extrema.begin(), y_extrema.end())));
	auto y_min = static_cast<int>(std::round(*min_element(y_extrema.begin(), y_extrema.end())));

	// iterate over each point in the bounding box
	for (auto x = x_min; x < x_max + 1; ++x) {
		// bounds check
		if (x < 0 || x >= WIDTH) continue;
		for (auto y = y_min; y < y_max + 1; ++y) {
			// bounds check
			if (y < 0 || y >= HEIGHT) continue;
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

				// we need to find a, the point in (at, bt, ct) that corresponds with (a, b, c)
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
				if ((*zbuffer)[x + y * WIDTH] < z) {
					// add our new highest z value
					(*zbuffer)[x + y * WIDTH] = z;
					// map a color from the texture to the pixel we're drawing
					TGAColor tex_color = texture.get(
						x_t * (texture.get_width()),
						y_t * (texture.get_height())
					);

					// shade
					// find the pixel's normal (ratio btwn three vertex normals) and interp lighting

					// calculate the distance between point (x,y) and the three face vertices for linear interp
					auto ad = (Vec3f(x, y, a.z) - a).norm();
					auto bd = (Vec3f(x, y, b.z) - b).norm();
					auto cd = (Vec3f(x, y, c.z) - c).norm();

					// multiply vertex normals (xn) by (x,y)'s distance to those vertices
					auto fragment_normal = an*((ad+bd+cd)/ad) + bn*((ad+bd+cd)/bd) + cn*((ad+bd+cd)/cd);
					auto fragment_illumination = get_illumination(fragment_normal, light_source);

					// interpolate texel color w/ fragment color from light
					TGAColor pixel_color(
						static_cast<unsigned char>(tex_color.r * fragment_illumination.r / 255),
						static_cast<unsigned char>(tex_color.g * fragment_illumination.g / 255),
						static_cast<unsigned char>(tex_color.b * fragment_illumination.b / 255),
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
void draw_model(Model &m, TGAImage &texture, TGAImage &image, std::unique_ptr<std::array<double, AREA>> &zbuffer, Vec3f &light_source) {
	// for each face
	// TODO: Models, as declared in model.h, do not have an iterable face collection :(
	for (auto i = 0; i < m.nfaces(); ++i) {
		draw_face(*(m.get_face(i)), image, zbuffer, texture, light_source);
	}
}

// render an image
int main(int argc, char *argv[]) {

	// create a light source
	// points from here towards origin
	// ignores occlusion
	// should make this a class
	auto light_source = Vec3f(3.0, 0.0, 1.0);
	//auto light_color = TGAColor(200, 200, 200);

	// load model
	// TODO: this boilerplate is not ideal, i should rewrite it
	auto model = Model("../data/african_head.obj");

	// load texture
	auto texture = TGAImage();
	texture.read_tga_file("../data/african_head_diffuse.tga");
	texture.flip_vertically();

	// init output image
	auto image = TGAImage(WIDTH, HEIGHT, TGAImage::RGB);
	// fill the image with a background color because the glare on my screen is fierce
	for (auto i = 0; i < WIDTH; ++i) {
		for (auto j = 0; j < HEIGHT; ++j) {
			image.set(i, j, TGAColor(200, 200, 200, 255));
		}
	}
	// init image z buffer
	auto zbuffer = std::make_unique<std::array<double, AREA>>();
	zbuffer->fill(0);

	// draw model to image
	draw_model(model, texture, image, zbuffer, light_source);

	// write image to file
	image.flip_vertically();
	image.write_tga_file("../data/output.tga");
	return 0;
}
