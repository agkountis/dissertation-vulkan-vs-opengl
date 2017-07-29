#ifndef VULKAN_SINGLE_THREADED_APPLICATION_H_
#define VULKAN_SINGLE_THREADED_APPLICATION_H_
#include "vulkan_application.h"
#include "vulkan_mesh.h"

struct UniformBufferObject {
	Mat4f model;
	Mat4f view;
	Mat4f projection;
	Mat4f inverseTransposeModelView;
	f32 time;
};

class VulkanSingleThreadedApplication : public VulkanApplication {
private:
	VkPipeline m_Pipeline{ VK_NULL_HANDLE };

	VkPipeline m_WireframePipeline{ VK_NULL_HANDLE };

	VkPipelineLayout m_PipelineLayout{ VK_NULL_HANDLE };

	VkDescriptorPool m_DescriptorPool{ VK_NULL_HANDLE };

	VkDescriptorSetLayout m_DescriptorSetLayout{ VK_NULL_HANDLE };

	VkDescriptorSet m_DescriptorSet{ VK_NULL_HANDLE };

	VulkanBuffer m_Ubo;

	VulkanMesh mesh;

	void EnableFeatures() noexcept override;

	bool CreateUniforms() noexcept;

	bool CreatePipelines() noexcept override;

	bool BuildCommandBuffers() noexcept override;

public:
	explicit VulkanSingleThreadedApplication(const ApplicationSettings& settings);

	~VulkanSingleThreadedApplication() override;

	bool Initialize() noexcept override;

	void Update() noexcept override;

	void Draw() noexcept override;

	void OnResize(const Vec2i& size) noexcept override;
};

#endif // VULKAN_SINGLE_THREADED_APPLICATION_H_
