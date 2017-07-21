#include <iostream>
#include <Vulkan/vulkan_shader.h>
#include "vulkan_single_threaded_application.h"
#include "logger.h"

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
	rasterizationState.frontFace = VK_FRONT_FACE_CLOCKWISE;
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

	for (i32 i = 0; i < 4; ++i) {
		colorBlendState.blendConstants[i] = 0.0f;
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
	VulkanShader* vertexShader{ GetResource<VulkanShader>("sdr/default.vert.spv", GetDevice()) };

	VulkanShader* fragmentShader{ GetResource<VulkanShader>("sdr/default.frag.spv", GetDevice()) };

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

	//TODO: this is temporary. Create it properly.
	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 0; // Optional
	pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
	pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
	pipelineLayoutInfo.pPushConstantRanges = 0; // Optional

	if (vkCreatePipelineLayout(GetDevice(), &pipelineLayoutInfo, nullptr, &m_PipelineLayout) != VK_SUCCESS) {
		ERROR_LOG("Failed to create pipeline layout.");
		return false;
	}

	VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
	pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineCreateInfo.renderPass = GetRenderPass();
	pipelineCreateInfo.layout = m_PipelineLayout;
	pipelineCreateInfo.flags = VK_NULL_HANDLE;
	pipelineCreateInfo.basePipelineIndex = -1;
	pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;

	//TODO: This is temporary. Create it properly.
	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 0;
	vertexInputInfo.pVertexBindingDescriptions = nullptr; // Optional
	vertexInputInfo.vertexAttributeDescriptionCount = 0;
	vertexInputInfo.pVertexAttributeDescriptions = nullptr; // Optional

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

	// Wire frame rendering pipeline
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
	clearValues[0].color = { 0.3f, 0.3f, 0.3f, 1.0f };
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

//		vkCmdBindDescriptorSets(commandBuffers[i],
//		                        VK_PIPELINE_BIND_POINT_GRAPHICS,
//		                        m_PipelineLayout,
//		                        0,
//		                        1,                //Descriptor set count.
//		                        &m_DescriptorSet, //The descriptor set
//		                        0,                //No Dynamic offsets.
//		                        nullptr);         //No Dynamic offsets.

		vkCmdDraw(commandBuffers[i], 3, 1, 0, 0);
		//TODO: Draw commands here.

		vkCmdEndRenderPass(commandBuffers[i]);

		result = vkEndCommandBuffer(commandBuffers[i]);

		if (result != VK_SUCCESS) {
			ERROR_LOG("Failed to end command buffer.");
			return false;
		}
	}

	return true;
}

VulkanSingleThreadedApplication::VulkanSingleThreadedApplication(const ApplicationSettings& settings)
		: VulkanApplication{ settings }
{
}

VulkanSingleThreadedApplication::~VulkanSingleThreadedApplication()
{
	vkDestroyPipeline(GetDevice(), m_Pipeline, nullptr);

	vkDestroyPipeline(GetDevice(), m_WireframePipeline, nullptr);

	vkDestroyPipelineLayout(GetDevice(), m_PipelineLayout, nullptr);
}

bool VulkanSingleThreadedApplication::Initialize() noexcept
{
	if (!VulkanApplication::Initialize()) {
		return false;
	}

	if (!CreatePipelines()) {
		return false;
	}

	return BuildCommandBuffers();
}

void VulkanSingleThreadedApplication::Draw() noexcept
{
	ui32 imageIndex;

	const auto& swapChain = GetSwapChain();

	VkResult result{ swapChain.GetNextImageIndex(GetPresentCompleteSemaphore(),
	                                             imageIndex) };

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
		//recreate_swapchain();
		return;
	}

	if (result != VK_SUCCESS) {
		ERROR_LOG("Failed to acquire swap chain image.");
		return;
	}

	auto& submitInfo = GetSubmitInfo();
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &GetCommandBuffers()[imageIndex];

	//TODO: Fix this. Graphics queue is NULL.
	result = vkQueueSubmit(GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);

	if (result != VK_SUCCESS) {
		ERROR_LOG("Failed to submit the command buffer.");
		return;
	}

	result = GetSwapChain().Present(GetGraphicsQueue(), imageIndex, GetDrawCompleteSemaphore());
}

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
