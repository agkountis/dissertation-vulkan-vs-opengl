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

	renderPassBeginInfo.framebuffer = GetFramebuffers()[GetCurrentBufferIndex()];

	// Use the 1st command buffer of the base class as the primary one.
	const auto& primaryCmdBuffer = GetCommandBuffers()[0];

	VkResult result{ vkBeginCommandBuffer(primaryCmdBuffer, &commandBufferBeginInfo) };

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

	int entityIndex = 0;
	for (int i = 0; i < m_PerThreadData.size(); ++i) {
		for (int j = 0; j < m_PerThreadData[i].secondaryCommandBuffers.size(); ++j) {
			m_ThreadPool.AddTask(i, [=]()
			{
				GetScene().DrawSingle(entityIndex, m_PerThreadData[i].secondaryCommandBuffers[j],
				                      commandBufferInheritanceInfo);
			});

			++entityIndex;
		}
	}

	m_ThreadPool.Wait();

	for (const auto& threadData : m_PerThreadData) {
		vkCmdExecuteCommands(primaryCmdBuffer, threadData.secondaryCommandBuffers.size(),
		                     threadData.secondaryCommandBuffers.data());
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

//---------------------------------------------------------------------------------------------

DemoApplication::DemoApplication(const ApplicationSettings& settings)
	: VulkanApplication{ settings }
{
}

bool DemoApplication::Initialize() noexcept
{
	if (!VulkanApplication::Initialize()) {
		return false;
	}

	if (!m_DemoScene.Initialize(GetSwapChain().GetExtent(), GetRenderPass())) {
		return false;
	}

	if (!m_ThreadPool.Initialize()) {
		return false;
	}

	s_EntitiesPerThread = ENTITY_COUNT / m_ThreadPool.GetWorkerCount();

	m_PerThreadData.resize(m_ThreadPool.GetWorkerCount());

	ui32 gfxQueueIndex{ G_VulkanDevice.GetQueueFamilyIndex(QueueFamily::GRAPHICS) };
	for (auto& threadData : m_PerThreadData) {
		// One command pool per thread
		threadData.commandPool = G_VulkanDevice.CreateCommandPool(gfxQueueIndex);

		// One secondary command buffer for each entity.
		threadData.secondaryCommandBuffers = G_VulkanDevice.CreateCommandBuffers(s_EntitiesPerThread,
		                                                                         threadData.commandPool,
		                                                                         VK_COMMAND_BUFFER_LEVEL_SECONDARY);
	}

	VkFenceCreateInfo fenceCreateInfo{};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

	VkResult result{ vkCreateFence(G_VulkanDevice, &fenceCreateInfo, nullptr, &m_RenderFence) };

	if (result != VK_SUCCESS) {
		ERROR_LOG("Failed to create fence.");
		return false;
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

	auto& submitInfo = GetSubmitInfo();
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &GetCommandBuffers()[0];

	w1 = GetTimer().GetSec();

	VkResult result{
		vkQueueSubmit(G_VulkanDevice.GetQueue(QueueFamily::GRAPHICS),
		              1,
		              &submitInfo, m_RenderFence)
	};

	if (result != VK_SUCCESS) {
		ERROR_LOG("Failed to submit the command buffer.");
		return;
	}

	result = vkWaitForFences(G_VulkanDevice, 1, &m_RenderFence, VK_TRUE, std::numeric_limits<ui64>::max());

	if (result != VK_SUCCESS) {
		ERROR_LOG("Wait for fences failed.");
		return;
	}
	vkResetFences(G_VulkanDevice, 1, &m_RenderFence);

	PostDraw();
}

void DemoApplication::OnResize(const Vec2i& size) noexcept
{
	LOG("RESIZE EVENT!");
}
