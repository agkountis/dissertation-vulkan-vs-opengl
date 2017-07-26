#ifndef MESH_H_
#define MESH_H_

#include <vector>
#include "vertex.h"
#include "resource.h"

class Mesh : public Resource {
private:
	std::vector<Vertex> m_Vertices;

	std::vector<ui32> m_Indices;

public:
	void AddVertex(const Vertex& vertex) noexcept;

	void AddIndex(ui32 index) noexcept;

	void AddVertices(const std::vector<Vertex>& vertices) noexcept;

	void AddIndices(const std::vector<ui32>& indices) noexcept;

	const std::vector<Vertex>& GetVertices() const noexcept;

	const std::vector<ui32>& GetIndices() const noexcept;

	Vertex* GetVertexDataPtr() noexcept;

	ui32* GetIndexDataPtr() noexcept;

	bool Load(const std::string& fileName) noexcept override;
};

#endif //MESH_H_
