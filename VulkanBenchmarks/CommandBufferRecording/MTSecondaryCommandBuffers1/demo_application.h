#ifndef VULKAN_SINGLE_THREADED_APPLICATION_H_
#define VULKAN_SINGLE_THREADED_APPLICATION_H_

#include "vulkan_application.h"
#include "demo_entity.h"
#include "demo_scene.h"
#include "thread_pool.h"

class DemoApplication : public VulkanApplication {
private:
	DemoScene m_DemoScene;

	ThreadPool m_ThreadPool;

	struct ThreadData {
		VkCommandPool commandPool{ VK_NULL_HANDLE };
		std::vector<VkCommandBuffer> secondaryCommandBuffers;
	};

	std::vector<ThreadData> m_PerThreadData;

	VkFence m_RenderFence{ VK_NULL_HANDLE };

	void EnableFeatures() noexcept override;

	bool BuildCommandBuffers() noexcept override;

public:
	explicit DemoApplication(const ApplicationSettings& settings);

	~DemoApplication();

	bool Initialize() noexcept override;

	void Update() noexcept override;

	void Draw() noexcept override;

	void OnResize(const Vec2i& size) noexcept override;

	const DemoScene& GetScene() const noexcept { return m_DemoScene; }
};

#endif // VULKAN_SINGLE_THREADED_APPLICATION_H_