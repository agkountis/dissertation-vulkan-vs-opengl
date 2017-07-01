#ifndef VULKAN_SINGLE_THREADED_APPLICATION_H_
#define VULKAN_SINGLE_THREADED_APPLICATION_H_
#include "application.h"
#include "vulkan_window.h"
#include <memory>
#include "vulkan_instance.h"
#include <optional>

class VulkanSingleThreadedApplication : public Application {
private:
	std::unique_ptr<VulkanWindow> m_Window;

	std::unique_ptr<VulkanInstance> m_Instance;

	std::optional<VkPhysicalDevice> PickPhysicalDevice(VkInstance instance, VkSurfaceKHR surface, const std::vector<std::string>& requiredExtensions) const noexcept;

public:
	bool Initialize() override;

	i32 Run() override;

	void Draw() noexcept override;
};

#endif // VULKAN_SINGLE_THREADED_APPLICATION_H_
