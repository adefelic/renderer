#include "tgaimage.h"
#include "model.h"
#include <cmath>

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255,   0,   0, 255);
const TGAColor green = TGAColor(  0, 255,   0, 255);
const int HEIGHT = 800;
const int WIDTH = 800;

void line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color) {

  // ensure that (x0, y0) is the leftmost point
  if (x0 > x1) {
    std::swap(x0, x1);
    std::swap(y0, y1);
  }

  // ensure that we're working in the zeroth octant, clockwise from the positive x axis
  // we assume we're working in quadrant 1
  bool isTransposed = false;
  // if the rise is greater than the run, transpose
  if ((y1 - y0) > (x1 - x0)) {
    std::swap(x0, y0);
    std::swap(x1, y1);
    isTransposed = true;
  }

  // get the slope
  // it'll always be < 1
  int dy = y1 - y0;
  int dx = x1 - x0;
  float derror = (float)dy/(float)dx; // we're preserving the fraction here
  float error = 0.0;

  // iterate over each x value on the line
  int y = y0;
  for (int x = x0; x <= x1; x++) {
    // draw the point, first point is (x0, y0)
    if (isTransposed) {
      image.set(y, x, color);
      printf("%d, %d\n", y, x);
    } else {
      image.set(x, y, color);
      printf("%d, %d\n", x, y);
    }
    // add overflow
    error += derror;
    if (error >= 1.0) {
      error--;
      y++;
    } else if (error <= -1.0) {
      error++;
      y--;
    }
  }
  printf("\n");
}

void draw_wireframe(Model &m, TGAImage &image, TGAColor color) {
  // iterate over faces
  for (int i = 0; i < m.nfaces(); ++i) {
    // a face is a vector of ints, copy it
    std::vector<int> face = m.face(i);
    for (int j = 0; j < 3; ++j) {
      Vec3f v0 = m.vert(face[j]);
      Vec3f v1 = m.vert(face[(j+1)%3]);
      int x0 = (v0.x+1.0)*WIDTH/2.0;
      int y0 = (v0.y+1.0)*HEIGHT/2.0;
      int x1 = (v1.x+1.0)*WIDTH/2.0;
      int y1 = (v1.y+1.0)*HEIGHT/2.0;
      line(x0, y0, x1, y1, image, color);
    }
  }
}

int main(int argc, char* argv[]) {
  TGAImage image(WIDTH, HEIGHT, TGAImage::RGB);

  Model model("./african_head.obj");
  draw_wireframe(model, image, white);
  //  (x0, y0)(x1, y1)
  //line(20, 13, 40, 80, image, green);
  //line(80, 40, 13, 20, image, red);
  image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
  image.write_tga_file("output.tga");
  return 0;
}