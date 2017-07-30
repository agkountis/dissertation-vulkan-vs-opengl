#ifndef VULKAN_PIPELINE_CACHE_H_
#define VULKAN_PIPELINE_CACHE_H_

#include <vulkan/vulkan.h>

class VulkanPipelineCache {
private:
	VkPipelineCache m_PipelineCache{ VK_NULL_HANDLE };

public:
	~VulkanPipelineCache();

	bool Create() noexcept;

	operator VkPipelineCache() const noexcept;
};

#endif //VULKAN_PIPELINE_CACHE_H_
