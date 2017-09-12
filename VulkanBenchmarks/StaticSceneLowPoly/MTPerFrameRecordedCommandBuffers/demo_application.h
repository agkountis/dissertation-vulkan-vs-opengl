#ifndef VULKAN_SINGLE_THREADED_APPLICATION_H_
#define VULKAN_SINGLE_THREADED_APPLICATION_H_

#include "vulkan_application.h"
#include "demo_entity.h"
#include "demo_scene.h"
#include <memory>
#include <vulkan_infrastructure_context.h>
#include "thread_pool.h"

class DemoApplication : public VulkanApplication {
private:
	DemoScene m_DemoScene;

	ThreadPool m_ThreadPool;

	//TODO: create resources per thread
	//TODO: create command buffers per thread.

	void EnableFeatures() noexcept override;

	bool BuildCommandBuffers() noexcept override;

public:
	explicit DemoApplication(const ApplicationSettings& settings);

	bool Initialize() noexcept override;

	void Update() noexcept override;

	void Draw() noexcept override;

	void OnResize(const Vec2i& size) noexcept override;
};

#endif // VULKAN_SINGLE_THREADED_APPLICATION_H_
