#include <logger.h>
#include <vulkan_infrastructure_context.h>
#include <random>
#include <vulkan_shader.h>
#include <mesh_utilities.h>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include "demo_scene.h"
#include "vulkan_application.h"

// Vulkan clip space has inverted Y and half Z.
static const Mat4f s_ClipCorrectionMat{
	1.0f, 0.0f, 0.0f, 0.0f,
	0.0f, -1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.5f, 0.0f,
	0.0f, 0.0f, 0.5f, 1.0f
};

// Private functions -------------------------------------------------
bool DemoScene::SpawnEntity() noexcept
{
	if (!GenerateCube(&m_CubeMesh, 1.0f)) {
		return false;
	}

	using namespace std::chrono;
	auto seed = high_resolution_clock::now().time_since_epoch().count();
	std::mt19937 rng{ static_cast<ui32>(seed) };

	auto realRangeRng = [&rng](float rangeBegin, float rangeEnd)
	{
		std::uniform_real_distribution<float> r(rangeBegin, rangeEnd);

		return r(rng);
	};

	for (int i = 0; i < ENTITY_COUNT; ++i) {

		auto entity = std::make_unique<DemoEntity>(&m_CubeMesh);

		entity->SetPosition(Vec3f{
			realRangeRng(-20.0f, 20.0f),
			realRangeRng(-20.0f, 20.0f),
			realRangeRng(-20.0f, 20.0f)
		});


		auto& material = entity->GetMaterial();

		material.textures[TEX_DIFFUSE] = G_ResourceManager.Get<VulkanTexture>("textures/tunnelDiff5.png",
		                                                                      TEX_DIFFUSE,
		                                                                      VK_FORMAT_R8G8B8A8_UNORM,
		                                                                      VK_IMAGE_ASPECT_COLOR_BIT);

		material.textures[TEX_SPECULAR] = G_ResourceManager.Get<VulkanTexture>("textures/tunnelSpec5.png",
		                                                                       TEX_SPECULAR,
		                                                                       VK_FORMAT_R8G8B8A8_UNORM,
		                                                                       VK_IMAGE_ASPECT_COLOR_BIT);

		material.textures[TEX_NORMAL] = G_ResourceManager.Get<VulkanTexture>("textures/tunnelNorm5.png",
		                                                                     TEX_NORMAL,
		                                                                     VK_FORMAT_R8G8B8A8_UNORM,
		                                                                     VK_IMAGE_ASPECT_COLOR_BIT);

		entity->Update(0.0f);

		m_Entities.push_back(std::move(entity));
	}

	// Sort entities from back to front to avoid early z optimizations.
	std::sort(m_Entities.begin(), m_Entities.end(), [](auto& a, auto& b)
	{
		return a->GetPosition().z > b->GetPosition().z;
	});

	return true;
}

bool DemoScene::CreateTextureSampler() noexcept
{
	// All textures will be sampled with the same sampler for this scene.
	VkSamplerCreateInfo samplerCreateInfo{};
	samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
	samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
	samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCreateInfo.anisotropyEnable = VK_TRUE;

	// If the device supports anisotropic filtering...
	if (G_VulkanDevice.GetPhysicalDevice().features.samplerAnisotropy) {
		samplerCreateInfo.maxAnisotropy = 16;
	}
	else {
		samplerCreateInfo.maxAnisotropy = 0;
	}

	samplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;
	samplerCreateInfo.compareEnable = VK_FALSE;
	samplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerCreateInfo.mipLodBias = 0.0f;
	samplerCreateInfo.minLod = 0.0f;
	samplerCreateInfo.maxLod = 0.0f;

	VkResult result{ vkCreateSampler(G_VulkanDevice, &samplerCreateInfo, nullptr, &m_TextureSampler) };

	if (result != VK_SUCCESS) {
		ERROR_LOG("Failed to create texture sampler.");
		return false;
	}

	return true;
}

bool DemoScene::PrepareUniforms() noexcept
{
	//First create a descriptor pool to allocate descriptors from.
	//Descriptors a.k.a uniforms

	// We need 1 descriptor set for the scene matrices.
	VkDescriptorPoolSize sceneMatricesPoolSize{};
	sceneMatricesPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	sceneMatricesPoolSize.descriptorCount = 1;

	//and ENTITY_COUNT descriptor sets for the material of each entity.
	VkDescriptorPoolSize materialPoolSize{};

	// The material descriptor sets will contain image samplers (Textures) only.
	// the rest of the material attributes will use a push constant block.
	materialPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

	// 3 textures per material of each entity.
	materialPoolSize.descriptorCount = static_cast<ui32>(3 * m_Entities.size());

	std::vector<VkDescriptorPoolSize> descriptorPoolSizes{};
	descriptorPoolSizes.push_back(sceneMatricesPoolSize);
	descriptorPoolSizes.push_back(materialPoolSize);

	VkDescriptorPoolCreateInfo descriptorPoolCreateInfo{};
	descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;

	//1 set for each entity's material plus one for the scene matrices
	descriptorPoolCreateInfo.maxSets = static_cast<ui32>(m_Entities.size() + 1);
	descriptorPoolCreateInfo.poolSizeCount = static_cast<ui32>(descriptorPoolSizes.size());
	descriptorPoolCreateInfo.pPoolSizes = descriptorPoolSizes.data();

	const auto& device = G_VulkanDevice;

	VkResult result{
		vkCreateDescriptorPool(device,
		                       &descriptorPoolCreateInfo,
		                       nullptr,
		                       &m_DescriptorPool)
	};

	if (result != VK_SUCCESS) {
		ERROR_LOG("Failed to create descriptor pool.");
		return false;
	}

	//Now a descriptor set has to be created, but before that, it's layout
	//has to be defined.
	//First the bindings have to be defined...
	//Descriptor set 0 - scene matrices;
	VkDescriptorSetLayoutBinding vertexShaderUboBinding{};
	vertexShaderUboBinding.binding = 0; //bind at location 0.
	vertexShaderUboBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; //It's a uniform m_Buffer
	vertexShaderUboBinding.descriptorCount = 1; //1 descriptor.
	vertexShaderUboBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT; //bound to the vertex shader stage

	std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings{ vertexShaderUboBinding };

	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{};
	descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorSetLayoutCreateInfo.bindingCount = static_cast<ui32>(descriptorSetLayoutBindings.size());
	descriptorSetLayoutCreateInfo.pBindings = descriptorSetLayoutBindings.data();

	result = vkCreateDescriptorSetLayout(device,
	                                     &descriptorSetLayoutCreateInfo,
	                                     nullptr,
	                                     &m_DescriptorSetLayouts.sceneMatrices);

	if (result != VK_SUCCESS) {
		ERROR_LOG("Failed to create scene matrices descriptor set layout.");
		return false;
	}

	descriptorSetLayoutBindings.clear();

	//Descriptor set 1 - entity materials
	VkDescriptorSetLayoutBinding diffuseTextureShaderBinding{};
	diffuseTextureShaderBinding.binding = TEX_DIFFUSE; //bind at location 0.
	diffuseTextureShaderBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; //Binding samplers.
	diffuseTextureShaderBinding.descriptorCount = 1;
	diffuseTextureShaderBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT; //bound to the fragment shader stage.

	descriptorSetLayoutBindings.push_back(diffuseTextureShaderBinding);

	VkDescriptorSetLayoutBinding specularTextureShaderBinding{};
	specularTextureShaderBinding.binding = TEX_SPECULAR; //bind at location 1.
	specularTextureShaderBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	specularTextureShaderBinding.descriptorCount = 1;
	specularTextureShaderBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	descriptorSetLayoutBindings.push_back(specularTextureShaderBinding);

	VkDescriptorSetLayoutBinding normalTextureShaderBinding{};
	normalTextureShaderBinding.binding = TEX_NORMAL; //bind at location 2
	normalTextureShaderBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	normalTextureShaderBinding.descriptorCount = 1;
	normalTextureShaderBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	descriptorSetLayoutBindings.push_back(normalTextureShaderBinding);

	descriptorSetLayoutCreateInfo.bindingCount = static_cast<ui32>(descriptorSetLayoutBindings.size());
	descriptorSetLayoutCreateInfo.pBindings = descriptorSetLayoutBindings.data();

	result = vkCreateDescriptorSetLayout(device,
	                                     &descriptorSetLayoutCreateInfo,
	                                     nullptr,
	                                     &m_DescriptorSetLayouts.material);

	if (result != VK_SUCCESS) {
		ERROR_LOG("Failed to create materials descriptor set layout.");
		return false;
	}

	// After the descriptor set layouts have been defined and created,
	// the pipeline layout can be defined. The pipeline layout will need the
	// descriptor layouts created above plus the constant ranges that will be defined bellow.
	// The pipeline layout tells the driver how the uniforms (and other data) will be organized
	// and passed to the shaders.
	// Create the pipeline layout.
	// Here we specify to the the pipeline if we use any push constants or descriptor sets.

	// Use a push constant block for the model matrix of each object...
	std::array<VkPushConstantRange, 2> pushConstantRanges{};
	pushConstantRanges[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	pushConstantRanges[0].size = sizeof(Mat4f); //1 4x4 matrix
	pushConstantRanges[0].offset = 0;

	// ... and a push constant range for the diffuse and specular vectors of the material.
	pushConstantRanges[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	pushConstantRanges[1].size = 2 * sizeof(Vec4f); // 2 vectors
	pushConstantRanges[1].offset = 64;

	std::vector<VkDescriptorSetLayout> descriptorSetLayouts{
		m_DescriptorSetLayouts.sceneMatrices,
		m_DescriptorSetLayouts.material
	};

	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.setLayoutCount = static_cast<ui32>(descriptorSetLayouts.size());
	pipelineLayoutCreateInfo.pSetLayouts = descriptorSetLayouts.data();
	pipelineLayoutCreateInfo.pushConstantRangeCount = static_cast<ui32>(pushConstantRanges.size());
	pipelineLayoutCreateInfo.pPushConstantRanges = pushConstantRanges.data();

	result = vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &m_PipelineLayout);
	if (result != VK_SUCCESS) {
		ERROR_LOG("Failed to create pipeline layout.");
		return false;
	}

	// Now all the layouts have been defined. It is time to allocate the descriptor sets and write to them.
	// First, allocate the scene matrices descriptor set.
	VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{};
	descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocateInfo.descriptorPool = m_DescriptorPool; //Allocate from this pool.
	descriptorSetAllocateInfo.descriptorSetCount = 1; //1 descriptor set.
	descriptorSetAllocateInfo.pSetLayouts = &m_DescriptorSetLayouts.sceneMatrices; //With this layout.

	result = vkAllocateDescriptorSets(device, &descriptorSetAllocateInfo, &m_SceneMatricesDescriptorSet);

	if (result != VK_SUCCESS) {
		ERROR_LOG("Failed to allocate scene matrices descriptor set.");
		return false;
	}

	// Create the buffer object that will store the uniforms.
	if (!device.CreateBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
	                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
	                         m_MatricesUbo,
	                         sizeof(UniformBufferObject))) {
		ERROR_LOG("Failed to create uniform buffer.");
		return false;
	}

	// Now that the descriptor set is allocated we have to write into it and update the uniforms.
	VkWriteDescriptorSet uniformDescriptorWrite{};
	uniformDescriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	uniformDescriptorWrite.dstSet = m_SceneMatricesDescriptorSet; //Write to this set
	uniformDescriptorWrite.dstBinding = 0; //at this binding
	uniformDescriptorWrite.dstArrayElement = 0; //It is not an array.
	uniformDescriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; //It is a uniform buffer.
	uniformDescriptorWrite.descriptorCount = 1; // 1 descriptor set.
	uniformDescriptorWrite.pBufferInfo = &m_MatricesUbo.descriptorBufferInfo; // And here is the buffer description.

	std::vector<VkWriteDescriptorSet> writeDescriptorSets{ uniformDescriptorWrite };

	// Update the descriptor set.
	vkUpdateDescriptorSets(device,
	                       static_cast<ui32>(writeDescriptorSets.size()),
	                       writeDescriptorSets.data(),
	                       0,
	                       nullptr);

	writeDescriptorSets.clear();

	// Allocate 1 descriptor set for each entity's material
	for (const auto& entity : m_Entities) {
		descriptorSetAllocateInfo.descriptorPool = m_DescriptorPool; //Allocate from this pool.
		descriptorSetAllocateInfo.descriptorSetCount = 1; //1 descriptor set.
		descriptorSetAllocateInfo.pSetLayouts = &m_DescriptorSetLayouts.material; //With this layout.

		result = vkAllocateDescriptorSets(device,
		                                  &descriptorSetAllocateInfo,
		                                  &entity->GetMaterial().descriptorSet);

		if (result != VK_SUCCESS) {
			ERROR_LOG("Failed to allocate material descriptor set.");
			return false;
		}

		const auto& material = entity->GetMaterial();

		// Define the descriptor image info structures for each texture type.
		VkDescriptorImageInfo diffuseTextureImageInfo{};
		diffuseTextureImageInfo.imageView = material.textures[TEX_DIFFUSE]->GetImageView();
		diffuseTextureImageInfo.imageLayout = material.textures[TEX_DIFFUSE]->GetImageLayout();
		diffuseTextureImageInfo.sampler = m_TextureSampler;

		VkDescriptorImageInfo specularTextureImageInfo{};
		specularTextureImageInfo.imageView = material.textures[TEX_SPECULAR]->GetImageView();
		specularTextureImageInfo.imageLayout = material.textures[TEX_SPECULAR]->GetImageLayout();
		specularTextureImageInfo.sampler = m_TextureSampler;

		VkDescriptorImageInfo normalTextureImageInfo{};
		normalTextureImageInfo.imageView = material.textures[TEX_NORMAL]->GetImageView();
		normalTextureImageInfo.imageLayout = material.textures[TEX_NORMAL]->GetImageLayout();
		normalTextureImageInfo.sampler = m_TextureSampler;

		// Define the descriptor writes for each texture type.
		VkWriteDescriptorSet diffuseTextureDescriptorWrite{};
		diffuseTextureDescriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		diffuseTextureDescriptorWrite.dstSet = material.descriptorSet;
		diffuseTextureDescriptorWrite.dstBinding = TEX_DIFFUSE;
		diffuseTextureDescriptorWrite.dstArrayElement = 0;
		diffuseTextureDescriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		diffuseTextureDescriptorWrite.descriptorCount = 1;
		diffuseTextureDescriptorWrite.pImageInfo = &diffuseTextureImageInfo;

		writeDescriptorSets.push_back(diffuseTextureDescriptorWrite);

		VkWriteDescriptorSet specularTextureDescriptorWrite{};
		specularTextureDescriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		specularTextureDescriptorWrite.dstSet = material.descriptorSet;
		specularTextureDescriptorWrite.dstBinding = TEX_SPECULAR;
		specularTextureDescriptorWrite.dstArrayElement = 0;
		specularTextureDescriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		specularTextureDescriptorWrite.descriptorCount = 1;
		specularTextureDescriptorWrite.pImageInfo = &specularTextureImageInfo;

		writeDescriptorSets.push_back(specularTextureDescriptorWrite);

		VkWriteDescriptorSet normalTextureDescriptorWrite{};
		normalTextureDescriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		normalTextureDescriptorWrite.dstSet = material.descriptorSet;
		normalTextureDescriptorWrite.dstBinding = TEX_NORMAL;
		normalTextureDescriptorWrite.dstArrayElement = 0;
		normalTextureDescriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		normalTextureDescriptorWrite.descriptorCount = 1;
		normalTextureDescriptorWrite.pImageInfo = &normalTextureImageInfo;

		writeDescriptorSets.push_back(normalTextureDescriptorWrite);

		// Finally update the material's descriptor set.
		vkUpdateDescriptorSets(device,
		                       static_cast<ui32>(writeDescriptorSets.size()),
		                       writeDescriptorSets.data(),
		                       0,
		                       nullptr);

		writeDescriptorSets.clear();
	}

	m_MatricesUbo.Map(sizeof m_MatricesUbo);

	return true;
}

bool DemoScene::CreatePipelines(const VkExtent2D swapChainExtent, const VkRenderPass renderPass) noexcept
{
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyState{};
	inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssemblyState.flags = VK_NULL_HANDLE;
	inputAssemblyState.primitiveRestartEnable = VK_FALSE;

	VkPipelineRasterizationStateCreateInfo rasterizationState{};
	rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizationState.flags = VK_NULL_HANDLE;
	rasterizationState.depthClampEnable = VK_FALSE;
	rasterizationState.rasterizerDiscardEnable = VK_FALSE;
	rasterizationState.lineWidth = 1.0f;

	VkPipelineColorBlendAttachmentState colorBlendAttachmentState{};
	colorBlendAttachmentState.colorWriteMask =
			VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachmentState.blendEnable = VK_FALSE;
	colorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo colorBlendState{};
	colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendState.logicOpEnable = VK_FALSE;
	colorBlendState.logicOp = VK_LOGIC_OP_COPY;
	colorBlendState.attachmentCount = 1;
	colorBlendState.pAttachments = &colorBlendAttachmentState;

	for (auto& blendConstant : colorBlendState.blendConstants) {
		blendConstant = 0.0f;
	}

	VkPipelineDepthStencilStateCreateInfo depthStencilState{};
	depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencilState.depthTestEnable = VK_TRUE;
	depthStencilState.depthWriteEnable = VK_TRUE;
	depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencilState.depthBoundsTestEnable = VK_FALSE;
	depthStencilState.stencilTestEnable = VK_FALSE;

	VkViewport viewport{};
	viewport.width = static_cast<float>(swapChainExtent.width);
	viewport.height = static_cast<float>(swapChainExtent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor{};
	scissor.extent = swapChainExtent;
	scissor.offset = VkOffset2D{ 0, 0 };

	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	VkPipelineMultisampleStateCreateInfo multisampleState{};
	multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampleState.sampleShadingEnable = VK_FALSE;
	multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampleState.minSampleShading = 1.0f;
	multisampleState.pSampleMask = nullptr;
	multisampleState.alphaToCoverageEnable = VK_FALSE;
	multisampleState.alphaToOneEnable = VK_FALSE;

	std::vector<VkDynamicState> dynamicStateToEnable = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};

	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.pDynamicStates = dynamicStateToEnable.data();
	dynamicState.dynamicStateCount = static_cast<ui32>(dynamicStateToEnable.size());
	dynamicState.flags = VK_NULL_HANDLE;

	// Solid rendering pipeline
	// Load shaders
	VulkanShader* vertexShader{ G_ResourceManager.Get<VulkanShader>("sdr/default.vert.spv") };

	if (!vertexShader) {
		ERROR_LOG("Failed to load vertex shader.");
		return false;
	}

	VulkanShader* fragmentShader{ G_ResourceManager.Get<VulkanShader>("sdr/default.frag.spv") };

	if (!fragmentShader) {
		ERROR_LOG("Failed to load fragment shader.");
		return false;
	}

	VkPipelineShaderStageCreateInfo vertexShaderStage{};
	vertexShaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertexShaderStage.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertexShaderStage.module = *vertexShader;
	vertexShaderStage.pName = "main"; //Shader function entry point.

	VkPipelineShaderStageCreateInfo fragmentShaderStage{};
	fragmentShaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragmentShaderStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragmentShaderStage.module = *fragmentShader;
	fragmentShaderStage.pName = "main"; //Shader function entry point.

	std::vector<VkPipelineShaderStageCreateInfo> shaderStages{
		vertexShaderStage,
		fragmentShaderStage
	};

	auto vertexInputBindingDescription = VulkanMesh::GetVertexInputBindingDescription();

	auto vertexInputAttributeDescriptions = VulkanMesh::GetVertexInputAttributeDescriptions();

	// Vertex binding information and attribute descriptions.
	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<ui32>(vertexInputAttributeDescriptions.size());
	vertexInputInfo.pVertexBindingDescriptions = &vertexInputBindingDescription;
	vertexInputInfo.pVertexAttributeDescriptions = vertexInputAttributeDescriptions.data();

	VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
	pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineCreateInfo.renderPass = renderPass;
	pipelineCreateInfo.layout = m_PipelineLayout;
	pipelineCreateInfo.flags = VK_NULL_HANDLE;
	pipelineCreateInfo.basePipelineIndex = -1;
	pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineCreateInfo.pVertexInputState = &vertexInputInfo;
	pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
	pipelineCreateInfo.pRasterizationState = &rasterizationState;
	pipelineCreateInfo.pColorBlendState = &colorBlendState;
	pipelineCreateInfo.pMultisampleState = &multisampleState;
	pipelineCreateInfo.pViewportState = &viewportState;
	pipelineCreateInfo.pDepthStencilState = &depthStencilState;
	pipelineCreateInfo.pDynamicState = &dynamicState;
	pipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
	pipelineCreateInfo.pStages = shaderStages.data();

	VkResult result{
		vkCreateGraphicsPipelines(G_VulkanDevice,
		                          m_PipelineCache,
		                          1,
		                          &pipelineCreateInfo,
		                          nullptr,
		                          &m_Pipelines.solid)
	};

	if (result != VK_SUCCESS) {
		ERROR_LOG("Failed to create pipeline.");
		return false;
	}

	//Wire frame pipeline
	if (G_VulkanDevice.GetPhysicalDevice().features.fillModeNonSolid) {

		rasterizationState.polygonMode = VK_POLYGON_MODE_LINE;
		rasterizationState.lineWidth = 1.0f;

		result = vkCreateGraphicsPipelines(G_VulkanDevice,
		                                   m_PipelineCache,
		                                   1,
		                                   &pipelineCreateInfo,
		                                   nullptr,
		                                   &m_Pipelines.wireframe);
	}

	if (result != VK_SUCCESS) {
		ERROR_LOG("Failed to create pipeline.");
		return false;
	}

	return true;
}

// -------------------------------------------------------------------
DemoScene::~DemoScene()
{
	const auto& device = G_VulkanDevice;

	vkDestroySampler(device, m_TextureSampler, nullptr);

	//No need to free descriptor sets. They are taken care of by the Vulkan driver.
	//Just destroy the descriptor set layout and the descriptor pool.
	vkDestroyDescriptorSetLayout(device, m_DescriptorSetLayouts.sceneMatrices, nullptr);
	vkDestroyDescriptorSetLayout(device, m_DescriptorSetLayouts.material, nullptr);

	vkDestroyDescriptorPool(device, m_DescriptorPool, nullptr);

	vkDestroyPipeline(device, m_Pipelines.solid, nullptr);

	vkDestroyPipeline(device, m_Pipelines.wireframe, nullptr);

	vkDestroyPipelineLayout(device, m_PipelineLayout, nullptr);
}

bool DemoScene::Initialize(const VkExtent2D swapChainExtent, const VkRenderPass renderPass) noexcept
{
	if (!SpawnEntity()) {
		ERROR_LOG("Failed to generate scene's entities.");
		return false;
	}

	if (!m_PipelineCache.Create()) {
		return false;
	}

	if (!CreateTextureSampler()) {
		return false;
	}

	if (!PrepareUniforms()) {
		ERROR_LOG("Failed to prepare the scene's uniforms");
		return false;
	}

	if (!CreatePipelines(swapChainExtent, renderPass)) {
		ERROR_LOG("Failed to create scene's pipelines.");
		return false;
	}

	return true;
}

void DemoScene::Update(const VkExtent2D swapChainExtent, const i64 msec, f64 dt) noexcept
{
	UniformBufferObject ubo{};
	ubo.view = glm::lookAt(Vec3f{ 0.0f, 0.0f, 80.0f }, Vec3f{}, Vec3f{ 0.0f, 1.0f, 0.0f });

	//ubo.view = glm::rotate(ubo.view, msec / 1000.0f * glm::radians(5.0f), Vec3f{1.0f, 1.0f, 1.0f});

	const auto aspect = static_cast<f32>(swapChainExtent.width) / static_cast<f32>(swapChainExtent.height);

	ubo.projection = s_ClipCorrectionMat * glm::perspective(glm::radians(45.0f), aspect, 0.1f, 200.0f);

	m_MatricesUbo.Fill(&ubo, sizeof ubo);
}

void DemoScene::Draw(const VkCommandBuffer commandBuffer) noexcept
{
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipelines.solid);

	for (const auto& entity : m_Entities) {

		auto& material = entity->GetMaterial();
		std::vector<VkDescriptorSet> descriptorSets{
			m_SceneMatricesDescriptorSet,
			material.descriptorSet
		};

		vkCmdBindDescriptorSets(commandBuffer,
		                        VK_PIPELINE_BIND_POINT_GRAPHICS,
		                        m_PipelineLayout,
		                        0,
		                        static_cast<ui32>(descriptorSets.size()),
		                        descriptorSets.data(),
		                        0,
		                        nullptr);

		const auto& xform = entity->GetXform();

		vkCmdPushConstants(commandBuffer,
		                   m_PipelineLayout,
		                   VK_SHADER_STAGE_VERTEX_BIT,
		                   0,
		                   sizeof(Mat4f),
		                   &xform);

		std::array<Vec4f, 2> materialProperties{ material.diffuse, material.specular };

		vkCmdPushConstants(commandBuffer,
		                   m_PipelineLayout,
		                   VK_SHADER_STAGE_FRAGMENT_BIT,
		                   sizeof(Mat4f),
		                   2 * sizeof(Vec4f),
		                   materialProperties.data());

		entity->Draw(commandBuffer);
	}
}

void DemoScene::DrawSingle(int entityIndex,
                           VkCommandBuffer commandBuffer,
                           VkCommandBufferInheritanceInfo inheritanceInfo) const noexcept
{
	VkCommandBufferBeginInfo commandBufferBeginInfo{};
	commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
	commandBufferBeginInfo.pInheritanceInfo = &inheritanceInfo;

	vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);

	auto extent = G_Application.GetSwapChain().GetExtent();

	VkViewport viewport{};
	viewport.width = static_cast<float>(extent.width);
	viewport.height = static_cast<float>(extent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

	VkRect2D scissor{};
	scissor.extent = extent;
	scissor.offset = VkOffset2D{ 0, 0 };
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipelines.solid);

	auto& material = m_Entities[entityIndex]->GetMaterial();
	std::vector<VkDescriptorSet> descriptorSets{
		m_SceneMatricesDescriptorSet,
		material.descriptorSet
	};

	vkCmdBindDescriptorSets(commandBuffer,
	                        VK_PIPELINE_BIND_POINT_GRAPHICS,
	                        m_PipelineLayout,
	                        0,
	                        static_cast<ui32>(descriptorSets.size()),
	                        descriptorSets.data(),
	                        0,
	                        nullptr);

	const auto& xform = m_Entities[entityIndex]->GetXform();

	vkCmdPushConstants(commandBuffer,
	                   m_PipelineLayout,
	                   VK_SHADER_STAGE_VERTEX_BIT,
	                   0,
	                   sizeof(Mat4f),
	                   &xform);

	std::array<Vec4f, 2> materialProperties{ material.diffuse, material.specular };

	vkCmdPushConstants(commandBuffer,
	                   m_PipelineLayout,
	                   VK_SHADER_STAGE_FRAGMENT_BIT,
	                   sizeof(Mat4f),
	                   2 * sizeof(Vec4f),
	                   materialProperties.data());

	m_Entities[entityIndex]->Draw(commandBuffer);

	vkEndCommandBuffer(commandBuffer);
}

void DemoScene::DrawRange(int startIndex,
                          int endIndex,
                          VkCommandBuffer commandBuffer,
                          VkCommandBufferInheritanceInfo inheritanceInfo) const noexcept
{
	VkCommandBufferBeginInfo commandBufferBeginInfo{};
	commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
	commandBufferBeginInfo.pInheritanceInfo = &inheritanceInfo;

	vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);

	auto extent = G_Application.GetSwapChain().GetExtent();

	VkViewport viewport{};
	viewport.width = static_cast<float>(extent.width);
	viewport.height = static_cast<float>(extent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

	VkRect2D scissor{};
	scissor.extent = extent;
	scissor.offset = VkOffset2D{ 0, 0 };
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipelines.solid);

	for (int i = startIndex; i < endIndex; ++i) {
		auto& material = m_Entities[i]->GetMaterial();
		std::vector<VkDescriptorSet> descriptorSets{
			m_SceneMatricesDescriptorSet,
			material.descriptorSet
		};

		vkCmdBindDescriptorSets(commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			m_PipelineLayout,
			0,
			static_cast<ui32>(descriptorSets.size()),
			descriptorSets.data(),
			0,
			nullptr);

		const auto& xform = m_Entities[i]->GetXform();

		vkCmdPushConstants(commandBuffer,
			m_PipelineLayout,
			VK_SHADER_STAGE_VERTEX_BIT,
			0,
			sizeof(Mat4f),
			&xform);

		std::array<Vec4f, 2> materialProperties{ material.diffuse, material.specular };

		vkCmdPushConstants(commandBuffer,
			m_PipelineLayout,
			VK_SHADER_STAGE_FRAGMENT_BIT,
			sizeof(Mat4f),
			2 * sizeof(Vec4f),
			materialProperties.data());

		m_Entities[i]->Draw(commandBuffer);
	}

	vkEndCommandBuffer(commandBuffer);
}
