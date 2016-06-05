#include <vector>
#include <cmath>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"

// r g b a
const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255,   0,   0, 255);
const TGAColor green = TGAColor(  0, 255,   0, 255);
Model *model = NULL;
const int width = 800;
const int height = 800;

void line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color) {
  bool steep = false;
  // ensure that the line is not steep, because we're drawing one pixel on the x at a time
  // steep lines would have lower resolution in this case
  if (std::abs(x0-x1) < std::abs(y0-y1)) {
    std::swap(x0, y0);
    std::swap(x1, y1);
    steep = true;
  }
  if (x0 > x1) { // maybe flip horizontally, so x0 is the leftmost point
    std::swap(x0, x1);
    std::swap(y0, y1);
  }
  int dx = x1 - x0;
  int dy = y1 - y0;
  float derror = std::abs(dy) * 2;
  float error = 0;
  int y = y0;
  for (int x = x0; x <= x1; x++) {
    // first point is (x0,y0),
    // then (x0+1, whichever y value most closely hits the slope)
    if (steep) {
      image.set(y, x, color);
    } else {
      image.set(x, y, color);
    }
    error += derror; // each x val, add the height different * 2
    // if there's more cumulative height different than width distance,
    // bump 1 by a pixel
    if (error > dx) {
      y += (y1 > y0 ? 1 : -1);
      error -= dx * 2;
    }
  }
}

void triangle(Vec2i a, Vec2i b, Vec2i c, TGAImage &image, TGAColor color) {
  // find the topmost, leftmost corner (tl)
  // starting at the highest y value, for the length of (tl -> l) or (tl -> r), whichever is shortest
  //   go down a unit
  //   if both lines
  //   rasterize towards the next leftmost point (after tl) (l)
  //   rasterize between the next rightmost point (r)
  //   draw a line between these coordinates
  //
  // determine if it's on the right or left

// ... actually, it would probably be easier to find the top-left most point and its opposite side,
// then draw lines from the point with the lower theta to the one with the greater one

  // find (tl)
  // find (v1)
  // boundary = v1
  // while boundary != v2
  //   draw line from tl -> boundary
  //   increment theta
  //   boundary = the point on line segment (v1, v2) that is theta away from v1
  // wait this is literally raycasting



  /// or maybe i should split it between two right triangles?
  line(a.x, a.y, b.x, b.y, image, color);
  line(b.x, b.y, c.x, c.y, image, color);
  line(c.x, c.y, a.x, a.y, image, color);

}

int main(int argc, char** argv) {
	TGAImage image(width, height, TGAImage::RGB);
/*
  model = new Model("african_head.obj");

  // for each face
  for (int i = 0; i < model->nfaces(); i++) {
    std::vector<int> face = model->face(i);
    // for each line
    for (int j = 0; j < 3; j++ ) {
      // get the face's first vertex
      Vec3f v0 = model->vert(face[j]);
      // get the next vertex ( 1 -> 2 -> 3 -> 1 )
      Vec3f v1 = model->vert(face[(j + 1) % 3]);
      int x0 = (v0.x + 1.0) * width / 2.0; // extract first vertex's x coord, convert from a number between -1 and 1
      int y0 = (v0.y + 1.0) * width / 2.0;
      int x1 = (v1.x + 1.0) * width / 2.0;
      int y1 = (v1.y + 1.0) * width / 2.0;
      line(x0, y0, x1, y1, image, white);
    }
  }
*/

  // draw three triangles
  Vec2i t0[3] = {Vec2i(10, 70), Vec2i(50, 160), Vec2i(70, 80)};
  Vec2i t1[3] = {Vec2i(180, 50), Vec2i(150, 1), Vec2i(70, 180)};
  Vec2i t2[3] = {Vec2i(180, 150), Vec2i(120, 160), Vec2i(130, 180)};
  triangle(t0[0], t0[1], t0[2], image, red);
  triangle(t1[0], t1[1], t1[2], image, white);
  triangle(t2[0], t2[1], t2[2], image, green);

	image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	image.write_tga_file("output.tga");
	return 0;
}

