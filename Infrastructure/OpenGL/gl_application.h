#ifndef GL_APPLICATION_H_
#define GL_APPLICATION_H_
#include "application.h"
#include "gl_window.h"
#include "resource_manager.h"
#include <vector>
#include <array>

class GLApplication : public Application {
private:
	GLWindow m_Window;

	GLuint m_Query{ 0 };

	ResourceManager m_ResourceManager;

	std::vector<f32> wholeFrameTimeSamples;

	std::vector<f32> cpuTimeSamples;

	std::vector<f32> gpuTimeSamples;

	bool calculateResults = false;

public:
	std::vector<f32> totalFrameTimeSamples;
	std::vector<f32> totalCpuTimeSamples;
	std::vector<f32> totalGpuTimeSamples;
	std::vector<f32> totalFpsSamples;

	f32 wholeFrameTime{ 0.0f };

	f32 cpuTime{ 0.0f };

	f32 gpuTime{ 0.0f };

	f32 totalAppDuration{ 0.0 };

	i64 frameCount{ 0 };

	f32 maxTotalFrameTime{ std::numeric_limits<f32>::min() };
	f32 maxTotalCpuTime{ std::numeric_limits<f32>::min() };
	f32 maxTotalGpuTime{ std::numeric_limits<f32>::min() };

	f32 avgTotalFrameTime{ 0.0f };
	f32 avgTotalCpuTime{ 0.0f };
	f32 avgTotalGpuTime{ 0.0f };

	f32 minTotalFrameTime{ std::numeric_limits<f32>::max() };
	f32 minTotalCpuTime{ std::numeric_limits<f32>::max() };
	f32 minTotalGpuTime{ std::numeric_limits<f32>::max() };

	f32 averageFps{ 0.0 };
	std::array<f32, 60> fpsAverages{};
	f32 minFps{ std::numeric_limits<f32>::max() };
	f32 maxFps{ std::numeric_limits<f32>::min() };

	f32 wholeFrameAverage{ 0.0 };
	std::array<f32, 60> wholeFrameAverages{};
	f32 minWholeFrame{ std::numeric_limits<f32>::max() };
	f32 maxWholeFrame{ std::numeric_limits<f32>::min() };

	f32 cpuTimeAverage{ 0.0 };
	f32 minCpuTime{ std::numeric_limits<f32>::max() };
	f32 maxCpuTime{ std::numeric_limits<f32>::min() };
	std::array<f32, 60> cpuTimeAverages{};

	f32 gpuTimeAverage{ 0.0 };
	std::array<f32, 60> gpuTimeAverages{};
	f32 minGpuTime{ std::numeric_limits<f32>::max() };
	f32 maxGpuTime{ std::numeric_limits<f32>::min() };

	f32 percentile99th{ 0.0f };

	bool benchmarkComplete{ false };

	bool frameRateTermination{ false };

	explicit GLApplication(const ApplicationSettings& settings) noexcept;

	bool Initialize() noexcept override;

	i32 Run() noexcept override;

	void PreDraw() noexcept override;

	void PostDraw() noexcept override;

	GLWindow& GetWindow() noexcept;

	void Reshape(const Vec2ui& size) noexcept;
};

#endif //GL_APPLICATION_H_