#include "face.h"
#include "util.h"

Face::Face(Model &m, int face_index) {
  // get the face's vertex position indices
  auto face_v = m.face_v(face_index); // position vertex indices (x, y, z)
  auto face_vt = m.face_vt(face_index); // 3 2d texture vertices

  // for each point
  for (auto i = 0; i < 3; ++i) {
    // grab its position values (xyz), store them in this.vertices
    this->vertices.push_back( Vec3f(
      m.vert(face_v[i]).x, // x
      m.vert(face_v[i]).y, // y
      m.vert(face_v[i]).z  // z
    ));
    this->texture_coordinates.push_back( Vec2f(
      m.vert_t(face_vt[i]).x, // u
      m.vert_t(face_vt[i]).y  // v
    ));
  }
}

// TODO: is this using right hand rule?
Vec3f Face::get_normal() {
  // derive two vectors from our three vertices
  // the vectors are arbitrary
  auto a = this->vertices[0];
  auto b = this->vertices[1];
  auto c = this->vertices[2];
  Vec3f ab(b.x-a.x, b.y-a.y, b.z-a.z);
  Vec3f ac(c.x-a.x, c.y-a.y, c.z-a.z);
  return cross_product(ab, ac);
}

const std::vector<Vec3f> &Face::get_vertices() const {
  return this->vertices;
}

const std::vector<Vec2f> &Face::get_texture_coordinates() const {
  return this->texture_coordinates;
}