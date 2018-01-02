#include <logger.h>
#include "gl_infrastructure_context.h"
#include <glm/gtc/matrix_transform.hpp>
#include <random>
#include <mesh_utilities.h>
#include <gl_shader.h>
#include <mutex>
#include "demo_scene.h"
#include <algorithm>
#include <gl_application.h>
#include "imgui.h"
#include <fstream>
#include "texture.h"
#include "gl_texture.h"
#include "gl_texture_sampler.h"

static std::mt19937 s_Rng;
static const GLfloat clearColor[]{0.0f, 0.0f, 0.0f, 0.0f};
static const auto depthClearValue{ 1.0f };

static auto RealRangeRng(const f32 begin, const f32 end)
{
	std::uniform_real_distribution<f32> distribution{ begin, end };
	return distribution(s_Rng);
}

// Private functions -------------------------------------------------
bool DemoScene::SpawnEntity() noexcept
{
	auto entity = std::make_unique<DemoEntity>(&m_CubeMesh);

	entity->SetPosition(Vec3f{
		RealRangeRng(-20.0f, 20.0f),
		RealRangeRng(-20.0f, 20.0f),
		RealRangeRng(-20.0f, 20.0f)
	});


	entity->SetMaterial(&m_Material);

	m_Entities.push_back(std::move(entity));

	// Sort entities from back to front to avoid early z optimizations.
	std::sort(m_Entities.begin(), m_Entities.end(), [](auto& a, auto& b)
	{
		return a->GetPosition().z > b->GetPosition().z;
	});

	return true;
}

void DemoScene::DrawUi() const noexcept
{
	//auto& application = G_Application;

//	ImGui_ImplGlfwVulkan_NewFrame();
//
//	if (!application.benchmarkComplete) {
//		// 1. Show a simple window.
//		// Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets appears in a window automatically called "Debug"
//		ImGui::Begin("Metrics");
//		ImGui::Text("Device name: %s", G_VulkanDevice.GetPhysicalDevice().properties.deviceName);
//
//		const auto deviceType = G_VulkanDevice.GetPhysicalDevice().properties.deviceType;
//
//		const char* type{ nullptr };
//
//		switch (deviceType) {
//		case VK_PHYSICAL_DEVICE_TYPE_OTHER:
//			type = "Other";
//			break;
//		case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
//			type = "Integrated GPU";
//			break;
//		case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
//			type = "Discrete GPU";
//			break;
//		case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
//			type = "Virtual GPU";
//			break;
//		case VK_PHYSICAL_DEVICE_TYPE_CPU:
//			type = "CPU";
//			break;
//		default: ;
//		}
//
//		ImGui::Text("Device ID: %d", G_VulkanDevice.GetPhysicalDevice().properties.deviceID);
//		ImGui::Text("Device type: %s", type);
//		ImGui::Text("Vendor: %d", G_VulkanDevice.GetPhysicalDevice().properties.vendorID);
//		ImGui::Text("Driver Version: %d", G_VulkanDevice.GetPhysicalDevice().properties.driverVersion);
//
//		ImGui::NewLine();
//		ImGui::Separator();
//		ImGui::NewLine();
//
//		char buff[60];
//
//		ImGui::Text("Average value real time graphs (refresh per sec)");
//		snprintf(buff, 60, "FPS\nAvg: %f\nMin: %f\nMax: %f", application.averageFps, application.minFps,
//		         application.maxFps);
//		ImGui::PlotLines(buff, application.fpsAverages.data(), application.fpsAverages.size(), 0, "",
//		                 0.0, application.maxFps, ImVec2(0, 80));
//
//		snprintf(buff, 60, "Frame time (ms)\nAvg: %f ms\nMin: %f ms\nMax: %f ms", application.wholeFrameAverage,
//		         application.minWholeFrame,
//		         application.maxWholeFrame);
//		ImGui::PlotLines(buff, application.wholeFrameAverages.data(), application.wholeFrameAverages.size(), 0, "",
//		                 0.0, application.maxWholeFrame, ImVec2(0, 80));
//
//		snprintf(buff, 60, "CPU time (ms)\nAvg: %f ms\nMin: %f ms\nMax: %f ms", application.cpuTimeAverage,
//		         application.minCpuTime,
//		         application.maxCpuTime);
//		ImGui::PlotLines(buff, application.cpuTimeAverages.data(), application.cpuTimeAverages.size(), 0, "",
//		                 0.0, application.maxCpuTime, ImVec2(0, 80));
//
//		snprintf(buff, 60, "GPU time (ms)\nAvg: %f ms\nMin: %f ms\nMax: %f ms", application.gpuTimeAverage,
//		         application.minGpuTime,
//		         application.maxGpuTime);
//		ImGui::PlotLines(buff, application.gpuTimeAverages.data(), application.gpuTimeAverages.size(), 0, "",
//		                 0.0, application.maxGpuTime, ImVec2(0, 80));
//
//		ImGui::NewLine();
//		ImGui::Text("Total Vertex Count: %d", m_Entities.size() * 24);
//		ImGui::Text("Draw calls: %u", m_Entities.size());
//		ImGui::Text("Running time: %f s", application.GetTimer().GetSec());
//		ImGui::Text("Frame count: %lld", application.frameCount);
//		ImGui::End();
//	}
//	else {
//		ImGui::Begin("Benchmark Results", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
//		ImGui::Text("Device name: %s", G_VulkanDevice.GetPhysicalDevice().properties.deviceName);
//
//		const auto deviceType = G_VulkanDevice.GetPhysicalDevice().properties.deviceType;
//
//		const char* type{ nullptr };
//
//		switch (deviceType) {
//		case VK_PHYSICAL_DEVICE_TYPE_OTHER:
//			type = "Other";
//			break;
//		case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
//			type = "Integrated GPU";
//			break;
//		case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
//			type = "Discrete GPU";
//			break;
//		case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
//			type = "Virtual GPU";
//			break;
//		case VK_PHYSICAL_DEVICE_TYPE_CPU:
//			type = "CPU";
//			break;
//		default: ;
//		}
//
//		ImGui::Text("Device ID: %d", G_VulkanDevice.GetPhysicalDevice().properties.deviceID);
//		ImGui::Text("Device type: %s", type);
//		ImGui::Text("Vendor: %d", G_VulkanDevice.GetPhysicalDevice().properties.vendorID);
//		ImGui::Text("Driver Version: %d", G_VulkanDevice.GetPhysicalDevice().properties.driverVersion);
//
//		ImGui::NewLine();
//		ImGui::Separator();
//		ImGui::NewLine();
//
//		char buff[60];
//
//		snprintf(buff, 60, "FPS\nAvg: %f\nMin: %f\nMax: %f", application.averageFps, application.minFps,
//		         application.maxFps);
//		ImGui::PlotLines(buff, application.totalFpsSamples.data(), application.totalFpsSamples.size(), 0, "",
//		                 0.0, application.maxFps, ImVec2(1750, 100));
//
//		ImGui::NewLine();
//
//		snprintf(buff, 60, "Frame time (ms)\nAvg: %f ms\nMin: %f ms\nMax: %f ms", application.avgTotalFrameTime,
//		         application.minTotalFrameTime,
//		         application.maxTotalFrameTime);
//		ImGui::PlotLines(buff, application.totalFrameTimeSamples.data(), application.totalFrameTimeSamples.size(), 0, "",
//		                 application.minTotalFrameTime, application.maxTotalFrameTime, ImVec2(1750, 100));
//
//		ImGui::NewLine();
//
//		snprintf(buff, 60, "CPU time (ms)\nAvg: %f ms\nMin: %f ms\nMax: %f ms", application.avgTotalCpuTime,
//		         application.minTotalCpuTime,
//		         application.maxTotalCpuTime);
//		ImGui::PlotLines(buff, application.totalCpuTimeSamples.data(), application.totalCpuTimeSamples.size(), 0, "",
//		                 application.minTotalCpuTime, application.maxTotalCpuTime, ImVec2(1750, 100));
//
//		ImGui::NewLine();
//
//		snprintf(buff, 60, "GPU time (ms)\nAvg: %f ms\nMin: %f ms\nMax: %f ms", application.avgTotalGpuTime,
//		         application.minTotalGpuTime,
//		         application.maxTotalGpuTime);
//		ImGui::PlotLines(buff, application.totalGpuTimeSamples.data(), application.totalGpuTimeSamples.size(), 0, "",
//		                 application.minTotalGpuTime, application.maxTotalGpuTime, ImVec2(1750, 100));
//
//		ImGui::NewLine();
//		ImGui::Separator();
//		ImGui::NewLine();
//
//		ImGui::Text("Total Vertex Count: %d", m_Entities.size() * 24);
//		ImGui::Text("Draw calls: %u", m_Entities.size());
//		ImGui::Text("Total Frames: %lld", application.frameCount);
//		ImGui::Text("Total duration: %f s", application.totalAppDuration);
//		ImGui::Text("Average FPS: %f", 1000.0f / application.avgTotalFrameTime);
//		ImGui::Text("Average frame time: %f ms", application.avgTotalFrameTime);
//		ImGui::Text("Average CPU time: %f ms", application.avgTotalCpuTime);
//		ImGui::Text("Average GPU time: %f ms", application.avgTotalGpuTime);
//		ImGui::Text("99th percentile (lower is better): %f ms", application.percentile99th);
//
//		ImGui::NewLine();
//
//		if (ImGui::Button("Save to CSV")) {
//			LOG("Saving to CSV");
//			SaveToCsv("DrawCallCount_Metrics");
//		}
//
//		if (ImGui::Button("Exit Application")) {
//			LOG("Terminating application.");
//			application.SetTermination(true);
//		}
//
//		ImGui::End();
//	}
//
//	ImGui_ImplGlfwVulkan_Render(commandBuffer);
}

// -------------------------------------------------------------------
DemoScene::~DemoScene()
{
	//ImGui_ImplGlfwgl3_Shutdown();
}

bool DemoScene::Initialize() noexcept
{
	using namespace std::chrono;
	const auto seed = high_resolution_clock::now().time_since_epoch().count();
	s_Rng = std::mt19937(seed);

	if (!GenerateCube(&m_CubeMesh, 1.0f)) {
		ERROR_LOG("Failed to generate cube mesh.");
		return false;
	}

	GLTextureSamplerCreateInfo samplerCreateInfo{};
	samplerCreateInfo.minFilter = GL_LINEAR;
	samplerCreateInfo.magFilter = GL_LINEAR;
	samplerCreateInfo.wrapS = GL_REPEAT;
	samplerCreateInfo.wrapT = GL_REPEAT;
	samplerCreateInfo.wrapR = GL_REPEAT;
	samplerCreateInfo.borderColor = {1.0, 1.0, 1.0, 1.0};

	if (!m_TextureSampler.Create(samplerCreateInfo)) {
		return false;
	}

	m_Material.textures[TEX_DIFFUSE] = G_ResourceManager.Get<GLTexture>("../../../Assets/opengl.jpg");

	m_Material.textures[TEX_SPECULAR] = G_ResourceManager.Get<GLTexture>("../../../Assets/opengl_spec.png");

	m_Material.textures[TEX_NORMAL] = G_ResourceManager.Get<GLTexture>("../../../Assets/opengl_norm.png");

	const auto vert = G_ResourceManager.Get<GLShader>("sdr/default.vert.spv", VERTEX);
	const auto frag = G_ResourceManager.Get<GLShader>("sdr/default.frag.spv", FRAGMENT);

	m_Pipeline.AddShader(vert);
	m_Pipeline.AddShader(frag);
	if (!m_Pipeline.Create()) {
		return false;
	}

	const auto& windowSize = G_Application.GetWindow().GetSize();
	glViewport(0, 0, windowSize.x, windowSize.y);

	m_Pipeline.Bind();

	const auto aspect = static_cast<f32>(windowSize.x) / static_cast<f32>(windowSize.y);
	const auto proj = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 200.0f);
	const auto view = glm::lookAt(Vec3f{ 0.0f, 0.0f, 60.0f }, Vec3f{}, Vec3f{ 0.0f, 1.0f, 0.0f });

	m_Pipeline.SetMatrix4f("projection", proj, VERTEX);
	m_Pipeline.SetMatrix4f("view", view, VERTEX);

	m_Pipeline.SetTexture("diffuse", m_Material.textures[TEX_DIFFUSE], m_TextureSampler, FRAGMENT);
	m_Pipeline.SetTexture("specular", m_Material.textures[TEX_SPECULAR], m_TextureSampler, FRAGMENT);
	m_Pipeline.SetTexture("normal",  m_Material.textures[TEX_NORMAL], m_TextureSampler, FRAGMENT);

	assert(glGetError() == GL_NO_ERROR);

	//TODO: Initialize Imgui here.

	return true;
}

static constexpr f64 spawnRate{ 500.0f };
static f64 entitiesToSpawn{ 0.0f };

void DemoScene::Update(i64 msec, f64 dt) noexcept
{
	//TODO: Remove. Temporary code
	entitiesToSpawn += spawnRate * dt;

	i32 e = entitiesToSpawn;

	entitiesToSpawn -= e;

	auto i = 0;
	while (i < e) {
		SpawnEntity();
		++i;
	}

	for (auto& entity : m_Entities) {
		entity->Update(dt);
	}

	assert(glGetError() == GL_NO_ERROR);

//	if (!G_Application.benchmarkComplete) {
//		entitiesToSpawn += spawnRate * dt;
//
//		i32 e = entitiesToSpawn;
//
//		entitiesToSpawn -= e;
//
//		int i = 0;
//		while (i < e) {
//			SpawnEntity();
//			++i;
//		}
//
//		for (auto& entity : m_Entities) {
//			entity->Update(dt);
//		}
//	}
}

void DemoScene::Draw() noexcept
{
	glClearBufferfv(GL_COLOR, 0, clearColor);
	glClearBufferfv(GL_DEPTH, 0, &depthClearValue);

	for (const auto& entity : m_Entities) {
		m_Pipeline.SetMatrix4f("model", entity->GetXform(), VERTEX);
		entity->Draw();
	}
}

void DemoScene::SaveToCsv(const std::string& fname) const
{
//	const auto& app = G_Application;
//	std::ofstream stream{ fname + ".csv" };
//
//	stream << "Whole Frame Time,CPU Time,GPU Time\n";
//
//	for (auto i = 0u; i < app.totalFrameTimeSamples.size(); ++i) {
//		stream << app.totalFrameTimeSamples[i] << "," << app.totalCpuTimeSamples[i] << "," << app.totalGpuTimeSamples[i] << "\n";
//	}
//
//	stream << "\nFPS,Whole Frame Time,CPU Time,GPU Time\n";
//
//	for (auto i = 0u; i < app.fpsAverages.size(); ++i) {
//		stream << app.fpsAverages[i] << "," << app.wholeFrameAverages[i] << "," << app.cpuTimeAverages[i] << "," << app.gpuTimeAverages[i] <<
//				"\n";
//	}
//
//	stream << "\nAverage FPS,Average Frame Time,Average CPU Time,Average GPU Time\n";
//	stream << 1000.0f / app.avgTotalFrameTime << "," << app.avgTotalFrameTime << "," << app.avgTotalCpuTime << "," << app.avgTotalGpuTime;
//
//	stream << "\nDraw Calls per Frame\n";
//	stream << m_Entities.size();
//
//	stream << "\n99th percentile\n";
//	stream << app.percentile99th;
//
//	stream.close();
}