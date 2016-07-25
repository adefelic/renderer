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
// a fact: this is the only function that calls image.set()
std::vector<Vec3i*> line(Vec2i *v0, Vec2i *v1, TGAImage &image, TGAColor color, std::map<Vec2i, int> &z_values, bool draw) {


	int x0 = v0->x;
	int y0 = v0->y;
	int x1 = v1->x;
	int y1 = v1->y;

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

/*
	// i have a feeling that this will skip the bottom line
	// points = triangle edge points
	// the bug here could be from there not being pairs on the stack
	void draw_triangle(std::vector<Vec3i*> &points, TGAImage &image, TGAColor color, std::map<Vec2i, int> &z_values) {

		// this vector is for tracking points on the same y
		std::vector<Vec3i*> horizontal_points;
		int previous_y;
		std::cout << "let's draw a triangle" << std::endl;
		for (std::vector<Vec3i*>::iterator i = points.begin(); i != points.end(); ++i) {

			// push the first point of the face
			if (i == points.begin()) {
				horizontal_points.push_back(*i);
				previous_y = (*i)->y;
				continue;
			}

			// if the current point is on the same y, push it
			if ((*i)->y == previous_y) {
				horizontal_points.push_back(*i);
			} else {
				// if it's not, draw the line, clear the vector, then push the point
				// make sure the points are sorted by x
				std::sort(
					horizontal_points.begin(),
					horizontal_points.end(),
					[] (Vec3i *a, Vec3i *b) { return a->x < b->x; }
				);
				Vec3i v0(
					horizontal_points.front()->x,
					horizontal_points.front()->y,
					horizontal_points.front()->z
				);
				Vec3i v1(
					horizontal_points.back()->x,
					horizontal_points.back()->y,
					horizontal_points.back()->z
				);
				// draw + fill
				std::cout << "drawing line at y=" << v0.y << std::endl;
				line(&v0, &v1, image, color, z_values, true);
				horizontal_points.clear(); // forget lines without partners
				horizontal_points.push_back(*i); // start the vec anew, with a new y
				previous_y = (*i)->y;
			}
		}
	}

	// this is inefficient, non-edge lines are each drawn twice
	// also i'm ignoring the draw mode
	void draw_object(Model &m, TGAImage &image, TGAColor color, bool z_culling, DRAW_MODE mode) {

		// let's cull some z
		// *shotgun pump sound*
		std::map<Vec2i, int> z_values;

		// for each of the object's faces
		for (int i = 0; i < m.nfaces(); ++i) {

			// a face is a vector of ints, copy it
			std::vector<int> face = m.face(i);
			// we'll store three lines of points at a time
			// this is going to allocate some pointers to memory, so we'll have to clear it later
			std::vector<Vec3i*> points_face;

			// for each line of the face's three lines
			for (int j = 0; j < 3; ++j) {

				// NORMALIZE POINTS
				Vec3f v0 = m.vert(face[j]);
				Vec3f v1 = m.vert(face[(j+1)%3]);
				// add 1 to each point to make all numbers positive, then scale by dimension
				Vec3i v0_i(
					(v0.x+1.0)*WIDTH/2,
					(v0.y+1.0)*HEIGHT/2,
					(v0.z+1.0)*DEPTH/2
				);
				Vec3i v1_i(
					(v1.x+1.0)*WIDTH/2,
					(v1.y+1.0)*HEIGHT/2,
					(v1.z+1.0)*DEPTH/2
				);

				// if we're not filling the triangles, we will just draw their outlines and leave
				if (mode == WIRE) {
					line(&v0_i, &v1_i, image, color, z_values, true);
				} else {
					// get the points of the triangle's outer lines, but don't waste time drawing them
					std::vector<Vec3i*> points_line = line(&v0_i, &v1_i, image, color, z_values, false);
					// merge the face's outline vertices with our running collection of face outline vertices
					points_face.insert(points_face.begin(), points_line.begin(), points_line.end());
				}
			}

			// we have all of the outer points of the triangle
			// sort the points by y value (with a lambda)
			std::sort(
				points_face.begin(),
				points_face.end(),
				[] (Vec3i *a, Vec3i *b) { return a->y < b->y; }
			);
			std::cout << "sorted edge vertices:" << std::endl;
			for (std::vector<Vec3i*>::iterator j = points_face.begin(); j != points_face.end(); ++j) {
				printf("(%d, %d, %d)\n", (*j)->x, (*j)->y, (*j)->z);
			}
			// draw the triangle naively
			draw_triangle(points_face, image, TGAColor(rand()%255, rand()%255, rand()%255, 255), z_values);

			// deallocate memory
			for (std::vector<Vec3i*>::iterator it = points_face.begin(); it != points_face.end(); ++it) {
				delete (*it);
			}
			points_face.clear();
		} // foreach face
	}
*/

/*
 * returns barycentric coordinates (u, v, 1) of point P in triangle v0v1v2
 * this is memory leak, better delete that vec when you're done
 */
Vec3f barycentric(Vec2i p, Vec3f v0, Vec3f v1, Vec3f v2) {

	// we could pre-compute the first two values of each vector
	// [(v2.x-v0.x) (v2.x-v1.x) (p.x-v2.x)]
	// X
	// [(v2.y-v0.y) (v2.y-v1.y) (p.y-v2.y)]
	float Ax = (v2.x-v0.x);
	float Ay = (v2.y-v0.y);
	float Bx = (v2.x-v1.x);
	float By = (v2.y-v1.y);
	float Cx = (p.x-v2.x);
	float Cy = (p.y-v2.y);
		/*
		float Ax = (v2.x-v0.x);
		float Ay = (v2.y-v0.y);
		float Bx = (v1.x-v0.x);
		float By = (v1.y-v0.y);
		float Cx = (v0.x-p.x);
		float Cy = (v0.y-p.y);
		*/
	float x = (Bx*Cy) - (By*Cx);
	float y = (Cx*Ay) - (Cy*Ax);
	float z = (Ax*By) - (Ay*Bx);
	//printf("triangle: (%f,%f),(%f,%f),(%f,%f)\n", v0.x, v0.y, v1.x, v1.y, v2.x, v2.y);
	//printf("point:    (%i,%i)\n", p.x, p.y);
	//printf("cross: %f %f %f\n", x, y, z);
	// the cross product vec = (u, v, 1), so now i have to transform it so that z actually equals 1
	// let's return (1-u-v), u, v
	//printf("u, v: %f, %f\n", x/z, y/z);
	if (std::abs(z) < 1) return Vec3f(-1, -1, -1); // degenerate triangle
	return Vec3f( (1-x/z-y/z), x/z, y/z);
}

// convert from world coordinates to screen coordinates
float normalize(float f) {
	return (f+1.0)*SCALE/2;
}

// find the bounding box, draw the triangle
void draw_triangle(Vec3f v0, Vec3f v1, Vec3f v2, TGAImage &image, TGAColor color) {

	// of the triangle's corner vertices, find the maxes and mins of x and y.
	// those describe the bounding box
	std::vector<float> x_extrema = {v0.x, v1.x, v2.x};
	std::vector<float> y_extrema = {v0.y, v1.y, v2.y};

	// this is turning our floats into ints. not sure if i want to do this here?
	int x_max = std::round(*max_element(x_extrema.begin(), x_extrema.end()));
	int x_min = std::round(*min_element(x_extrema.begin(), x_extrema.end()));
	int y_max = std::round(*max_element(y_extrema.begin(), y_extrema.end()));
	int y_min = std::round(*min_element(y_extrema.begin(), y_extrema.end()));

	// iterate over each point in the bounding box
	for (int x = x_min; x < x_max; ++x) {
		for (int y = y_min; y < y_max; ++y) {
			Vec2i point(x, y);
			//image.set(x, y, WHITE);
			Vec3f u_v_1 = barycentric(point, v0, v1, v2);
			//printf("point:    (%i,%i)\n", x, y);
			//printf("bary : %f %f %f\n", u_v_1.x, u_v_1.y, u_v_1.z);
			//std::cout << u_v_1.x << u_v_1.y << u_v_1.z << std::endl;
			//std::cout << u_v_1.x + u_v_1.y + u_v_1.z << std::endl;
			if (!(u_v_1.x < 0 || u_v_1.y < 0 || u_v_1.z < 0)) {
				//printf("bary : %f %f %f\n", u_v_1.x, u_v_1.y, u_v_1.z);
				image.set(x, y, color);
			}
		}
	}
}

void draw_object(Model &m, TGAImage &image) {
	// for each of the object's faces (each triangle)
	for (int i = 0; i < m.nfaces(); ++i) {
		TGAColor color(rand()%255, rand()%255, rand()%255, 255);

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

		draw_triangle(v0_n, v1_n, v2_n, image, color);

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
	//draw_triangle(Vec3f(10,10,0), Vec3f(100,30,0), Vec3f(190,160,0), image, RED);
	image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	image.write_tga_file("output.tga");
	return 0;
}