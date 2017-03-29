#ifndef __VERTEX_H__
#define __VERTEX_H__

#include "geometry.h"

class Vertex {
private:
	Vec3f position; // x,y,z
	Vec3f normal; // x,y,z
	Vec2f texture_coordinates; // u,v
public:	
	Vertex(Vec3f p, Vec3f n, Vec2f t);
	const Vec3f &get_position() const;
	const Vec3f &get_normal() const;
	const Vec2f &get_texture_coordinates() const;
};

#endif
