#include "gl_shader.h"
#include "demo_application.h"
#include "resource_manager.h"
#include "gl_program_pipeline.h"
#include "gl_mesh.h"
#include "mesh_utilities.h"

static ResourceManager mngr;
static GLProgramPipeline pipeline;
static GLMesh m;

DemoApplication::DemoApplication(const ApplicationSettings& settings) noexcept
	: GLApplication{ settings }
{
}

bool DemoApplication::Initialize() noexcept
{
	if (!GLApplication::Initialize()) {
		return false;
	}

	GLShader* vert = mngr.Get<GLShader>("sdr/default.vert.spv", GLShaderType::VERTEX);
	GLShader* frag = mngr.Get<GLShader>("sdr/default.frag.spv", GLShaderType::FRAGMENT);

	pipeline.AddShader(vert);
	pipeline.AddShader(frag);
	if (!pipeline.Create()) {
		return false;
	}



	GenerateCube(&m, 1.0);

	return true;
}

void DemoApplication::Update() noexcept
{

}

void DemoApplication::Draw() noexcept
{
	glClearColor(1.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	pipeline.Bind();
	glDrawArrays(GL_TRIANGLES, 0, 6);

	//m.Draw();

}

void DemoApplication:: OnResize(const Vec2i& size) noexcept
{

}
