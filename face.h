#ifndef __FACE_H__
#define __FACE_H__

#include <vector>
#include "geometry.h"
#include "model.h"

class Model;

class Face {
private:
	Vec3f normal;
	std::vector<Vec3f> vertices;
	std::vector<Vec2f> texture_coordinates;
public:	
	Face(Model &m, int face_index);
	Vec3f get_normal();
	const std::vector<Vec3f> &get_vertices() const;
	const std::vector<Vec2f> &get_texture_coordinates() const;
};

#endif
