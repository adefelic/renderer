#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include "model.h"

/**
 * Read in a .obj file
 *
 *
 *
 *
 */
Model::Model(const char *filename) : verts_(), vfaces_() {
  std::ifstream in;

  // open the file
  in.open(filename, std::ifstream::in);
  if (in.fail()) return;

  // for each line
  std::string line;
  while (!in.eof()) {
    std::getline(in, line);
    std::istringstream iss(line.c_str());
    char trash;
    // if the line begins with "v ", store the next three values in a Vec3f in verts_
    if (!line.compare(0, 2, "v ")) {
      iss >> trash; // throw out the first character
      // store 3 whitespace-delimited strings. hooray stringstreams!
      Vec3f v;
      for (int i=0;i<3;i++) iss >> v.raw[i];
      verts_.push_back(v);
    }
    // if the line starts with a "f ", it's a face
    // faces are indices into v, vt, and vn
    // f 1106/1145/1106 1136/1182/1136 1132/1178/1132
    //     v0/ vt0/ vn0   v1/ vt1/ vn1   v2/ vt2/ vn2
    else if (!line.compare(0, 2, "f ")) {
      std::vector<int> f_v;
      std::vector<int> f_vt;
      std::vector<int> f_vn;
      int v_index, vt_index, vn_index;
      iss >> trash; // throw out the 'f'
      while (iss >> v_index >> trash >> vt_index >> trash >> vn_index) { // trash the slash
        // in wavefront obj all indices start at 1, not zero
        // push em back
        f_v.push_back(v_index-1);
        f_vt.push_back(vt_index-1);
        f_vn.push_back(vn_index-1);
      }
      vfaces_.push_back(f_v);
      vtfaces_.push_back(f_vt);
      vnfaces_.push_back(f_vn);
    }
  }
  std::cerr << "# v# " << verts_.size() << " f# "  << vfaces_.size() << std::endl;
}

Model::~Model() {
}

int Model::nverts() {
  return (int)verts_.size();
}

int Model::nfaces() {
  return (int)vfaces_.size();
}

std::vector<int> Model::face_v(int i) {
  return vfaces_[i];
}

std::vector<int> Model::face_vt(int i) {
  return vtfaces_[i];
}

std::vector<int> Model::face_vn(int i) {
  return vnfaces_[i];
}

Vec3f Model::vert(int i) {
  return verts_[i];
}

