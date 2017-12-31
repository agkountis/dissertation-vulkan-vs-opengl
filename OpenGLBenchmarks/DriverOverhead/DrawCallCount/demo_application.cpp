#include "gl_shader.h"
#include "demo_application.h"
#include "resource_manager.h"
#include "gl_program_pipeline.h"
#include "gl_mesh.h"
#include "mesh_utilities.h"
#include <glm/gtc/matrix_transform.inl>

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

	GLShader* vert = mngr.Get<GLShader>("sdr/default.vert.spv", VERTEX);
	GLShader* frag = mngr.Get<GLShader>("sdr/default.frag.spv", FRAGMENT);

	pipeline.AddShader(vert);
	pipeline.AddShader(frag);
	if (!pipeline.Create()) {
		return false;
	}

	glViewport(0, 0, 1920, 1080);


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

	float aspect = 1920.0f / 1080.0f;

	Mat4f proj = glm::perspective(glm::radians(60.0f), aspect, 0.1f, 1000.0f);
	Mat4f model = glm::translate(Mat4f{}, Vec3f{0.0f, 0.0f, -5.0f});
	model = glm::rotate(model, glm::radians(GetTimer().GetMsec() / 30.0f), Vec3f{1.0f, 1.0f, 0.0f});

	Mat4f MVP = proj * model;

	pipeline.SetMatrix4f("MVP", MVP, VERTEX);

	m.Draw();

	ui32 a = glGetError();
	assert(glGetError() == GL_NO_ERROR);
//	glDrawArrays(GL_TRIANGLE_STRIP, 0, 3);




}

void DemoApplication:: OnResize(const Vec2i& size) noexcept
{

}
