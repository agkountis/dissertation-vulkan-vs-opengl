#include "demo_application.h"

static int s_EntitiesPerThread{ 0 };

//Private functions ---------------------------------------------------------------------------
void DemoApplication::EnableFeatures() noexcept
{
	const auto& physicalDevice = G_VulkanDevice.GetPhysicalDevice();

	auto& featuresToEnable = GetFeaturesToEnable();
	if (physicalDevice.features.samplerAnisotropy) {
		featuresToEnable.samplerAnisotropy = VK_TRUE;
	}

	if (physicalDevice.features.fillModeNonSolid) {
		featuresToEnable.fillModeNonSolid = VK_TRUE;
	}
}

bool DemoApplication::BuildCommandBuffers() noexcept
{
	VkCommandBufferBeginInfo commandBufferBeginInfo{};
	commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	VkClearValue clearValues[2];
	clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
	clearValues[1].depthStencil = { 1.0f, 0 };

	const auto& swapChainExtent = GetSwapChain().GetExtent();

	VkRenderPassBeginInfo renderPassBeginInfo{};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.renderPass = GetRenderPass();
	renderPassBeginInfo.renderArea.offset.x = 0;
	renderPassBeginInfo.renderArea.offset.y = 0;
	renderPassBeginInfo.renderArea.extent.width = swapChainExtent.width;
	renderPassBeginInfo.renderArea.extent.height = swapChainExtent.height;
	renderPassBeginInfo.clearValueCount = 2;
	renderPassBeginInfo.pClearValues = clearValues;

	renderPassBeginInfo.framebuffer = GetFramebuffers()[GetCurrentBufferIndex()];

	// Use the 1st command buffer of the base class as the primary one.
	const auto& primaryCmdBuffer = GetCommandBuffers()[0];

	auto result = vkBeginCommandBuffer(primaryCmdBuffer, &commandBufferBeginInfo);

	if (result != VK_SUCCESS) {
		ERROR_LOG("Failed to begin command buffer.");
		return false;
	}

	vkCmdResetQueryPool(primaryCmdBuffer, queryPools[GetCurrentBufferIndex()], 0, 2);

	vkCmdWriteTimestamp(primaryCmdBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, queryPools[GetCurrentBufferIndex()], 0);

	vkCmdBeginRenderPass(primaryCmdBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

	VkCommandBufferInheritanceInfo commandBufferInheritanceInfo{};
	commandBufferInheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
	commandBufferInheritanceInfo.renderPass = GetRenderPass();
	commandBufferInheritanceInfo.framebuffer = renderPassBeginInfo.framebuffer;

	for (auto i = 0u; i < m_PerThreadData.size(); ++i) {
		const auto& [start, end] = m_PerThreadData[i].startEndIndices;

		m_ThreadPool.AddTask(i, [=]()
		{
			m_DemoScene.DrawRange(start, end, m_PerThreadData[i].secondaryCommandBuffer,
			                      commandBufferInheritanceInfo);
		});
	}

	m_ThreadPool.Wait();

	for (const auto& threadData : m_PerThreadData) {
		vkCmdExecuteCommands(primaryCmdBuffer, 1, &threadData.secondaryCommandBuffer);
	}

	vkCmdEndRenderPass(primaryCmdBuffer);

	vkCmdWriteTimestamp(primaryCmdBuffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, queryPools[GetCurrentBufferIndex()], 1);

	result = vkEndCommandBuffer(primaryCmdBuffer);

	if (result != VK_SUCCESS) {
		ERROR_LOG("Failed to end command buffer.");
		return false;
	}

	return true;
}

void DemoApplication::DrawUi() const noexcept
{
	VkCommandBufferBeginInfo commandBufferBeginInfo{};
	commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	const auto& swapChainExtent = GetSwapChain().GetExtent();

	VkRenderPassBeginInfo renderPassBeginInfo{};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.renderPass = m_UiRenderPass;
	renderPassBeginInfo.renderArea.offset.x = 0;
	renderPassBeginInfo.renderArea.offset.y = 0;
	renderPassBeginInfo.renderArea.extent.width = swapChainExtent.width;
	renderPassBeginInfo.renderArea.extent.height = swapChainExtent.height;
	renderPassBeginInfo.clearValueCount = 0;
	renderPassBeginInfo.pClearValues = VK_NULL_HANDLE;

	renderPassBeginInfo.framebuffer = GetFramebuffers()[GetCurrentBufferIndex()];

	vkBeginCommandBuffer(m_UiCommandBuffer, &commandBufferBeginInfo);

	vkCmdBeginRenderPass(m_UiCommandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	VkViewport viewport{};
	viewport.x = 0;
	viewport.y = 0;
	viewport.width = swapChainExtent.width;
	viewport.height = swapChainExtent.height;

	vkCmdSetViewport(m_UiCommandBuffer, 0, 1, &viewport);

	VkRect2D scissor{};
	scissor.extent = renderPassBeginInfo.renderArea.extent;
	scissor.offset.x = 0;
	scissor.offset.y = 0;

	vkCmdSetScissor(m_UiCommandBuffer, 0, 1, &scissor);

	m_DemoScene.DrawUi(m_UiCommandBuffer);

	vkCmdEndRenderPass(m_UiCommandBuffer);

	vkEndCommandBuffer(m_UiCommandBuffer);
}

bool DemoApplication::CreateRenderPasses() noexcept
{
	if (!VulkanApplication::CreateRenderPasses()) {
		return false;
	}

	std::vector<VkAttachmentDescription> attachments;

	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = GetSwapChain().GetFormat();
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD; // do not clear
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	attachments.push_back(colorAttachment);

	VkAttachmentDescription depthAttachment{};
	depthAttachment.format = GetDepthStencil().GetFormat();
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD; //do not clear
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	attachments.push_back(depthAttachment);

	VkAttachmentReference colorAttachmentReference{};
	colorAttachmentReference.attachment = 0;
	colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachmentReference{};
	depthAttachmentReference.attachment = 1;
	depthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpassDescription{};
	subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDescription.colorAttachmentCount = 1;
	subpassDescription.pColorAttachments = &colorAttachmentReference;
	subpassDescription.pDepthStencilAttachment = &depthAttachmentReference;
	subpassDescription.inputAttachmentCount = 0;
	subpassDescription.pInputAttachments = nullptr;
	subpassDescription.preserveAttachmentCount = 0;
	subpassDescription.pPreserveAttachments = nullptr;
	subpassDescription.pResolveAttachments = nullptr;

	// Subpass dependencies for layout transitions
	std::array<VkSubpassDependency, 2> dependencies{};

	dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[0].dstSubpass = 0;
	dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	dependencies[1].srcSubpass = 0;
	dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<ui32>(attachments.size());
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpassDescription;
	renderPassInfo.dependencyCount = static_cast<ui32>(dependencies.size());
	renderPassInfo.pDependencies = dependencies.data();

	const auto result = vkCreateRenderPass(G_VulkanDevice,
	                                       &renderPassInfo,
	                                       nullptr,
	                                       &m_UiRenderPass);

	if (result != VK_SUCCESS) {
		ERROR_LOG("Failed to create render pass.");
		return false;
	}

	return true;
}

//---------------------------------------------------------------------------------------------

DemoApplication::DemoApplication(const ApplicationSettings& settings)
	: VulkanApplication{ settings }
{
}

DemoApplication::~DemoApplication()
{
	vkDeviceWaitIdle(G_VulkanDevice);
	for (const auto& threadData : m_PerThreadData) {
		vkDestroyCommandPool(G_VulkanDevice, threadData.commandPool, nullptr);
	}

	vkDestroyRenderPass(G_VulkanDevice, m_UiRenderPass, nullptr);
}

bool DemoApplication::Initialize() noexcept
{
	if (!VulkanApplication::Initialize()) {
		return false;
	}

	m_UiCommandBuffer = G_VulkanDevice.CreateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);

	if (!m_UiSemaphore.Create()) {
		ERROR_LOG("Failed to create semaphore for UI render pass.");
		return false;
	}

	if (!m_DemoScene.Initialize(GetSwapChain().GetExtent(), GetRenderPass(), m_UiRenderPass)) {
		return false;
	}

	if (!m_ThreadPool.Initialize()) {
		return false;
	}

	s_EntitiesPerThread = ENTITY_COUNT / m_ThreadPool.GetWorkerCount();

	m_PerThreadData.resize(m_ThreadPool.GetWorkerCount());

	const auto gfxQueueIndex = G_VulkanDevice.GetQueueFamilyIndex(QueueFamily::GRAPHICS);
	static auto startIndex = 0;
	for (auto& threadData : m_PerThreadData) {
		// One command pool per thread
		threadData.commandPool = G_VulkanDevice.CreateCommandPool(gfxQueueIndex);

		// One secondary command buffer for each entity range.
		threadData.secondaryCommandBuffer = G_VulkanDevice.CreateCommandBuffer(threadData.commandPool,
		                                                                       VK_COMMAND_BUFFER_LEVEL_SECONDARY);

		const auto endIndex{ startIndex + s_EntitiesPerThread };
		threadData.startEndIndices = std::make_tuple(startIndex, endIndex);

		startIndex = endIndex;
	}

	return true;
}

void DemoApplication::Update() noexcept
{
	m_DemoScene.Update(GetSwapChain().GetExtent(),
	                   GetTimer().GetMsec(),
	                   GetTimer().GetDelta());
}

void DemoApplication::Draw() noexcept
{
	PreDraw();

	BuildCommandBuffers();
	DrawUi();

	auto& submitInfo = GetSubmitInfo();
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &GetCommandBuffers()[0];
	submitInfo.pWaitSemaphores = GetPresentCompleteSemaphore().Get();
	submitInfo.pSignalSemaphores = m_UiSemaphore.Get();

	w1 = GetTimer().GetSec();

	auto result = vkQueueSubmit(G_VulkanDevice.GetQueue(QueueFamily::GRAPHICS),
	                                  1,
	                                  &submitInfo, VK_NULL_HANDLE);

	if (result != VK_SUCCESS) {
		ERROR_LOG("Failed to submit the command buffer.");
		return;
	}

	submitInfo.pCommandBuffers = &m_UiCommandBuffer;
	submitInfo.pWaitSemaphores = m_UiSemaphore.Get();
	submitInfo.pSignalSemaphores = GetDrawCompleteSemaphore().Get();

	result = vkQueueSubmit(G_VulkanDevice.GetQueue(QueueFamily::GRAPHICS),
	                       1,
	                       &submitInfo,
	                       VK_NULL_HANDLE);

	if (result != VK_SUCCESS) {
		ERROR_LOG("Failed to submit the command buffer.");
		return;
	}

	PostDraw();
}

void DemoApplication::OnResize(const Vec2i& size) noexcept
{
	LOG("RESIZE EVENT!");
}
