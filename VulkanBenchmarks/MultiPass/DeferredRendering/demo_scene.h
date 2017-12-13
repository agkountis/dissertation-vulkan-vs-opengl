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

constexpr int lightCount{ 3 };

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
		int position{ -1 };
		int normal{ -1 };
		int albedo{ -1 };
		int specular{ -1 };
		int depth{ -1 };
	} m_AttachmentIndices;

	// All textures will be sampled with a single sampler.
	VkSampler m_TextureSampler{ VK_NULL_HANDLE };

	VulkanRenderTarget m_GBuffer;

	LightsUbo m_Lights;

	//UI -------------------------------
    mutable int m_CurrentAttachment{ 0 };
	std::array<const char*, 6> m_AttachmentComboItems{ "Lit", "Position", "Normals", "Albedo", "Specular", "Depth" };
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

	bool InitializeImGUI(const VkRenderPass renderpass) noexcept;

	void UpdateLightPositions(const i64 msec, f64 dt) noexcept;

	void DrawEntity(DemoEntity* entity, VkCommandBuffer commandBuffer) noexcept;

public:
	~DemoScene();

	bool Initialize(VkExtent2D swapChainExtent, VkRenderPass displayRenderPass) noexcept;

	void Update(VkExtent2D swapChainExtent, i64 msec, f64 dt) noexcept;

	void Draw(VkCommandBuffer commandBuffer) noexcept;

	void DrawFullscreenQuad(VkCommandBuffer commandBuffer) const noexcept;

	const VulkanRenderTarget& GetGBuffer() const noexcept;
};

#endif //DISSERTATION_DEMO_SCENE_H
