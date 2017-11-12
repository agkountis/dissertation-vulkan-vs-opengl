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
	if (!BuildDeferredPassCommandBuffer()) {
		ERROR_LOG("Failed to build deferred command buffer.");
		return false;
	}

	//	if (!BuildDisplayCommandBuffer()) {
	//		ERROR_LOG("Failed to build display command buffer.");
	//		return false;
	//	}

	return true;
}

bool DemoApplication::BuildDeferredPassCommandBuffer()
{
	//TODO
	std::array<VkClearValue, 4> clearValues;
	for (auto i = 0; i < clearValues.size(); ++i) {
		if (i < 3) {
			clearValues[i].color = VkClearColorValue{ 1.0f, 0.0f, 0.0f, 0.0f };
		}
		else {
			clearValues[i].depthStencil = VkClearDepthStencilValue{ 1, 0 };
		}
	}

	VkRenderPassBeginInfo renderPassBeginInfo{};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.renderPass = m_GBuffer.GetRenderPass();
	renderPassBeginInfo.framebuffer = m_GBuffer;

	const auto& size = m_GBuffer.GetSize();
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

	vkCmdSetViewport(m_DeferredCommandBuffer, 0, 1, &viewport);

	VkRect2D scissor{};
	scissor.extent = renderPassBeginInfo.renderArea.extent;
	scissor.offset.x = 0;
	scissor.offset.y = 0;

	vkCmdSetScissor(m_DeferredCommandBuffer, 0, 1, &scissor);

	// todo: Draw here.
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

	// Command buffers are re-recorded per frame so we do not need a command buffer per framebuffer.
	// Use the first one of the available command buffers already allocated.
	const auto bufferIndex = GetCurrentBufferIndex();

	const auto commandBuffer = GetCommandBuffers()[bufferIndex];

	renderPassBeginInfo.framebuffer = GetFramebuffers()[bufferIndex];

	VkResult result = vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);

	if (result != VK_SUCCESS) {
		ERROR_LOG("Failed to begin command buffer.");
		return false;
	}

	vkCmdResetQueryPool(commandBuffer, queryPools[bufferIndex], 0, 2);

	vkCmdWriteTimestamp(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, queryPools[bufferIndex], 0);

	vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	VkViewport viewport{};
	viewport.width = static_cast<float>(swapChainExtent.width);
	viewport.height = static_cast<float>(swapChainExtent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

	VkRect2D scissor{};
	scissor.extent = swapChainExtent;
	scissor.offset = VkOffset2D{ 0, 0 };
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

	m_DemoScene.Draw(commandBuffer);

	vkCmdEndRenderPass(commandBuffer);

	vkCmdWriteTimestamp(commandBuffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, queryPools[bufferIndex], 1);

	//TODO

	result = vkEndCommandBuffer(commandBuffer);

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

	const auto& attachmentSize = GetSettings().windowResolution;

	m_GBuffer.AddAttachment(
		attachmentSize,
		1,
		VK_FORMAT_R16G16B16A16_SFLOAT,
		AttachmentType::COLOR,
		true
	);

	m_GBuffer.AddAttachment(
		attachmentSize,
		1,
		VK_FORMAT_R16G16B16A16_SFLOAT,
		AttachmentType::COLOR,
		true
	);

	m_GBuffer.AddAttachment(
		attachmentSize,
		1,
		VK_FORMAT_R8G8B8A8_UNORM,
		AttachmentType::COLOR,
		true
	);

	m_GBuffer.AddAttachment(
		attachmentSize,
		1,
		VK_FORMAT_D32_SFLOAT,
		AttachmentType::DEPTH,
		true
	);

	if (!m_GBuffer.Create(attachmentSize)) {
		ERROR_LOG("Failed to create GBuffer!");
		return false;
	}

	if (!m_DemoScene.Initialize(GetSwapChain().GetExtent(), m_GBuffer.GetRenderPass())) {
		return false;
	}

	m_DeferredCommandBuffer = G_VulkanDevice.CreateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);


	if (!m_DeferredSemaphore.Create()) {
		ERROR_LOG("Failed to create semaphore for the deferred render pass.");
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
