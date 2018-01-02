#include "GL/glew.h"
#include "gl_application.h"
#include "logger.h"
#include "gl_infrastructure_context.h"

GLApplication::GLApplication(const ApplicationSettings& settings) noexcept
	: Application{ settings }
{
}

bool GLApplication::Initialize() noexcept
{
	if (!Application::Initialize()) {
		return false;
	}

	const auto& settings = GetSettings();

	if (!m_Window.Create(settings.name,
	                     settings.windowResolution,
	                     settings.windowPosition,
	                     this)) {
		return false;
	}

	if (settings.vsync) {
		glfwSwapInterval(1);
	}

	glewExperimental = true;
	const auto error = glewInit();

	if (error != GLEW_OK) {
		ERROR_LOG(glewGetErrorString(error));
		return false;
	}

	if (!GLEW_ARB_gl_spirv) {
		ERROR_LOG("GL_ARB_gl_spirv extension not supported. Aborting!");
		return false;
	}

	if (!GLEW_ARB_separate_shader_objects) {
		ERROR_LOG("GL_ARB_separate_shader_objects extension not supported. Aborting!");
		return false;
	}

	GLInfrastructureContext::Register(&m_ResourceManager, this);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glDisable(GL_MULTISAMPLE);

	glCreateQueries(GL_TIME_ELAPSED, 1, &m_Query);

	return true;
}

i32 GLApplication::Run() noexcept
{
	GetTimer().Start();
	while (!glfwWindowShouldClose(m_Window) && !ShouldTerminate()) {
		glfwPollEvents();

		static auto prev = 0.0;

		Update();

		glBeginQuery(GL_TIME_ELAPSED, m_Query);

		PreDraw();
		Draw();
		PostDraw();

		glEndQuery(GL_TIME_ELAPSED);

		const auto now = GetTimer().GetSec();
		wholeFrameTime = (now - prev) * 1000.0;

		GLuint64 gpuResult{ 0 };
		glGetQueryObjectui64v(m_Query, GL_QUERY_RESULT, &gpuResult);

		gpuTime = gpuResult * 1e-6;
		cpuTime = wholeFrameTime - gpuTime;

		if (cpuTime < 0.0f) {
			cpuTime = 0.0f;
		}

		if (!benchmarkComplete) {
			// Calculate moving averages
			static float accum{ 0 };
			accum += wholeFrameTime;
			wholeFrameTimeSamples.push_back(wholeFrameTime);
			cpuTimeSamples.push_back(cpuTime);
			gpuTimeSamples.push_back(gpuTime);

			totalFrameTimeSamples.push_back(wholeFrameTime);
			totalCpuTimeSamples.push_back(cpuTime);
			totalGpuTimeSamples.push_back(gpuTime);

			if (accum > 1000.0f || calculateResults) {
				const auto size = wholeFrameTimeSamples.size();
				auto wholeFrameTimeSum{ 0.0 };
				auto cpuTimeSum{ 0.0 };
				auto gpuTimeSum{ 0.0 };

				for (auto i = 0u; i < size; ++i) {
					wholeFrameTimeSum += wholeFrameTimeSamples[i];
					cpuTimeSum += cpuTimeSamples[i];
					gpuTimeSum += gpuTimeSamples[i];
				}
				wholeFrameAverage = wholeFrameTimeSum / static_cast<f32>(size);
				averageFps = 1000.0f / wholeFrameAverage;

				totalFpsSamples.push_back(averageFps);

				cpuTimeAverage = cpuTimeSum / static_cast<f32>(size);
				gpuTimeAverage = gpuTimeSum / static_cast<f32>(size);


				minWholeFrame = std::min(minWholeFrame, wholeFrameTime);
				maxWholeFrame = std::max(wholeFrameTime, maxWholeFrame);

				minFps = std::min(minFps, averageFps);
				maxFps = std::max(maxFps, averageFps);

				minCpuTime = std::min(minCpuTime, cpuTimeAverage);
				maxCpuTime = std::max(maxCpuTime, cpuTimeAverage);

				minGpuTime = std::min(minGpuTime, gpuTimeAverage);
				maxGpuTime = std::max(maxGpuTime, gpuTimeAverage);

				std::rotate(fpsAverages.begin(), fpsAverages.begin() + 1, fpsAverages.end());
				std::rotate(wholeFrameAverages.begin(), wholeFrameAverages.begin() + 1, wholeFrameAverages.end());
				std::rotate(cpuTimeAverages.begin(), cpuTimeAverages.begin() + 1, cpuTimeAverages.end());
				std::rotate(gpuTimeAverages.begin(), gpuTimeAverages.begin() + 1, gpuTimeAverages.end());

				fpsAverages.back() = averageFps;
				wholeFrameAverages.back() = wholeFrameAverage;
				cpuTimeAverages.back() = cpuTimeAverage;
				gpuTimeAverages.back() = gpuTimeAverage;

				wholeFrameTimeSamples.clear();
				cpuTimeSamples.clear();
				gpuTimeSamples.clear();

				accum = 0;
			}

			prev = now;
			++frameCount;
		}

		if (calculateResults) {

			maxTotalFrameTime = *std::max_element(totalFrameTimeSamples.cbegin(), totalFrameTimeSamples.cend());
			minTotalFrameTime = *std::min_element(totalFrameTimeSamples.cbegin(), totalFrameTimeSamples.cend());

			maxTotalCpuTime = *std::max_element(totalCpuTimeSamples.cbegin(), totalCpuTimeSamples.cend());
			minTotalCpuTime = *std::min_element(totalCpuTimeSamples.cbegin(), totalCpuTimeSamples.cend());

			maxTotalGpuTime = *std::max_element(totalGpuTimeSamples.cbegin(), totalGpuTimeSamples.cend());
			minTotalGpuTime = *std::min_element(totalGpuTimeSamples.cbegin(), totalGpuTimeSamples.cend());

			for (auto i = 0u; i < totalFrameTimeSamples.size(); ++i) {
				avgTotalFrameTime += totalFrameTimeSamples[i];
				avgTotalCpuTime += totalCpuTimeSamples[i];
				avgTotalGpuTime += totalGpuTimeSamples[i];
			}

			avgTotalFrameTime /= static_cast<f32>(totalFrameTimeSamples.size());
			avgTotalCpuTime /= static_cast<f32>(totalCpuTimeSamples.size());
			avgTotalGpuTime /= static_cast<f32>(totalGpuTimeSamples.size());

			auto frameTimeVecCopy = totalFrameTimeSamples;

			std::sort(frameTimeVecCopy.begin(), frameTimeVecCopy.end(), [](auto a, auto b) { return a < b; });

			auto index = 0.99f * frameTimeVecCopy.size();

			auto integral = 0.0f;
			const auto fractional = modff(index, &integral);

			if (fractional == 0.0f) {
				percentile99th = frameTimeVecCopy[static_cast<i32>(index)];
			} else {
				index = std::round(index);
				percentile99th = frameTimeVecCopy[index];
			}

			benchmarkComplete = true;
			calculateResults = false;
		}

		if (!frameRateTermination) {
			if (GetDuration() > 0.0f && now > GetDuration() && !benchmarkComplete) {
				totalAppDuration = now;
				calculateResults = true;
			}
		}
		else {
			if (wholeFrameAverage > 33.3 && !benchmarkComplete) {
				totalAppDuration = now;
				calculateResults = true;
			}
		}

		glfwSwapBuffers(m_Window);
	}

	glDeleteQueries(1, &m_Query);

	return 0;
}

void GLApplication::PreDraw() noexcept
{
}

void GLApplication::PostDraw() noexcept
{
}

GLWindow& GLApplication::GetWindow() noexcept
{
	return m_Window;
}

void GLApplication::Reshape(const Vec2ui& size) noexcept
{
}
