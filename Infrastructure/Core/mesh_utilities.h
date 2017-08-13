#ifndef MESH_UTILITIES_H_
#define MESH_UTILITIES_H_

#include "mesh.h"

bool GenerateQuadrilateral(Mesh* mesh,
                           const Vec3f& dimensions,
                           VertexWinding vertexWinding = VertexWinding::COUNTERCLOCKWISE) noexcept;

bool GenerateCube(Mesh* mesh,
                  float size,
                  VertexWinding vertexWinding = VertexWinding::COUNTERCLOCKWISE) noexcept;

#endif //MESH_UTILITIES_H_
