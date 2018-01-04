#include <logger.h>
#include <glm/gtc/matrix_transform.hpp>
#include <random>
#include <mutex>
#include <algorithm>
#include "demo_scene.h"
#include "imgui.h"
#include <functional>
#include <assimp/cimport.h>
#include <assimp/postprocess.h>
#include "gl_infrastructure_context.h"
#include "imgui_impl_glfw_gl3.h"
#include "gl_application.h"
#include "gl_texture.h"

static const GLfloat clearColor[]{ 1.0f, 0.0f, 0.0f, 0.0f };
static const auto depthClearValue{ 1.0f };

// Private functions -------------------------------------------------

//Model loading --------------------------------------
static Vec3f AssVector(const aiVector3D vec) noexcept
{
	return Vec3f{ vec.x, vec.y, vec.z };
}

static Quatf AssQuat(const aiQuaternion q) noexcept
{
	return Quatf{ q.w, q.x, q.y, q.z };
}

void DemoScene::LoadMeshes(const aiScene* scene) noexcept
{
	for (auto i = 0; i < scene->mNumMeshes; ++i) {
		auto mesh = new GLMesh;

		auto* aiMesh{ scene->mMeshes[i] };

		if (aiMesh->HasPositions()) {
			std::vector<Vertex> vertices;
			vertices.resize(aiMesh->mNumVertices);

			m_SceneVertexCount += aiMesh->mNumVertices;

			for (auto j = 0; j < vertices.size(); ++j) {
				vertices[j].position = Vec3f{ aiMesh->mVertices[j].x, aiMesh->mVertices[j].y, aiMesh->mVertices[j].z };
				vertices[j].normal = Vec3f{ aiMesh->mNormals[j].x, aiMesh->mNormals[j].y, aiMesh->mNormals[j].z };
				vertices[j].tangent = Vec3f{ aiMesh->mTangents[j].x, aiMesh->mTangents[j].y, aiMesh->mTangents[j].z };
				vertices[j].texcoord = Vec2f{ aiMesh->mTextureCoords[0][j].x, aiMesh->mTextureCoords[0][j].y };
			}

			mesh->AddVertices(vertices);
		}
		else {
			ERROR_LOG("Mesh has no vertices!");
			delete mesh;
			return;
		}

		if (aiMesh->HasFaces()) {
			std::vector<unsigned int> indices;

			for (auto k = 0; k < aiMesh->mNumFaces; ++k) {
				const auto& face = aiMesh->mFaces[k];
				assert(face.mNumIndices == 3);
				indices.push_back(face.mIndices[0]);
				indices.push_back(face.mIndices[1]);
				indices.push_back(face.mIndices[2]);
			}

			mesh->AddIndices(indices);
		}

		mesh->CreateBuffers();

		mesh->SetMaterialIndex(aiMesh->mMaterialIndex);

		m_Meshes.push_back(mesh);
	}
}

void DemoScene::LoadMaterials(const aiScene* scene) noexcept
{
	DemoMaterial material;
	material.diffuse = Vec4f{ 1.0f };
	material.specular = Vec4f{ 1.0f };

	const auto GetFileName = [](std::string path) -> std::string
	{
		auto sepUnix = '/';

		auto sepWindows = '\\';

		auto n = path.rfind(sepUnix);

		if (n == std::string::npos) {
			n = path.rfind(sepWindows);
		}

		if (n != std::string::npos) {
			return path.substr(n + 1);
		}

		return "";
	};

	for (auto i = 0; i < scene->mNumMaterials; ++i) {
		const auto aiMaterial = scene->mMaterials[i];

		aiString path;
		aiGetMaterialTexture(aiMaterial, aiTextureType_DIFFUSE, 0, &path);

		material.textures[TEX_DIFFUSE] = G_ResourceManager.Get<GLTexture>("../../../Assets/" + GetFileName(path.data));

		aiMaterial->GetTexture(aiTextureType_SPECULAR, 0, &path);

		material.textures[TEX_SPECULAR] = G_ResourceManager.Get<GLTexture>("../../../Assets/" + GetFileName(path.data));

		aiMaterial->GetTexture(aiTextureType_NORMALS, 0, &path);

		material.textures[TEX_NORMAL] = G_ResourceManager.Get<GLTexture>("../../../Assets/" + GetFileName(path.data));

		m_Materials.push_back(material);
	}
}

DemoEntity* DemoScene::LoadNode(const aiNode* aiNode) noexcept
{
	const std::string name = aiNode->mName.data;

	auto entity = new DemoEntity;
	entity->SetName(name);

	if (aiNode->mNumMeshes > 0) {
		const auto mesh = m_Meshes[aiNode->mMeshes[0]];

		entity->SetMesh(mesh);
		entity->SetMaterial(&m_Materials[mesh->GetMaterialIndex()]);
	}

	aiVector3D aiPosition;
	aiQuaternion aiOrientation;
	aiVector3D aiScaling;

	aiNode->mTransformation.Decompose(aiScaling, aiOrientation, aiPosition);

	entity->SetPosition(AssVector(aiPosition));
	entity->SetOrientation(AssQuat(aiOrientation));
	entity->SetScale(AssVector(aiScaling));


	// recursion for all the children
	for (int i = 0; i < aiNode->mNumChildren; ++i) {
		Entity* child{ LoadNode(aiNode->mChildren[i]) };

		entity->AddChild(child);
	}

	return entity;
}

std::unique_ptr<DemoEntity> DemoScene::LoadModel(const std::string& fileName) noexcept
{
	auto result = std::make_unique<DemoEntity>();

	const auto scn = aiImportFile(fileName.c_str(),
	                              aiProcess_GenSmoothNormals |
	                              aiProcess_FixInfacingNormals |
	                              aiProcess_Triangulate |
	                              aiProcess_CalcTangentSpace | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices |
	                              aiProcess_FixInfacingNormals | aiProcess_SortByPType);

	if (!scn) {
		ERROR_LOG("Failed to load scene: " + fileName);
		return nullptr;
	}

	if (scn->HasMeshes()) {
		LoadMeshes(scn);
	}

	if (scn->HasMaterials()) {
		LoadMaterials(scn);
	}

	const auto rootNode = scn->mRootNode;

	if (rootNode) {
		for (auto i = 0; i < rootNode->mNumChildren; ++i) {
			const auto child = rootNode->mChildren[i];

			const auto entity = LoadNode(child);

			result->AddChild(entity);
		}
	}
	else {
		ERROR_LOG("The model has no root node.");
		return nullptr;
	}

	aiReleaseImport(scn);

	return std::move(result);
}

// -------------------------------------------------

// -------------------------------------------------------------------
DemoScene::~DemoScene()
{
	for (auto mesh : m_Meshes) {
		delete mesh;
	}
	m_Meshes.clear();

	glDeleteVertexArrays(1, &m_FullscreenVA);

	ImGui_ImplGlfwGL3_Shutdown();
}

bool DemoScene::Initialize() noexcept
{
	const auto& window = G_Application.GetWindow();

	m_Entities.push_back(LoadModel("../../../Assets/scene.fbx"));

	for (auto& entity : m_Entities) {
		entity->Update(0.0f);
	}

	GLTextureSamplerCreateInfo samplerCreateInfo{};
	samplerCreateInfo.minFilter = GL_LINEAR;
	samplerCreateInfo.magFilter = GL_LINEAR;
	samplerCreateInfo.wrapS = GL_REPEAT;
	samplerCreateInfo.wrapT = GL_REPEAT;
	samplerCreateInfo.wrapR = GL_REPEAT;
	samplerCreateInfo.borderColor = { 1.0, 1.0, 1.0, 1.0 };

	if (!m_TextureSampler.Create(samplerCreateInfo)) {
		return false;
	}

	samplerCreateInfo.minFilter = GL_NEAREST;
	samplerCreateInfo.magFilter = GL_NEAREST;
	samplerCreateInfo.wrapS = GL_CLAMP_TO_EDGE;
	samplerCreateInfo.wrapT = GL_CLAMP_TO_EDGE;
	samplerCreateInfo.wrapR = GL_CLAMP_TO_EDGE;

	if (!m_AttachmentSampler.Create(samplerCreateInfo)) {
		return false;
	}

	GLRenderTargetAttachmentCreateInfo floatAttachment{};
	floatAttachment.size = window.GetSize();
	floatAttachment.internalFormat = GL_RGBA16F;

	GLRenderTargetAttachmentCreateInfo colorAttachment{};
	colorAttachment.size = window.GetSize();
	colorAttachment.internalFormat = GL_RGBA8;

	GLRenderTargetAttachmentCreateInfo depthAttachment{};
	depthAttachment.size = window.GetSize();
	depthAttachment.internalFormat = GL_DEPTH_COMPONENT32F;

	const std::vector<GLRenderTargetAttachmentCreateInfo> attachments{
		floatAttachment, //Positions
		floatAttachment, //Normals
		colorAttachment, //Albedo
		colorAttachment, //Specular
		depthAttachment  //Depth
	};

	if (!m_GBuffer.Create(attachments)) {
		ERROR_LOG("Failed to create G-Buffer");
		return false;
	}

	m_LightsUbo.Create();

	//Initialize pipelines;
	auto vert = G_ResourceManager.Get<GLShader>("sdr/deferred.vert.spv", VERTEX);
	auto frag = G_ResourceManager.Get<GLShader>("sdr/deferred.frag.spv", FRAGMENT);

	m_DeferredPipeline.AddShader(vert);
	m_DeferredPipeline.AddShader(frag);
	m_DeferredPipeline.SetRenderTarget(&m_GBuffer);

	if (!m_DeferredPipeline.Create()) {
		return false;
	}

	vert = G_ResourceManager.Get<GLShader>("sdr/display.vert.spv", VERTEX);
	frag = G_ResourceManager.Get<GLShader>("sdr/display.frag.spv", FRAGMENT);

	m_DisplayPipeline.AddShader(vert);
	m_DisplayPipeline.AddShader(frag);

	if (!m_DisplayPipeline.Create()) {
		return false;
	}

	m_DisplayPipeline.Bind();
	m_DisplayPipeline.SetUniformBuffer("Lights", m_LightsUbo, FRAGMENT);

	glCreateVertexArrays(1, &m_FullscreenVA);

	m_LightsUbo.Map(GL_WRITE_ONLY);

	assert(glGetError() == GL_NO_ERROR);

	return ImGui_ImplGlfwGL3_Init(window, true);
}

void DemoScene::Update(i64 msec, f64 dt) noexcept
{
	for (auto& entity : m_Entities) {
		entity->Update(dt);
	}

	m_DeferredPipeline.Bind();

	const auto& size = G_Application.GetWindow().GetSize();
	const auto aspect = static_cast<f32>(size.x) / static_cast<f32>(size.y);
	m_DeferredPipeline.SetMatrix4f("projection", glm::perspective(glm::radians(60.0f), aspect, 0.1f, 2000.0f), VERTEX);

	const auto radius = 20.0f;
	const Vec3f eye{ sin(msec / 3000.0f) * radius, 3.0f, cos(msec / 3000.0f) * radius };
	m_DeferredPipeline.SetMatrix4f("view", glm::lookAt(eye, Vec3f{ 0.0, 6.0f, 0.0f }, Vec3f{ 0.0f, 1.0f, 0.0f }), VERTEX);

	m_Lights.positions[0] = Vec4f{ sin(msec / 1000.0f) * 20.0f, 10.0f, cos(msec / 1000.0f) * 20.0f - 10.0f, 0.0f };
	m_Lights.colors[0] = Vec4f{ 1.0f, 1.0f, 1.0f, 1.0 };
	m_Lights.radi[0] = Vec4f{ 80.0f };

	m_Lights.positions[1] = Vec4f{ cos(msec / 500.0f) * 10.0f, 5.0f, sin(msec / 500.0f) * 10.0f - 10.0f, 0.0f };
	m_Lights.colors[1] = Vec4f{ 0.0f, 0.0f, 1.0f, 1.0 };
	m_Lights.radi[1] = Vec4f{ 80.0f };

	m_Lights.positions[2] = Vec4f{ cos(msec / 3000.0f) * 10.0f, 20.0f, sin(msec / 3000.0f) * 10.0f - 10.0f, 0.0f };
	m_Lights.colors[2] = Vec4f{ 0.0f, 1.0f, 0.0f, 1.0 };
	m_Lights.radi[2] = Vec4f{ 120.0f };

	m_Lights.positions[3] = Vec4f{ cos(msec / 200.0f) * 5.0f, 10.0f, sin(msec / 200.0f) * 5.0f - 10.0f, 0.0f };
	m_Lights.colors[3] = Vec4f{ 1.0f, 0.0f, 0.0f, 1.0 };
	m_Lights.radi[3] = Vec4f{ 120.0f };

	m_Lights.eyePos = eye;

	m_LightsUbo.Fill(&m_Lights);
}

void DemoScene::DrawEntity(DemoEntity* entity) noexcept
{
	m_DeferredPipeline.SetMatrix4f("model", entity->GetXform(), VERTEX);
	const auto mesh = entity->GetMesh();
	if (mesh) {
		const auto material = entity->GetMaterial();

		if (!material) {
			LOG("HAVE MESH> NO MATERIAL");
			return;
		}

		m_DeferredPipeline.SetTexture("diffuseSampler", material->textures[TEX_DIFFUSE], m_TextureSampler, FRAGMENT);
		m_DeferredPipeline.SetTexture("specularSampler", material->textures[TEX_SPECULAR], m_TextureSampler, FRAGMENT);
		m_DeferredPipeline.SetTexture("normalSampler", material->textures[TEX_NORMAL], m_TextureSampler, FRAGMENT);

		mesh->Draw();
	}

	for (auto child : entity->GetChildren()) {
		const auto c = static_cast<DemoEntity*>(child);
		DrawEntity(c);
	}
}

void DemoScene::DrawUi() const noexcept
{
	auto& application = G_Application;

	ImGui_ImplGlfwGL3_NewFrame();

	const auto glVersion = glGetString(GL_VERSION);
	const auto glRenderer = glGetString(GL_RENDERER);

	GLint profileMask;
	glGetIntegerv(GL_CONTEXT_PROFILE_MASK, &profileMask);

	const char* profileType;
	if (profileMask & GL_CONTEXT_CORE_PROFILE_BIT) {
		profileType = "Core";
	}
	else {
		profileType = "Compatibility";
	}

	const auto vendor = glGetString(GL_VENDOR);

	if (!application.benchmarkComplete) {
		// 1. Show a simple window.
		// Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets appears in a window automatically called "Debug"
		ImGui::Begin("Metrics");

		ImGui::Text("OpenGL version: %s", glVersion);
		ImGui::Text("OpenGL profile: %s", profileType);
		ImGui::Text("OpenGL renderer: %s", glRenderer);
		ImGui::Text("Vendor: %s", vendor);

		ImGui::NewLine();
		ImGui::Separator();
		ImGui::NewLine();

		char buff[60];

		ImGui::Text("Average value real time graphs (refresh per sec)");
		snprintf(buff, 60, "FPS\nAvg: %f\nMin: %f\nMax: %f", application.averageFps, application.minFps,
		         application.maxFps);
		ImGui::PlotLines(buff, application.fpsAverages.data(), application.fpsAverages.size(), 0, "",
		                 0.0, application.maxFps, ImVec2(0, 80));

		snprintf(buff, 60, "Frame time (ms)\nAvg: %f ms\nMin: %f ms\nMax: %f ms", application.wholeFrameAverage,
		         application.minWholeFrame,
		         application.maxWholeFrame);
		ImGui::PlotLines(buff, application.wholeFrameAverages.data(), application.wholeFrameAverages.size(), 0, "",
		                 0.0, application.maxWholeFrame, ImVec2(0, 80));

		snprintf(buff, 60, "CPU time (ms)\nAvg: %f ms\nMin: %f ms\nMax: %f ms", application.cpuTimeAverage,
		         application.minCpuTime,
		         application.maxCpuTime);
		ImGui::PlotLines(buff, application.cpuTimeAverages.data(), application.cpuTimeAverages.size(), 0, "",
		                 0.0, application.maxCpuTime, ImVec2(0, 80));

		snprintf(buff, 60, "GPU time (ms)\nAvg: %f ms\nMin: %f ms\nMax: %f ms", application.gpuTimeAverage,
		         application.minGpuTime,
		         application.maxGpuTime);
		ImGui::PlotLines(buff, application.gpuTimeAverages.data(), application.gpuTimeAverages.size(), 0, "",
		                 0.0, application.maxGpuTime, ImVec2(0, 80));

		ImGui::NewLine();
		ImGui::Text("Total Vertex Count: %d", m_Entities.size() * 24);
		ImGui::Text("Draw calls: %u", m_Entities.size());
		ImGui::Text("Running time: %f s", application.GetTimer().GetSec());
		ImGui::Text("Frame count: %lld", application.frameCount);
		ImGui::End();
	}
	else {
		ImGui::Begin("Benchmark Results", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

		ImGui::Text("OpenGL version: %s", glVersion);
		ImGui::Text("OpenGL profile: %s", profileType);
		ImGui::Text("OpenGL renderer: %s", glRenderer);
		ImGui::Text("Vendor: %s", vendor);

		ImGui::NewLine();
		ImGui::Separator();
		ImGui::NewLine();

		if (ImGui::Combo("Active attachment", &m_CurrentAttachment, m_AttachmentComboItems.data(),
			m_AttachmentComboItems.size())) {
			LOG("Value changed to: " + std::to_string(m_CurrentAttachment));
		}

		ImGui::NewLine();
		ImGui::Separator();
		ImGui::NewLine();

		char buff[60];

		snprintf(buff, 60, "FPS\nAvg: %f\nMin: %f\nMax: %f", application.averageFps, application.minFps,
		         application.maxFps);
		ImGui::PlotLines(buff, application.totalFpsSamples.data(), application.totalFpsSamples.size(), 0, "",
		                 0.0, application.maxFps, ImVec2(1750, 100));

		ImGui::NewLine();

		snprintf(buff, 60, "Frame time (ms)\nAvg: %f ms\nMin: %f ms\nMax: %f ms", application.avgTotalFrameTime,
		         application.minTotalFrameTime,
		         application.maxTotalFrameTime);
		ImGui::PlotLines(buff, application.totalFrameTimeSamples.data(), application.totalFrameTimeSamples.size(), 0, "",
		                 application.minTotalFrameTime, application.maxTotalFrameTime, ImVec2(1750, 100));

		ImGui::NewLine();

		snprintf(buff, 60, "CPU time (ms)\nAvg: %f ms\nMin: %f ms\nMax: %f ms", application.avgTotalCpuTime,
		         application.minTotalCpuTime,
		         application.maxTotalCpuTime);
		ImGui::PlotLines(buff, application.totalCpuTimeSamples.data(), application.totalCpuTimeSamples.size(), 0, "",
		                 application.minTotalCpuTime, application.maxTotalCpuTime, ImVec2(1750, 100));

		ImGui::NewLine();

		snprintf(buff, 60, "GPU time (ms)\nAvg: %f ms\nMin: %f ms\nMax: %f ms", application.avgTotalGpuTime,
		         application.minTotalGpuTime,
		         application.maxTotalGpuTime);
		ImGui::PlotLines(buff, application.totalGpuTimeSamples.data(), application.totalGpuTimeSamples.size(), 0, "",
		                 application.minTotalGpuTime, application.maxTotalGpuTime, ImVec2(1750, 100));

		ImGui::NewLine();
		ImGui::Separator();
		ImGui::NewLine();

		ImGui::Text("Total Vertex Count: %d", m_Entities.size() * 24);
		ImGui::Text("Draw calls: %u", m_Entities.size());
		ImGui::Text("Total Frames: %lld", application.frameCount);
		ImGui::Text("Total duration: %f s", application.totalAppDuration);
		ImGui::Text("Average FPS: %f", 1000.0f / application.avgTotalFrameTime);
		ImGui::Text("Average frame time: %f ms", application.avgTotalFrameTime);
		ImGui::Text("Average CPU time: %f ms", application.avgTotalCpuTime);
		ImGui::Text("Average GPU time: %f ms", application.avgTotalGpuTime);
		ImGui::Text("99th percentile (lower is better): %f ms", application.percentile99th);

		ImGui::NewLine();

		if (ImGui::Button("Save to CSV")) {
			LOG("Saving to CSV");
			application.SaveToCsv("GL_DeferredRendering_Metrics");
		}

		if (ImGui::Button("Exit Application")) {
			LOG("Terminating application.");
			application.SetTermination(true);
		}

		ImGui::End();
	}

	ImGui::Render();
}

void DemoScene::Draw() noexcept
{
	glClearBufferfv(GL_COLOR, 0, clearColor);
	glClearBufferfv(GL_DEPTH, 0, &depthClearValue);

	for (const auto& entity : m_Entities) {
		DrawEntity(entity.get());
	}

	m_DisplayPipeline.Bind();

	glClearBufferfv(GL_COLOR, 0, clearColor);
	glClearBufferfv(GL_DEPTH, 0, &depthClearValue);

	glBindVertexArray(m_FullscreenVA);

	glDrawArrays(GL_TRIANGLES, 0, 3);

	glBindVertexArray(0);

	DrawUi();
}
