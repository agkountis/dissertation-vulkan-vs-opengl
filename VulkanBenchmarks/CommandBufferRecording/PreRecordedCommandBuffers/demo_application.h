#ifndef VULKAN_SINGLE_THREADED_APPLICATION_H_
#define VULKAN_SINGLE_THREADED_APPLICATION_H_

#include "vulkan_application.h"
#include "demo_entity.h"
#include "demo_scene.h"

class DemoApplication final : public VulkanApplication {
private:
	DemoScene m_DemoScene;

	VkCommandBuffer m_UiCommandBuffer{ VK_NULL_HANDLE };

	VulkanSemaphore m_UiSemaphore;

	VkRenderPass m_UiRenderPass{ VK_NULL_HANDLE };

	void EnableFeatures() noexcept override;

	bool BuildCommandBuffers() noexcept override;

	void DrawUi() noexcept;

	// Override to create a new render pass for the UI that does not clear the
	// framebuffers but writes on top of them.
	bool CreateRenderPasses() noexcept override;
public:
	explicit DemoApplication(const ApplicationSettings& settings);

	~DemoApplication();

	bool Initialize() noexcept override;

	void Update() noexcept override;

	void Draw() noexcept override;

	void OnResize(const Vec2i& size) noexcept override;
};

#endif // VULKAN_SINGLE_THREADED_APPLICATION_H_
