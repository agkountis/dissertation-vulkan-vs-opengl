#ifndef VULKAN_SINGLE_THREADED_APPLICATION_H_
#define VULKAN_SINGLE_THREADED_APPLICATION_H_
#include "vulkan_application.h"

class VulkanSingleThreadedApplication : public VulkanApplication {
public:
	bool Initialize() noexcept override;

	void Draw() const noexcept override;
};

#endif // VULKAN_SINGLE_THREADED_APPLICATION_H_
