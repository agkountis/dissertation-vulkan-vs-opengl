#ifndef VULKAN_SINGLE_THREADED_APPLICATION_H_
#define VULKAN_SINGLE_THREADED_APPLICATION_H_

#include "vulkan_application.h"
#include "demo_entity.h"
#include "demo_scene.h"

class DemoApplication final : public VulkanApplication {
private:
	DemoScene m_DemoScene;

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
