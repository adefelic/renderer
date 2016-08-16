#ifndef __MODEL_H__
#define __MODEL_H__

#include <vector>
#include "geometry.h"

class Model {
private:
	std::vector<Vec3f> verts_;  // vertices (x, y, z)
	std::vector<Vec3f> verts_t_;  // vertices (u, w, 0)
	std::vector<Vec3f> verts_n_;  // vertices (?, ?, ?)
	std::vector<std::vector<int> > vfaces_;
	std::vector<std::vector<int> > vtfaces_;
	std::vector<std::vector<int> > vnfaces_;
public:
	Model(const char *filename);
	~Model();
	int nverts();
	int nfaces();
	Vec3f vert(int i);
	Vec3f vert_t(int i);
	Vec3f vert_n(int i);
	std::vector<int> face_v(int i);
	std::vector<int> face_vt(int i);
	std::vector<int> face_vn(int i);
};

#endif


