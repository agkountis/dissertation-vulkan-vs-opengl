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

		vkCmdResetQueryPool(commandBuffers[i], queryPools[i], 0, 2);

		vkCmdWriteTimestamp(commandBuffers[i], VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, queryPools[i], 0);

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

		m_DemoScene.Draw(commandBuffers[i]);

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

	return BuildCommandBuffers();
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

	auto& submitInfo = GetSubmitInfo();
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &GetCommandBuffers()[GetCurrentBufferIndex()];

	w1 = GetTimer().GetSec();

	VkResult result{ vkQueueSubmit(G_VulkanDevice.GetQueue(QueueFamily::GRAPHICS),
	                               1,
	                               &submitInfo,
	                               VK_NULL_HANDLE) };

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
