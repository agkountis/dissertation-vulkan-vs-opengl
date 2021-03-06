#ifndef DISSERTATION_DEMO_SCENE_H
#define DISSERTATION_DEMO_SCENE_H

#include <memory>
#include <vulkan_pipeline_cache.h>
#include "demo_entity.h"
#include "vulkan_render_target.h"
#include "assimp/scene.h"

struct MatricesUbo {
	Mat4f view;
	Mat4f projection;
};

constexpr int lightCount{ 4 };

struct LightsUbo {
	Vec4f positions[lightCount];
	Vec4f colors[lightCount];
	Vec4f radi[lightCount];
    Vec3f eyePos;
};

class DemoScene final {
private:
	std::vector<std::unique_ptr<DemoEntity>> m_Entities;

	VkDescriptorPool m_DescriptorPool{ VK_NULL_HANDLE };

	VkDescriptorPool m_ImGUIDescriptorPool{ VK_NULL_HANDLE };

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
		VkPipelineLayout deferred{ VK_NULL_HANDLE };
		VkPipelineLayout display{ VK_NULL_HANDLE };
	} m_PipelineLayouts;

	struct {
		i32 position{ -1 };
		i32 normal{ -1 };
		i32 albedo{ -1 };
		i32 specular{ -1 };
		i32 depth{ -1 };
	} m_AttachmentIndices;

	// All textures will be sampled with a single sampler.
	VkSampler m_TextureSampler{ VK_NULL_HANDLE };

	VulkanRenderTarget m_GBuffer;

	LightsUbo m_Lights;

	//UI -------------------------------
    mutable i32 m_CurrentAttachment{ 0 };
	std::array<const char*, 6> m_AttachmentComboItems{ "Lit", "Position", "Normals", "Albedo", "Specular", "Depth" };

	i64 m_SceneVertexCount{ 0 };
	// ---------------------------

	// Model Loading--------------------
	std::vector<DemoMaterial> m_Materials;

	std::vector<VulkanMesh*> m_Meshes;

	void LoadMeshes(const aiScene* scene) noexcept;

	void LoadMaterials(const aiScene* scene) noexcept;

	DemoEntity* LoadNode(const aiNode* aiNode) noexcept;

	std::unique_ptr<DemoEntity> LoadModel(const std::string& fileName) noexcept;
	//----------------------------------

	bool CreateTextureSampler() noexcept;

	bool PrepareUniforms() noexcept;

	bool CreatePipelines(VkExtent2D swapChainExtent, VkRenderPass displayRenderPass) noexcept;

	bool InitializeImGui(VkRenderPass renderPass) noexcept;

	void DrawEntity(DemoEntity* entity, VkCommandBuffer commandBuffer) noexcept;

	void DrawUi(VkCommandBuffer commandBuffer) const noexcept;

public:
	~DemoScene();

	bool Initialize(VkExtent2D swapChainExtent, VkRenderPass displayRenderPass) noexcept;

	void Update(VkExtent2D swapChainExtent, i64 msec, f64 dt) noexcept;

	void Draw(VkCommandBuffer commandBuffer) noexcept;

	void DrawFullscreenQuad(const VkCommandBuffer commandBuffer) const noexcept;

	const VulkanRenderTarget& GetGBuffer() const noexcept;
};

#endif //DISSERTATION_DEMO_SCENE_H
