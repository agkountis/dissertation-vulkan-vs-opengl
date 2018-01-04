#ifndef DISSERTATION_DEMO_SCENE_H
#define DISSERTATION_DEMO_SCENE_H

#include <memory>
#include "demo_entity.h"
#include "assimp/scene.h"
#include "gl_render_target.h"
#include "gl_texture_sampler.h"
#include "gl_buffer.h"
#include "gl_program_pipeline.h"

struct MatricesUbo {
	Mat4f view;
	Mat4f projection;
};

constexpr int lightCount{ 4 };

struct Lights {
	Vec4f positions[lightCount];
	Vec4f colors[lightCount];
	Vec4f radi[lightCount];
    Vec3f eyePos;
};

class DemoScene final {
private:
	std::vector<std::unique_ptr<DemoEntity>> m_Entities;

	GLProgramPipeline m_DeferredPipeline;

	GLProgramPipeline m_DisplayPipeline;

	GLTextureSampler m_TextureSampler;

	GLTextureSampler m_AttachmentSampler;

	GLRenderTarget m_GBuffer;

	Lights m_Lights;

	GLBuffer<Lights> m_LightsUbo;

	GLuint m_FullscreenVA{ 0 };

	//UI -------------------------------
    mutable i32 m_CurrentAttachment{ 0 };
	std::array<const char*, 6> m_AttachmentComboItems{ "Lit", "Position", "Normals", "Albedo", "Specular", "Depth" };

	i64 m_SceneVertexCount{ 0 };
	// ---------------------------

	// Model Loading--------------------
	std::vector<DemoMaterial> m_Materials;

	std::vector<GLMesh*> m_Meshes;

	void LoadMeshes(const aiScene* scene) noexcept;

	void LoadMaterials(const aiScene* scene) noexcept;

	DemoEntity* LoadNode(const aiNode* aiNode) noexcept;

	std::unique_ptr<DemoEntity> LoadModel(const std::string& fileName) noexcept;
	//----------------------------------

	void DrawEntity(DemoEntity* entity) noexcept;

	void DrawUi() const noexcept;

public:
	~DemoScene();

	bool Initialize() noexcept;

	void Update(i64 msec, f64 dt) noexcept;

	void Draw() noexcept;
};

#endif //DISSERTATION_DEMO_SCENE_H
