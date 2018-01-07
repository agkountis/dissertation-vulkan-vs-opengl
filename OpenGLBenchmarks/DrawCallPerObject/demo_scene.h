#ifndef DISSERTATION_DEMO_SCENE_H
#define DISSERTATION_DEMO_SCENE_H

#include <memory>
#include "demo_entity.h"
#include "gl_texture_sampler.h"
#include "gl_program_pipeline.h"

class DemoScene final {
private:
	std::vector<std::unique_ptr<DemoEntity>> m_Entities;

	GLMesh m_CubeMesh;

	GLProgramPipeline m_Pipeline;

	GLTextureSampler m_TextureSampler;

	bool SpawnEntity() noexcept;

	void DrawUi() const noexcept;

public:
	~DemoScene();

	bool Initialize() noexcept;

	void Update(i64 msec, f64 dt) noexcept;

	void Draw() noexcept;

	void SaveToCsv(const std::string& fname) const;
};

#endif //DISSERTATION_DEMO_SCENE_H
