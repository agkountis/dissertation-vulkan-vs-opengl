#include "mesh_utilities.h"

bool GenerateQuadrilateral(Mesh* mesh, const Vec3f& dimensions, VertexWinding vertexWinding) noexcept
{

	Vec3f halfDimensions{ dimensions / 2.0f };

	//front
	mesh->AddVertex(
			Vertex{
					Vec3f{ -halfDimensions.x, -halfDimensions.y, halfDimensions.z },
					Vec3f{ 0.0f, 0.0f, 1.0f },
					Vec3f{ 1.0f, 0.0f, 0.0f },
					Vec3f{ 1.0f },
					Vec2f{ 0.0f, 1.0f }
			});

	mesh->AddVertex(
			Vertex{
					Vec3f{ -halfDimensions.x, halfDimensions.y, halfDimensions.z },
					Vec3f{ 0.0f, 0.0f, 1.0f },
					Vec3f{ 1.0f, 0.0f, 0.0f },
					Vec3f{ 1.0f },
					Vec2f{ 0.0f, 0.0f }
			});

	mesh->AddVertex(
			Vertex{
					Vec3f{ halfDimensions.x, -halfDimensions.y, halfDimensions.z },
					Vec3f{ 0.0f, 0.0f, 1.0f },
					Vec3f{ 1.0f, 0.0f, 0.0f },
					Vec3f{ 1.0f },
					Vec2f{ 1.0f, 1.0f }
			});

	mesh->AddVertex(
			Vertex{
					Vec3f{ halfDimensions.x, halfDimensions.y, halfDimensions.z },
					Vec3f{ 0.0f, 0.0f, 1.0f },
					Vec3f{ 1.0f, 0.0f, 0.0f },
					Vec3f{ 1.0f },
					Vec2f{ 1.0f, 0.0f }
			});

	//right
	mesh->AddVertex(
			Vertex{
					Vec3f{ halfDimensions.x, -halfDimensions.y, halfDimensions.z },
					Vec3f{ 1.0f, 0.0f, 0.0f },
					Vec3f{ 0.0f, 0.0f, 1.0f },
					Vec3f{ 1.0f },
					Vec2f{ 0.0f, 1.0f }
			});

	mesh->AddVertex(
			Vertex{
					Vec3f{ halfDimensions.x, halfDimensions.y, halfDimensions.z },
					Vec3f{ 1.0f, 0.0f, 0.0f },
					Vec3f{ 0.0f, 0.0f, 1.0f },
					Vec3f{ 1.0f },
					Vec2f{ 0.0f, 0.0f }
			});

	mesh->AddVertex(
			Vertex{
					Vec3f{ halfDimensions.x, -halfDimensions.y, -halfDimensions.z },
					Vec3f{ 1.0f, 0.0f, 0.0f },
					Vec3f{ 0.0f, 0.0f, 1.0f },
					Vec3f{ 1.0f },
					Vec2f{ 1.0f, 1.0f }
			});

	mesh->AddVertex(
			Vertex{
					Vec3f{ halfDimensions.x, halfDimensions.y, -halfDimensions.z },
					Vec3f{ 1.0f, 0.0f, 0.0f },
					Vec3f{ 0.0f, 0.0f, 1.0f },
					Vec3f{ 1.0f },
					Vec2f{ 1.0f, 0.0f }
			});

	//left
	mesh->AddVertex(
			Vertex{
					Vec3f{ -halfDimensions.x, -halfDimensions.y, -halfDimensions.z },
					Vec3f{ -1.0f, 0.0f, 0.0f },
					Vec3f{ 0.0f, 0.0f, -1.0f },
					Vec3f{ 1.0f },
					Vec2f{ 0.0f, 1.0f }
			});

	mesh->AddVertex(
			Vertex{
					Vec3f{ -halfDimensions.x, halfDimensions.y, -halfDimensions.z },
					Vec3f{ -1.0f, 0.0f, 0.0f },
					Vec3f{ 0.0f, 0.0f, -1.0f },
					Vec3f{ 1.0f },
					Vec2f{ 0.0f, 0.0f }
			});

	mesh->AddVertex(
			Vertex{
					Vec3f{ -halfDimensions.x, -halfDimensions.y, halfDimensions.z },
					Vec3f{ -1.0f, 0.0f, 0.0f },
					Vec3f{ 0.0f, 0.0f, -1.0f },
					Vec3f{ 1.0f },
					Vec2f{ 1.0f, 1.0f }
			});

	mesh->AddVertex(
			Vertex{
					Vec3f{ -halfDimensions.x, halfDimensions.y, halfDimensions.z },
					Vec3f{ -1.0f, 0.0f, 0.0f },
					Vec3f{ 0.0f, 0.0f, -1.0f },
					Vec3f{ 1.0f },
					Vec2f{ 1.0f, 0.0f }
			});

	//back
	mesh->AddVertex(
			Vertex{
					Vec3f{ halfDimensions.x, -halfDimensions.y, -halfDimensions.z },
					Vec3f{ 0.0f, 0.0f, -1.0f },
					Vec3f{ -1.0f, 0.0f, 0.0f },
					Vec3f{ 1.0f },
					Vec2f{ 0.0f, 1.0f }
			});

	mesh->AddVertex(
			Vertex{
					Vec3f{ halfDimensions.x, halfDimensions.y, -halfDimensions.z },
					Vec3f{ 0.0f, 0.0f, -1.0f },
					Vec3f{ -1.0f, 0.0f, 0.0f },
					Vec3f{ 1.0f },
					Vec2f{ 0.0f, 0.0f }
			});

	mesh->AddVertex(
			Vertex{
					Vec3f{ -halfDimensions.x, -halfDimensions.y, -halfDimensions.z },
					Vec3f{ 0.0f, 0.0f, -1.0f },
					Vec3f{ -1.0f, 0.0f, 0.0f },
					Vec3f{ 1.0f },
					Vec2f{ 1.0f, 1.0f }
			});

	mesh->AddVertex(
			Vertex{
					Vec3f{ -halfDimensions.x, halfDimensions.y, -halfDimensions.z },
					Vec3f{ 0.0f, 0.0f, -1.0f },
					Vec3f{ -1.0f, 0.0f, 0.0f },
					Vec3f{ 1.0f },
					Vec2f{ 1.0f, 0.0f }
			});

	//top
	mesh->AddVertex(
			Vertex{
					Vec3f{ -halfDimensions.x, halfDimensions.y, halfDimensions.z },
					Vec3f{ 0.0f, 1.0f, 0.0f },
					Vec3f{ 1.0f, 0.0f, 0.0f },
					Vec3f{ 1.0f },
					Vec2f{ 0.0f, 1.0f }
			});

	mesh->AddVertex(
			Vertex{
					Vec3f{ -halfDimensions.x, halfDimensions.y, -halfDimensions.z },
					Vec3f{ 0.0f, 1.0f, 0.0f },
					Vec3f{ 1.0f, 0.0f, 0.0f },
					Vec3f{ 1.0f },
					Vec2f{ 0.0f, 0.0f }
			});

	mesh->AddVertex(
			Vertex{
					Vec3f{ halfDimensions.x, halfDimensions.y, halfDimensions.z },
					Vec3f{ 0.0f, 1.0f, 0.0f },
					Vec3f{ 1.0f, 0.0f, 0.0f },
					Vec3f{ 1.0f },
					Vec2f{ 1.0f, 1.0f }
			});

	mesh->AddVertex(
			Vertex{
					Vec3f{ halfDimensions.x, halfDimensions.y, -halfDimensions.z },
					Vec3f{ 0.0f, 1.0f, 0.0f },
					Vec3f{ 1.0f, 0.0f, 0.0f },
					Vec3f{ 1.0f },
					Vec2f{ 1.0f, 0.0f }
			});

	//bottom
	mesh->AddVertex(
			Vertex{
					Vec3f{ halfDimensions.x, -halfDimensions.y, halfDimensions.z },
					Vec3f{ 0.0f, -1.0f, 0.0f },
					Vec3f{ -1.0f, 0.0f, 0.0f },
					Vec3f{ 1.0f },
					Vec2f{ 0.0f, 1.0f }
			});

	mesh->AddVertex(
			Vertex{
					Vec3f{ halfDimensions.x, -halfDimensions.y, -halfDimensions.z },
					Vec3f{ 0.0f, -1.0f, 0.0f },
					Vec3f{ -1.0f, 0.0f, 0.0f },
					Vec3f{ 1.0f },
					Vec2f{ 0.0f, 0.0f }
			});

	mesh->AddVertex(
			Vertex{
					Vec3f{ -halfDimensions.x, -halfDimensions.y, halfDimensions.z },
					Vec3f{ 0.0f, -1.0f, 0.0f },
					Vec3f{ -1.0f, 0.0f, 0.0f },
					Vec3f{ 1.0f },
					Vec2f{ 1.0f, 1.0f }
			});

	mesh->AddVertex(
			Vertex{
					Vec3f{ -halfDimensions.x, -halfDimensions.y, -halfDimensions.z },
					Vec3f{ 0.0f, -1.0f, 0.0f },
					Vec3f{ -1.0f, 0.0f, 0.0f },
					Vec3f{ 1.0f },
					Vec2f{ 1.0f, 0.0f }
			});

	mesh->GenerateIndices(vertexWinding);

	return mesh->CreateBuffers();
}

bool GenerateCube(Mesh* mesh, float size, VertexWinding vertexWinding) noexcept
{
	return GenerateQuadrilateral(mesh, Vec3f{ size, size, size }, vertexWinding);
}

