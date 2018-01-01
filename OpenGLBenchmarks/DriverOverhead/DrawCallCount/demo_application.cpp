#include "gl_shader.h"
#include "demo_application.h"
#include "resource_manager.h"
#include "gl_program_pipeline.h"
#include "gl_mesh.h"
#include "mesh_utilities.h"
#include <glm/gtc/matrix_transform.inl>
#include "imgui_impl_glfw_gl3.h"
#include "imgui.h"
#include "gl_texture_sampler.h"

static ResourceManager mngr;
static GLProgramPipeline pipeline;
static GLMesh m;
static GLTextureSampler sampler;
static const GLTexture* diff{ nullptr };
static const GLTexture* spec{ nullptr };
static const GLTexture* norm{ nullptr };

DemoApplication::DemoApplication(const ApplicationSettings& settings) noexcept
	: GLApplication{ settings }
{
}

DemoApplication::~DemoApplication()
{
	ImGui_ImplGlfwGL3_Shutdown();
}

bool DemoApplication::Initialize() noexcept
{
	if (!GLApplication::Initialize()) {
		return false;
	}

	GLTextureSamplerCreateInfo samplerCreateInfo{};
	samplerCreateInfo.minFilter = GL_LINEAR;
	samplerCreateInfo.magFilter = GL_LINEAR;
	samplerCreateInfo.wrapS = GL_REPEAT;
	samplerCreateInfo.wrapT = GL_REPEAT;
	samplerCreateInfo.wrapR = GL_REPEAT;
	samplerCreateInfo.borderColor = {1.0, 1.0, 1.0, 1.0};

	if (!sampler.Create(samplerCreateInfo)) {
		return false;
	}

	const auto vert = mngr.Get<GLShader>("sdr/default.vert.spv", VERTEX);
	const auto frag = mngr.Get<GLShader>("sdr/default.frag.spv", FRAGMENT);

	diff = mngr.Get<GLTexture>("../../../Assets/opengl.jpg");
	spec = mngr.Get<GLTexture>("../../../Assets/opengl_spec.png");
	norm = mngr.Get<GLTexture>("../../../Assets/opengl_norm.png");

	pipeline.AddShader(vert);
	pipeline.AddShader(frag);
	if (!pipeline.Create()) {
		return false;
	}

	glViewport(0, 0, 1920, 1080);

	GenerateCube(&m, 1.0);

	ImGui_ImplGlfwGL3_Init(GetWindow(), true);

	return true;
}

void DemoApplication::Update() noexcept
{
}

void DemoApplication::Draw() noexcept
{
	const GLfloat clearColor[]{0.0f, 0.0f, 0.0f, 0.0f};
	const auto depthClearValue{ 1.0f };
	glClearBufferfv(GL_COLOR, 0, clearColor);
	glClearBufferfv(GL_DEPTH, 0, &depthClearValue);

	pipeline.Bind();

	const auto aspect = 1920.0f / 1080.0f;

	const auto proj = glm::perspective(glm::radians(60.0f), aspect, 0.1f, 1000.0f);
	auto model = glm::translate(Mat4f{}, Vec3f{ 0.0f, 0.0f, -5.0f });
	model = glm::rotate(model, glm::radians(GetTimer().GetMsec() / 30.0f), Vec3f{ 1.0f, 1.0f, 0.0f });

	pipeline.SetMatrix4f("projection", proj, VERTEX);
	pipeline.SetMatrix4f("model", model, VERTEX);
	pipeline.SetMatrix4f("view", Mat4f{1.0f}, VERTEX);

	pipeline.SetTexture("diffuse", diff, sampler, FRAGMENT);
	pipeline.SetTexture("specular", spec, sampler, FRAGMENT);
	pipeline.SetTexture("normal", norm, sampler, FRAGMENT);

	m.Draw();
	assert(glGetError() == GL_NO_ERROR);

	ImGui_ImplGlfwGL3_NewFrame();

	ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiCond_FirstUseEver);
	bool show_test_window = true;
	ImGui::ShowTestWindow(&show_test_window);

	ImGui::Render();
}

void DemoApplication::OnResize(const Vec2i& size) noexcept
{
}
