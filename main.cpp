#include "tgaimage.h"
#include "model.h"
#include <cmath>

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255,   0,   0, 255);
const TGAColor green = TGAColor(  0, 255,   0, 255);
const int HEIGHT = 800;
const int WIDTH = 800;

void line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color) {

  bool isTransposed = false;
  // if the rise is greater than the run, transpose
  if (std::abs(y1 - y0) > std::abs(x1 - x0)) {
    std::swap(x0, y0);
    std::swap(x1, y1);
    isTransposed = true;
    printf("transposed!\n");
  }

  // swap
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
    // a problem!
    // the offset can only ever deliver 1 y of error per x
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
}

// not sure which function this bug is in, but some lines are breaking as theta=3pi/4 from positive y
void draw_wireframe(Model &m, TGAImage &image, TGAColor color) {
  // iterate over faces
  for (int i = 0; i < m.nfaces(); ++i) {
  // for (int i = 0; i < 1; ++i) {
    // a face is a vector of ints, copy it
    std::vector<int> face = m.face(i);
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
}

int main(int argc, char* argv[]) {
  TGAImage image(WIDTH, HEIGHT, TGAImage::RGB);

  Model model("./african_head.obj");
  draw_wireframe(model, image, white);
  image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
  image.write_tga_file("output.tga");
  return 0;
}