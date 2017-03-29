#ifndef __FACE_H__
#define __FACE_H__

#include <vector>
#include "geometry.h"
#include "model.h"
#include "vertex.h"

class Model;

class Face {
private:
	Vec3f normal;
	std::vector<Vertex> vertices; // x,y,z
public:	
	Face(Model &m, int face_index);
	Vec3f get_normal();
	const std::vector<Vertex> &get_vertices() const;
};

#endif
