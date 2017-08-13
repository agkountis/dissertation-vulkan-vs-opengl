#ifndef DISSERTATION_DEMO_SCENE_H
#define DISSERTATION_DEMO_SCENE_H

#include <memory>
#include <vulkan_pipeline_cache.h>
#include "demo_entity.h"

struct UniformBufferObject {
	Mat4f view;
	Mat4f projection;
};

class DemoScene {
private:
	std::vector<std::unique_ptr<DemoEntity>> m_Entities;

	VkDescriptorPool m_DescriptorPool{ VK_NULL_HANDLE };

	struct {
		VkDescriptorSetLayout sceneMatrices{ VK_NULL_HANDLE };
		VkDescriptorSetLayout material{ VK_NULL_HANDLE };
	} m_DescriptorSetLayouts;

	VkDescriptorSet m_SceneMatricesDescriptorSet{ VK_NULL_HANDLE };

	VulkanBuffer m_MatricesUbo;

	struct {
		VkPipeline solid{ VK_NULL_HANDLE };
		VkPipeline wireframe{ VK_NULL_HANDLE };
	} m_Pipelines;

	VulkanPipelineCache m_PipelineCache;

	VkPipelineLayout m_PipelineLayout;

	// All textures will be sampled with a single sampler.
	VkSampler m_TextureSampler{ VK_NULL_HANDLE };

	VulkanMesh m_CubeMesh;

	bool GenerateEntities() noexcept;

	bool CreateTextureSampler() noexcept;

	bool PrepareUniforms() noexcept;

	bool CreatePipelines(VkExtent2D swapChainExtent, VkRenderPass renderPass) noexcept;

public:
	~DemoScene();

	bool Initialize(VkExtent2D swapChainExtent, VkRenderPass renderPass) noexcept;

	void Update(VkExtent2D swapChainExtent, i64 msec, f64 dt) noexcept;

	void Draw(VkCommandBuffer commandBuffer) noexcept;
};

#endif //DISSERTATION_DEMO_SCENE_H
