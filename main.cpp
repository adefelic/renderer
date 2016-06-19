/**
 * a simple renderer
 * - adam defelice
 *
 * this needs to be refactored :/
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

const int HEIGHT = 1024;
const int WIDTH  = 1024;
const int DEPTH  = 1024; // i know this is crazy ... should've just made a scale variable
enum DRAW_MODE {WIRE, FILL};

// returns a vector of points on the heap
// this is a memory leak if you don't deallocate the returned vector :X
// a fact: this is the only function that calls image.set()
std::vector<Vec3i*> line(Vec3i *v0, Vec3i *v1, TGAImage &image, TGAColor color, std::map<Vec2i, int> &z_values, bool draw) {

  // TODO: make this calculate Z, so that we can ignore them later

  int x0 = v0->x;
  int y0 = v0->y;
  int z0 = v0->z;
  int x1 = v1->x;
  int y1 = v1->y;
  int z1 = v1->z;

  // this vector will hold the points that make up the line. we'll be returning it.
  std::vector<Vec3i*> points;

  /* DETERMINE Y VALUES FOR EACH X */
  // heck, looks like i'm going to duplicate this all at the bottom for (x, z)
  bool yTransposed = false;
  // if the rise is greater than the run, transpose
  // we do this before potentially swapping points so that we don't end up un-swapping them
  if (std::abs(y1 - y0) > std::abs(x1 - x0)) {
    std::swap(x0, y0);
    std::swap(x1, y1);
    yTransposed = true;
    //printf("x + y transposed!\n");
  }

  // ensure that (x0, y0) is the leftmost point (closest to zero on the positive side)
  if (std::abs(x0) > std::abs(x1)) {
    std::swap(x0, x1);
    std::swap(y0, y1);
    //printf("swapped!\n");
  }

  int dy = y1 - y0;
  int dx = x1 - x0;
  float slope = (float)dy/(float)dx; // we're preserving the fraction here
  //printf("%f\n", slope);
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

    /*
        // break early if we've already drawn a point with a higher z value
        Vec2i z_key(x_to_draw, y_to_draw);
        if (z_values.count(z_key) != 0) {
          if (z_values.at(z_key) < z){
            // code
          }
        } else {
          // found
        }
    */
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
void draw_object(Model &m, TGAImage &image, TGAColor color, bool z_culling, DRAW_MODE mode ) {

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

      /* NORMALIZE POINTS */
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
      //printf("Normalized outline vertices: (%d, %d, %d), (%d, %d, %d)\n", v0_i.x, v0_i.y, v0_i.z, v1_i.x, v1_i.y, v1_i.z);

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
    draw_triangle(points_face, image, TGAColor(rand()%255, rand()%255, rand()%255, 255), z_values);

    // deallocate memory
    for (std::vector<Vec3i*>::iterator it = points_face.begin(); it != points_face.end(); ++it) {
      delete (*it);
    }
    points_face.clear();
  } // foreach face
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
  draw_object(model, image, RED, false, mode);
  image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
  image.write_tga_file("output.tga");
  return 0;
}