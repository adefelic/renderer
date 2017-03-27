#include "face.h"
#include "util.h"

Face::Face(Model &m, int face_index) {
  // get the face's vertex position indices
  auto face_v = m.face_v(face_index); // position vertex indices (x, y, z)
  auto face_vn = m.face_vn(face_index); // normal vertex indices (x, y, z)
  auto face_vt = m.face_vt(face_index); // texture vertex indices (u, v, 0)

  // for each point
  for (auto i = 0; i < 3; ++i) {
    // grab its position values (xyz), store them in this.vertices
    this->vertices.push_back( Vertex(
      Vec3f(
        m.vert(face_v[i]).x, // x
        m.vert(face_v[i]).y, // y
        m.vert(face_v[i]).z  // z
      ),
      Vec3f(
        m.vert(face_vn[i]).x, // x
        m.vert(face_vn[i]).y, // y
        m.vert(face_vn[i]).z  // z
      ),
      Vec2f(
        m.vert_t(face_vt[i]).x, // u
        m.vert_t(face_vt[i]).y  // v
      )
    ));
  }
}

Vec3f Face::get_normal() {
  // derive two vectors from our three vertices
  // the vectors are arbitrary
  auto a = this->vertices[0].get_position();
  auto b = this->vertices[1].get_position();
  auto c = this->vertices[2].get_position();
  Vec3f ab(b.x-a.x, b.y-a.y, b.z-a.z);
  Vec3f ac(c.x-a.x, c.y-a.y, c.z-a.z);
  return cross_product(ab, ac);
}

const std::vector<Vertex> &Face::get_vertices() const {
  return this->vertices;
}
