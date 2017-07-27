#include <iostream>
#include "vulkan_shader.h"
#include "math_utilities.h"
#include "vulkan_single_threaded_application.h"

// Vulkan clip space has inverted Y and half Z.
const Mat4f clipCorrectionMat{ 1.0f, 0.0f, 0.0f, 0.0f,
                               0.0f, -1.0f, 0.0f, 0.0f,
                               0.0f, 0.0f, 0.5f, 0.0f,
                               0.0f, 0.0f, 0.5f, 1.0f };

const std::vector<Vertex> vertices{
		//front
		{{ -0.5f, -0.5f, 0.5f },  { 0.0f,  0.0f, 1.0f },  { 1.0f,  0.0f, 0.0f },  { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f }},
		{{ 0.5f,  -0.5f, 0.5f },  { 0.0f,  0.0f, 1.0f },  { 1.0f,  0.0f, 0.0f },  { 0.0f, 1.0f, 0.0f }, { 1.0f, 1.0f }},
		{{ 0.5f,  0.5f,  0.5f },  { 0.0f,  0.0f, 1.0f },  { 1.0f,  0.0f, 0.0f },  { 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f }},
		{{ -0.5f, 0.5f,  0.5f },  { 0.0f,  0.0f, 1.0f },  { 1.0f,  0.0f, 0.0f },  { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f }},

		//back
		{{ 0.5f,  -0.5f, -0.5f }, { 0.0f,  0.0f, -1.0f }, { -1.0f, 0.0f, 0.0f },  { 1.0f, 0.0f, 0.0f }, { 0.0,  1.0f }},
		{{ -0.5f, -0.5f, -0.5f }, { 0.0f,  0.0f, -1.0f }, { -1.0f, 0.0f, 0.0f },  { 0.0f, 1.0f, 0.0f }, { 1.0f, 1.0f }},
		{{ 0.5f,  0.5f,  -0.5f }, { 0.0f,  0.0f, -1.0f }, { -1.0f, 0.0f, 0.0f },  { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f }},
		{{ -0.5f, 0.5f,  -0.5f }, { 0.0f,  0.0f, -1.0f }, { -1.0f, 0.0f, 0.0f },  { 1.0f, 1.0f, 1.0f }, { 1.0f, 0.0f }},

		//right
		{{ 0.5f,  -0.5f, 0.5f },  { 1.0f,  0.0f, 0.0f },  { 0.0f,  0.0f, 1.0f },  { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f }},
		{{ 0.5f,  -0.5f, -0.5f }, { 1.0f,  0.0f, 0.0f },  { 0.0f,  0.0f, 1.0f },  { 0.0f, 1.0f, 0.0f }, { 1.0f, 1.0f }},
		{{ 0.5f,  0.5f,  0.5f },  { 1.0f,  0.0f, 0.0f },  { 0.0f,  0.0f, 1.0f },  { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f }},
		{{ 0.5f,  0.5f,  -0.5f }, { 1.0f,  0.0f, 0.0f },  { 0.0f,  0.0f, 1.0f },  { 1.0f, 1.0f, 1.0f }, { 1.0f, 0.0f }},

		//left
		{{ -0.5f, -0.5f, -0.5f }, { -1.0f, 0.0f, 0.0f },  { 0.0f,  0.0f, -1.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f }},
		{{ -0.5f, -0.5f, 0.5f },  { -1.0f, 0.0f, 0.0f },  { 0.0f,  0.0f, -1.0f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 1.0f }},
		{{ -0.5f, 0.5f,  -0.5f }, { -1.0f, 0.0f, 0.0f },  { 0.0f,  0.0f, -1.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f }},
		{{ -0.5f, 0.5f,  0.5f },  { -1.0f, 0.0f, 0.0f },  { 0.0f,  0.0f, -1.0f }, { 1.0f, 1.0f, 1.0f }, { 1.0f, 0.0f }}
};

const std::vector<ui32> indices{
		0, 1, 2, 2, 3, 0, 4, 5, 6, 6, 5, 7, 8, 9, 10, 10, 9, 11, 12, 13, 14, 14, 13, 15
};

//Private functions ---------------------------------------------------------------------------
void VulkanSingleThreadedApplication::EnableFeatures() noexcept
{
	const auto& physicalDevice = GetDevice().GetPhysicalDevice();

	auto& featuresToEnable = GetFeaturesToEnable();
	if (physicalDevice.features.samplerAnisotropy) {
		featuresToEnable.samplerAnisotropy = VK_TRUE;
	}

	if (physicalDevice.features.fillModeNonSolid) {
		featuresToEnable.fillModeNonSolid = VK_TRUE;
	}
}

bool VulkanSingleThreadedApplication::CreateUniforms() noexcept
{
	//First create a descriptor pool to allocate descriptors from.
	//Descriptors a.k.a uniforms

	// For now only a Uniform Buffer is needed (UBO: uniform m_Buffer object)
	VkDescriptorPoolSize uniformBufferPoolSize{};
	uniformBufferPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uniformBufferPoolSize.descriptorCount = 1;

	std::vector<VkDescriptorPoolSize> descriptorPoolSizes{ uniformBufferPoolSize };

	VkDescriptorPoolCreateInfo descriptorPoolCreateInfo{};
	descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolCreateInfo.maxSets = 1;
	descriptorPoolCreateInfo.poolSizeCount = static_cast<ui32>(descriptorPoolSizes.size());
	descriptorPoolCreateInfo.pPoolSizes = descriptorPoolSizes.data();

	const auto& device = GetDevice();

	VkResult result{ vkCreateDescriptorPool(device,
	                                        &descriptorPoolCreateInfo,
	                                        nullptr,
	                                        &m_DescriptorPool) };

	if (result != VK_SUCCESS) {
		ERROR_LOG("Failed to create descriptor pool.");
		return false;
	}

	//Now a descriptor set has to be created, but before that, it's layout
	//has to be defined.
	//First the bindings have to be defined...
	VkDescriptorSetLayoutBinding vertexShaderUbo{};
	vertexShaderUbo.binding = 0; //bind at location 0.
	vertexShaderUbo.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; //It's a uniform m_Buffer
	vertexShaderUbo.descriptorCount = 1; //1 descriptor.
	vertexShaderUbo.stageFlags = VK_SHADER_STAGE_VERTEX_BIT; //bound to the vertex shader stage

	std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings{ vertexShaderUbo };

	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{};
	descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorSetLayoutCreateInfo.bindingCount = static_cast<ui32>(descriptorSetLayoutBindings.size());
	descriptorSetLayoutCreateInfo.pBindings = descriptorSetLayoutBindings.data();

	result = vkCreateDescriptorSetLayout(device,
	                                     &descriptorSetLayoutCreateInfo,
	                                     nullptr,
	                                     &m_DescriptorSetLayout);

	if (result != VK_SUCCESS) {
		ERROR_LOG("Failed to create descriptor set layout.");
		return false;
	}

	//Finally allocate a descriptor set from the descriptor pool.
	VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{};
	descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocateInfo.descriptorPool = m_DescriptorPool; //Allocate from this pool.
	descriptorSetAllocateInfo.descriptorSetCount = 1; //1 descriptor set.
	descriptorSetAllocateInfo.pSetLayouts = &m_DescriptorSetLayout; //With this layout.

	result = vkAllocateDescriptorSets(device, &descriptorSetAllocateInfo, &m_DescriptorSet);

	if (result != VK_SUCCESS) {
		ERROR_LOG("Failed to allocate descriptor set.");
		return false;
	}

	// Create the m_Buffer that will store the uniforms.
	if (!device.CreateBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
	                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
	                         m_Ubo,
	                         sizeof(UniformBufferObject))) {
		ERROR_LOG("Failed to create uniform m_Buffer.");
		return false;
	}

	//Now that the descriptor set is allocated we have to write into it and update the uniforms.
	VkWriteDescriptorSet uniformDescriptorWrite{};
	uniformDescriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	uniformDescriptorWrite.dstSet = m_DescriptorSet;
	uniformDescriptorWrite.dstBinding = 0;
	uniformDescriptorWrite.dstArrayElement = 0;
	uniformDescriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uniformDescriptorWrite.descriptorCount = 1;
	uniformDescriptorWrite.pBufferInfo = &m_Ubo.descriptorBufferInfo;

	std::vector<VkWriteDescriptorSet> writeDescriptorSets{ uniformDescriptorWrite };

	vkUpdateDescriptorSets(device,
	                       static_cast<ui32>(writeDescriptorSets.size()),
	                       writeDescriptorSets.data(),
	                       0,
	                       nullptr);

	return true;
}

bool VulkanSingleThreadedApplication::CreatePipelines() noexcept
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

	for (float& blendConstant : colorBlendState.blendConstants) {
		blendConstant = 0.0f;
	}

	VkPipelineDepthStencilStateCreateInfo depthStencilState{};
	depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencilState.depthTestEnable = VK_TRUE;
	depthStencilState.depthWriteEnable = VK_TRUE;
	depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencilState.depthBoundsTestEnable = VK_FALSE;
	depthStencilState.stencilTestEnable = VK_FALSE;

	VkExtent2D swapChainExtent{ GetSwapChain().GetExtent() };

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
	VulkanShader* vertexShader{ LoadShader("sdr/default.vert.spv") };

	if (!vertexShader) {
		ERROR_LOG("Failed to load vertex shader.");
		return false;
	}

	VulkanShader* fragmentShader{ LoadShader("sdr/default.frag.spv") };

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

	std::vector<VkPipelineShaderStageCreateInfo> shaderStages{ vertexShaderStage,
	                                                           fragmentShaderStage };

	//Create the pipeline layout.
	//Here we specify to the the pipeline if we use an push constants or descriptor sets.
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &m_DescriptorSetLayout;
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	pipelineLayoutInfo.pPushConstantRanges = nullptr;

	if (vkCreatePipelineLayout(GetDevice(), &pipelineLayoutInfo, nullptr, &m_PipelineLayout) != VK_SUCCESS) {
		ERROR_LOG("Failed to create pipeline layout.");
		return false;
	}

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
	pipelineCreateInfo.renderPass = GetRenderPass();
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

	VkResult result{ vkCreateGraphicsPipelines(GetDevice(),
	                                           GetPipelineCache(),
	                                           1,
	                                           &pipelineCreateInfo,
	                                           nullptr,
	                                           &m_Pipeline) };

	if (result != VK_SUCCESS) {
		ERROR_LOG("Failed to create pipeline.");
		return false;
	}

	//Wire frame pipeline
	if (GetDevice().GetPhysicalDevice().features.fillModeNonSolid) {
		rasterizationState.polygonMode = VK_POLYGON_MODE_LINE;
		rasterizationState.lineWidth = 1.0f;
		result = vkCreateGraphicsPipelines(GetDevice(),
		                                   GetPipelineCache(),
		                                   1,
		                                   &pipelineCreateInfo,
		                                   nullptr,
		                                   &m_WireframePipeline);
	}

	if (result != VK_SUCCESS) {
		ERROR_LOG("Failed to create pipeline.");
		return false;
	}

	return true;
}

bool VulkanSingleThreadedApplication::BuildCommandBuffers() noexcept
{
	VkCommandBufferBeginInfo commandBufferBeginInfo{};
	commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	VkClearValue clearValues[2];
	clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
	clearValues[1].depthStencil = { 1.0f, 0 };

	VkExtent2D swapChainExtent{ GetSwapChain().GetExtent() };

	VkRenderPassBeginInfo renderPassBeginInfo{};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.renderPass = GetRenderPass();
	renderPassBeginInfo.renderArea.offset.x = 0;
	renderPassBeginInfo.renderArea.offset.y = 0;
	renderPassBeginInfo.renderArea.extent.width = swapChainExtent.width;
	renderPassBeginInfo.renderArea.extent.height = swapChainExtent.height;
	renderPassBeginInfo.clearValueCount = 2;
	renderPassBeginInfo.pClearValues = clearValues;

	const auto& commandBuffers = GetCommandBuffers();

	const auto& frameBuffers = GetFramebuffers();

	VkResult result;

	for (i32 i = 0; i < commandBuffers.size(); ++i) {

		renderPassBeginInfo.framebuffer = frameBuffers[i];

		result = vkBeginCommandBuffer(commandBuffers[i], &commandBufferBeginInfo);

		if (result != VK_SUCCESS) {
			ERROR_LOG("Failed to begin command buffer.");
			return false;
		}

		vkCmdBeginRenderPass(commandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.width = static_cast<float>(swapChainExtent.width);
		viewport.height = static_cast<float>(swapChainExtent.height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(commandBuffers[i], 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.extent = swapChainExtent;
		scissor.offset = VkOffset2D{ 0, 0 };
		vkCmdSetScissor(commandBuffers[i], 0, 1, &scissor);

		vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline);

		//Bind the uniforms.
		vkCmdBindDescriptorSets(commandBuffers[i],
		                        VK_PIPELINE_BIND_POINT_GRAPHICS,
		                        m_PipelineLayout,
		                        0,
		                        1,                //Descriptor set count.
		                        &m_DescriptorSet, //The descriptor set
		                        0,                //No Dynamic offsets.
		                        nullptr);         //No Dynamic offsets.

		mesh.Draw(commandBuffers[i]);

		vkCmdEndRenderPass(commandBuffers[i]);

		result = vkEndCommandBuffer(commandBuffers[i]);

		if (result != VK_SUCCESS) {
			ERROR_LOG("Failed to end command buffer.");
			return false;
		}
	}

	return true;
}

//---------------------------------------------------------------------------------------------

VulkanSingleThreadedApplication::VulkanSingleThreadedApplication(const ApplicationSettings& settings)
		: VulkanApplication{ settings }
{
}

VulkanSingleThreadedApplication::~VulkanSingleThreadedApplication()
{
	const auto& device = GetDevice();

	//No need to free descriptor sets. They are taken care of by the Vulkan driver.
	//Just destroy the descriptor set layout and the descriptor pool.
	vkDestroyDescriptorSetLayout(device, m_DescriptorSetLayout, nullptr);

	vkDestroyDescriptorPool(device, m_DescriptorPool, nullptr);

	vkDestroyPipeline(device, m_Pipeline, nullptr);

	vkDestroyPipeline(device, m_WireframePipeline, nullptr);

	vkDestroyPipelineLayout(device, m_PipelineLayout, nullptr);
}

bool VulkanSingleThreadedApplication::Initialize() noexcept
{
	if (!VulkanApplication::Initialize()) {
		return false;
	}

	if (!CreateUniforms()) {
		return false;
	}

	if (!CreatePipelines()) {
		return false;
	}

	mesh.AddVertices(vertices);
	mesh.AddIndices(indices);
	mesh.CreateBuffers(GetDevice());

	return BuildCommandBuffers();
}

void VulkanSingleThreadedApplication::Update() noexcept
{
	using namespace std::chrono;

	static auto startTime = high_resolution_clock::now();

	auto currentTime = high_resolution_clock::now();
	f32 time{ duration_cast<milliseconds>(currentTime - startTime).count() / 1000.0f };

	UniformBufferObject ubo{};
	ubo.model = Translate(Mat4f{}, Vec3f{ 0.0f, 0.0f, -0.5f });
	ubo.model = Rotate(ubo.model, time * ToRadians(15.0f), Vec3f{ 0.0f, 1.0f, 0.0f });
	ubo.view = LookAt(Vec3f{ 0.0f, 0.0f, 2.0f }, Vec3f{}, Vec3f{ 0.0f, 1.0f, 0.0f });

	auto swapChainExtent = GetSwapChain().GetExtent();

	f32 aspect{ static_cast<f32>(swapChainExtent.width) / static_cast<f32>(swapChainExtent.height) };

	ubo.projection = clipCorrectionMat * Perspective(ToRadians(45.0f), aspect, 0.1f, 10.0f);

	ubo.inverseTransposeModelView = Transpose(Inverse(ubo.view * ubo.model));

	m_Ubo.Map(sizeof m_Ubo, 0);

	m_Ubo.Fill(&ubo, sizeof ubo);

	m_Ubo.Unmap();
}

void VulkanSingleThreadedApplication::Draw() noexcept
{
	PreDraw();

	auto& submitInfo = GetSubmitInfo();
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &GetCommandBuffers()[GetCurrentBufferIndex()];

	VkResult result{ vkQueueSubmit(GetDevice().GetQueue(QueueFamily::GRAPHICS),
	                               1,
	                               &submitInfo,
	                               VK_NULL_HANDLE) };

	if (result != VK_SUCCESS) {
		ERROR_LOG("Failed to submit the command m_Buffer.");
		return;
	}

	PostDraw();
}

void VulkanSingleThreadedApplication::OnResize(const Vec2i& size) noexcept
{
	LOG("RESIZE EVENT!");
}
