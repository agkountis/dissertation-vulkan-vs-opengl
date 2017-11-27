#ifndef DISSERTATION_DEMO_SCENE_H
#define DISSERTATION_DEMO_SCENE_H

#include <memory>
#include <vulkan_pipeline_cache.h>
#include "demo_entity.h"
#include "vulkan_render_target.h"

struct MatricesUbo {
	Mat4f view;
	Mat4f projection;
};

struct LightsUbo {};

class DemoScene {
private:
	std::vector<std::unique_ptr<DemoEntity>> m_Entities;

	VkDescriptorPool m_DescriptorPool{ VK_NULL_HANDLE };

	struct {
		VkDescriptorSetLayout sceneMatrices{ VK_NULL_HANDLE };
		VkDescriptorSetLayout material{ VK_NULL_HANDLE };
		VkDescriptorSetLayout gBufferAndLights{ VK_NULL_HANDLE };
	} m_DescriptorSetLayouts;

	struct {
		VkDescriptorSet sceneMatrices{ VK_NULL_HANDLE };
		VkDescriptorSet display{ VK_NULL_HANDLE };
	} m_DescriptorSets;


	struct {
		VulkanBuffer matrices;
		VulkanBuffer lights;
	} m_Ubos;

	struct {
		VkPipeline deferred{ VK_NULL_HANDLE };
		VkPipeline display{ VK_NULL_HANDLE };
	} m_Pipelines;

	VulkanPipelineCache m_PipelineCache;

	struct {
		VkPipelineLayout deferred;
		VkPipelineLayout display;
	} m_PipelineLayouts;

	struct {
		int position;
		int normal;
		int albedo;
		int specular;
		int depth;
	} m_AttachmentIndices;

	// All textures will be sampled with a single sampler.
	VkSampler m_TextureSampler{ VK_NULL_HANDLE };

	VulkanMesh m_CubeMesh;

	DemoMaterial m_Material;

	VulkanRenderTarget m_GBuffer;

	bool SpawnEntity() noexcept;

	bool CreateTextureSampler() noexcept;

	bool PrepareUniforms() noexcept;

	bool CreatePipelines(VkExtent2D swapChainExtent, VkRenderPass displayRenderPass) noexcept;

public:
	~DemoScene();

	bool Initialize(VkExtent2D swapChainExtent, VkRenderPass displayRenderPass) noexcept;

	void Update(VkExtent2D swapChainExtent, i64 msec, f64 dt) noexcept;

	void Draw(VkCommandBuffer commandBuffer) noexcept;

	void DrawFullscreenQuad(VkCommandBuffer commandBuffer) const noexcept;

	const VulkanRenderTarget& GetGBuffer() const noexcept;
};

#endif //DISSERTATION_DEMO_SCENE_H
