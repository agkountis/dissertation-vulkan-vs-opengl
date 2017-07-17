#ifndef VULKAN_SINGLE_THREADED_APPLICATION_H_
#define VULKAN_SINGLE_THREADED_APPLICATION_H_
#include "vulkan_application.h"

class VulkanSingleThreadedApplication : public VulkanApplication {
private:
	VkPipeline m_Pipeline{ VK_NULL_HANDLE };

	VkPipelineLayout m_PipelineLayout{ VK_NULL_HANDLE };

	VkDescriptorSet m_DescriptorSet{ VK_NULL_HANDLE };

	VkDescriptorSetLayout m_DescriptorSetLayout{ VK_NULL_HANDLE };

	void EnableFeatures() noexcept override;

	bool CreatePipelines() noexcept override;

public:
	explicit VulkanSingleThreadedApplication(const ApplicationSettings& settings);

	~VulkanSingleThreadedApplication();

	bool Initialize() noexcept override;

	void Draw() const noexcept override;
};

#endif // VULKAN_SINGLE_THREADED_APPLICATION_H_
