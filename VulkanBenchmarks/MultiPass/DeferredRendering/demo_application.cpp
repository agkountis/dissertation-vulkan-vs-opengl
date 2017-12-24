#include "demo_application.h"
#include <vulkan_infrastructure_context.h>

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
	return true;
}


bool DemoApplication::BuildDeferredPassCommandBuffer()
{
	std::array<VkClearValue, 5> clearValues;
	for (auto i = 0; i < clearValues.size(); ++i) {
		if (i < 4) {
			clearValues[i].color = VkClearColorValue{ 0.0f, 0.0f, 0.0f, 0.0f };
		}
		else {
			clearValues[i].depthStencil = VkClearDepthStencilValue{ 1.0, 0 };
		}
	}

	VkRenderPassBeginInfo renderPassBeginInfo{};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.renderPass = m_DemoScene.GetGBuffer().GetRenderPass();
	renderPassBeginInfo.framebuffer = m_DemoScene.GetGBuffer();

	const auto& size = m_DemoScene.GetGBuffer().GetSize();
	renderPassBeginInfo.renderArea.extent = VkExtent2D{ size.x, size.y };
	renderPassBeginInfo.clearValueCount = static_cast<ui32>(clearValues.size());
	renderPassBeginInfo.pClearValues = clearValues.data();

	VkCommandBufferBeginInfo commandBufferBeginInfo{};
	commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	VkResult result{ vkBeginCommandBuffer(m_DeferredCommandBuffer, &commandBufferBeginInfo) };

	if (result != VK_SUCCESS) {
		ERROR_LOG("Failed to begin command buffer.");
		return false;
	}

	vkCmdBeginRenderPass(m_DeferredCommandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	VkViewport viewport{};
	viewport.x = 0;
	viewport.y = 0;
	viewport.width = size.x;
	viewport.height = size.y;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	vkCmdSetViewport(m_DeferredCommandBuffer, 0, 1, &viewport);

	VkRect2D scissor{};
	scissor.extent = renderPassBeginInfo.renderArea.extent;
	scissor.offset.x = 0;
	scissor.offset.y = 0;

	vkCmdSetScissor(m_DeferredCommandBuffer, 0, 1, &scissor);

	m_DemoScene.Draw(m_DeferredCommandBuffer);

	vkCmdEndRenderPass(m_DeferredCommandBuffer);

	result = vkEndCommandBuffer(m_DeferredCommandBuffer);

	if (result != VK_SUCCESS) {
		ERROR_LOG("Failed to end command buffer.");
		return false;
	}

	return true;
}

bool DemoApplication::BuildDisplayCommandBuffer()
{
	VkCommandBufferBeginInfo commandBufferBeginInfo{};
	commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	VkClearValue clearValues[2];
	clearValues[0].color = { 0.0f, 0.0f, 0.0f, 0.0f };
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
	for (auto i = 0; i < commandBuffers.size(); ++i) {

		const auto commandBuffer = commandBuffers[i];
		renderPassBeginInfo.framebuffer = GetFramebuffers()[i];

		VkResult result = vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);

		if (result != VK_SUCCESS) {
			ERROR_LOG("Failed to begin command buffer.");
			return false;
		}

		vkCmdResetQueryPool(commandBuffer, queryPools[i], 0, 2);

		vkCmdWriteTimestamp(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, queryPools[i], 0);

		vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.x = 0;
		viewport.y = 0;
		viewport.width = swapChainExtent.width;
		viewport.height = swapChainExtent.height;

		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.extent = renderPassBeginInfo.renderArea.extent;
		scissor.offset.x = 0;
		scissor.offset.y = 0;

		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		m_DemoScene.DrawFullscreenQuad(commandBuffer);

		vkCmdEndRenderPass(commandBuffer);

		vkCmdWriteTimestamp(commandBuffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, queryPools[i], 1);

		result = vkEndCommandBuffer(commandBuffer);

		if (result != VK_SUCCESS) {
			ERROR_LOG("Failed to end command buffer.");
			return false;
		}
	}

	return true;
}

//---------------------------------------------------------------------------------------------

DemoApplication::DemoApplication(const ApplicationSettings& settings)
	: VulkanApplication{ settings }
{
}

DemoApplication::~DemoApplication() noexcept
{
	vkDeviceWaitIdle(G_VulkanDevice);
}


bool DemoApplication::Initialize() noexcept
{
	if (!VulkanApplication::Initialize()) {
		return false;
	}

	if (!m_DemoScene.Initialize(GetSwapChain().GetExtent(), GetRenderPass())) {
		return false;
	}

	m_DeferredCommandBuffer = G_VulkanDevice.CreateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);

	if (!m_DeferredSemaphore.Create()) {
		ERROR_LOG("Failed to create semaphore for the deferred render pass.");
		return false;
	}

	if (!BuildDeferredPassCommandBuffer()) {
		ERROR_LOG("Failed to build deferred command buffer.");
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

	if (!BuildDisplayCommandBuffer()) {
		ERROR_LOG("Failed to build display command buffer.");
		return;
	}

	auto& submitInfo = GetSubmitInfo();
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &m_DeferredCommandBuffer;
	submitInfo.pWaitSemaphores = GetPresentCompleteSemaphore().Get();
	submitInfo.pSignalSemaphores = m_DeferredSemaphore.Get();

	w1 = GetTimer().GetSec();

	VkResult result{
		vkQueueSubmit(G_VulkanDevice.GetQueue(QueueFamily::GRAPHICS),
		              1,
		              &submitInfo,
		              VK_NULL_HANDLE)
	};

	if (result != VK_SUCCESS) {
		ERROR_LOG("Failed to submit the command buffer.");
		return;
	}

	submitInfo.pCommandBuffers = &GetCommandBuffers()[GetCurrentBufferIndex()];
	submitInfo.pWaitSemaphores = m_DeferredSemaphore.Get();
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
