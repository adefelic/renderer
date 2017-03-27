#include "vertex.h"

Vertex::Vertex(Vec3f p, Vec3f n, Vec2f t) : position(p), normal(n), texture_coordinates(t){}

const Vec3f &Vertex::get_position() const {
	return position;
};
const Vec3f &Vertex::get_normal() const {
	return normal;
};
const Vec2f &Vertex::get_texture_coordinates() const {
	return texture_coordinates;
};
