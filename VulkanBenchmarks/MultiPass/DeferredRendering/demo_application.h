#ifndef DISSERTATION_DEMO_APPLICATION_H_
#define DISSERTATION_DEMO_APPLICATION_H_

#include "vulkan_application.h"
#include "demo_entity.h"
#include "demo_scene.h"

class DemoApplication final : public VulkanApplication {
private:
	DemoScene m_DemoScene;

	VkCommandBuffer m_DeferredCommandBuffer{ VK_NULL_HANDLE };

	VulkanSemaphore m_DeferredSemaphore;

	void EnableFeatures() noexcept override;

	bool BuildCommandBuffers() noexcept override;

	bool BuildDeferredPassCommandBuffer(); // G-Buffer

	bool BuildDisplayCommandBuffer(); // Fullscreen quad

public:
	explicit DemoApplication(const ApplicationSettings& settings);

	~DemoApplication() noexcept;

	bool Initialize() noexcept override;

	void Update() noexcept override;

	void Draw() noexcept override;

	void OnResize(const Vec2i& size) noexcept override;
};

#endif // DISSERTATION_DEMO_APPLICATION_H_
