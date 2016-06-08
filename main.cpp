#include "tgaimage.h"
#include "model.h"
#include <cmath>
#include <iostream>

const TGAColor WHITE = TGAColor(255, 255, 255, 255);
const TGAColor RED   = TGAColor(255,   0,   0, 255);
const TGAColor GREEN = TGAColor(  0, 255,   0, 255);
const int HEIGHT = 800;
const int WIDTH = 800;
enum DRAW_MODE {WIRE, FILL};

// returns a vector of points on the heap
// this is a memory leak if you don't deal with the deallocate the returned vector :X
std::vector<Vec3i*> line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color) {

  std::vector<Vec3i*> points;
  bool isTransposed = false;
  // if the rise is greater than the run, transpose
  // we do this before potentially swapping points so that we don't end up un-swapping them
  if (std::abs(y1 - y0) > std::abs(x1 - x0)) {
    std::swap(x0, y0);
    std::swap(x1, y1);
    isTransposed = true;
    printf("transposed!\n");
  }

  // ensure that (x0, y0) is the leftmost point (closest to zero on the positive side)
  if (std::abs(x0) > std::abs(x1)) {
    std::swap(x0, x1);
    std::swap(y0, y1);
    printf("swapped!\n");
  }

  int dy = y1 - y0;
  int dx = x1 - x0;
  float slope = (float)dy/(float)dx; // we're preserving the fraction here
  printf("%f\n", slope);
  float error = 0.0;

  // iterate over each x value on the line
  int y = y0;
  printf("%d -> %d\n", x0, x1);
  // draw the points, first point is (x0, y0)
  for (int x = x0; x <= x1; x++) {
    if (isTransposed) {
      // push the point to our point array
      points.push_back(new Vec3i(y, x, 0));
      // draw the point
      image.set(y, x, color);
      // print coords
      printf("%d, %d\n", y, x);
    } else {
      points.push_back(new Vec3i(x, y, 0));
      image.set(x, y, color);
      printf("%d, %d\n", x, y);
    }
    // add overflow
    // a problem? the offset can only ever deliver 1 y of error per x
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

void fill_triangle(std::vector<Vec3i*> &points, TGAImage &image) {

  std::vector<Vec3i*> horizontal_points;
  int previous_y;
  for (std::vector<Vec3i*>::iterator i = points.begin(); i != points.end(); ++i) {

    // first point of the face
    if (i == points.begin()) {
      std::cout << "continue!" << std::endl;
      horizontal_points.push_back(*i);
      previous_y = (*i)->y;
      continue;
    }

    std::cout << (*i)->x << ", " << (*i)->y << std::endl;
    std::cout << (*i)->y << previous_y << std::endl;
    // if the current point is on the same y, push it
    if ((*i)->y == previous_y) {
      std::cout << "push!" << std::endl;
      horizontal_points.push_back(*i);

    // if it's not, draw the line, clear the vector, then push the point
    } else {
      std::cout << "draw!" << std::endl;
      int x0 = horizontal_points.front()->x;
      int y0 = horizontal_points.front()->y;
      int x1 = horizontal_points.back()->x;
      int y1 = horizontal_points.back()->y;
      std::cout << "we're drawing: (" << x0 << ", " << y0 <<"), (" << x1 << ", " << y1 << ")" << std::endl;
      line(x0, y0, x1, y1, image, RED);
      horizontal_points.clear();
      horizontal_points.push_back(*i);
      previous_y = (*i)->y;
    }
  }
}

// this is inefficient, non-edge lines are each drawn twice
void draw_object(Model &m, TGAImage &image, TGAColor color, bool z_culling, DRAW_MODE mode ) {
  // for each face
  for (int i = 0; i < m.nfaces(); ++i) {

    // a face is a vector of ints, copy it
    std::vector<int> face = m.face(i);
    //if (mode == FILL) {
    // we'll store three lines of points at a time
    // this is going to allocate some pointers to memory, so we'll have to clear it later
    std::vector<Vec3i*> points_face;

    // for each line
    for (int j = 0; j < 3; ++j) {
      Vec3f v0 = m.vert(face[j]);
      Vec3f v1 = m.vert(face[(j+1)%3]);
      // add 1 to each point to make all numbers positive, then scale by dimension
      int x0 = (v0.x+1.0)*WIDTH/2;
      int y0 = (v0.y+1.0)*HEIGHT/2;
      int x1 = (v1.x+1.0)*WIDTH/2;
      int y1 = (v1.y+1.0)*HEIGHT/2;
      printf("(%d, %d), (%d, %d)\n", x0, y0, x1, y1);
      // get and (unnecessarily) draw the triangle's outer lines
      // this should merge the return value of lines() with points' current value
      std::vector<Vec3i*> points_line = line(x0, y0, x1, y1, image, color);
      points_face.insert(points_face.begin(), points_line.begin(), points_line.end());
    }

    // we have all of the outer points of the triangle
    // sort the points by y value (with a lambda)
    std::sort(
      points_face.begin(),
      points_face.end(),
      [] (Vec3i *a, Vec3i *b) { return a->y < b->y; }
    );
    fill_triangle(points_face, image);

    // deallocate memory
    for (std::vector<Vec3i*>::iterator it = points_face.begin() ; it != points_face.end(); ++it) {
      delete (*it);
    }
    points_face.clear();
    printf("clear!\n");
    /*
      } else {
        for (int j = 0; j < 3; ++j) {
          Vec3f v0 = m.vert(face[j]);
          Vec3f v1 = m.vert(face[(j+1)%3]);
          // add 1 to each point to make all numbers positive, then scale by dimension
          int x0 = (v0.x+1.0)*WIDTH/2;
          int y0 = (v0.y+1.0)*HEIGHT/2;
          int x1 = (v1.x+1.0)*WIDTH/2;
          int y1 = (v1.y+1.0)*HEIGHT/2;
          printf("(%d, %d), (%d, %d)\n", x0, y0, x1, y1);
          line(x0, y0, x1, y1, image, color);
        }
      }
    */
  } // foreach face
}

int main(int argc, char* argv[]) {
  TGAImage image(WIDTH, HEIGHT, TGAImage::RGB);
  Model model("./african_head.obj");

  // next step ... filling triangles or z culling
  draw_object(model, image, GREEN, false, FILL);
  image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
  image.write_tga_file("output.tga");
  return 0;
}