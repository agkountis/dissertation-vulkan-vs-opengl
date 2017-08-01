#include "vulkan_application.h"
#include "vulkan_infrastructure_context.h"
#include <array>

// Private functions -------------------------------
bool VulkanApplication::CreateInstance() noexcept
{
	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = GetSettings().name.c_str();
	appInfo.pEngineName = GetSettings().name.c_str();
	appInfo.apiVersion = VK_MAKE_VERSION(1, 0, 54);

	auto instanceExtensions = m_Window.GetExtensions();

#if !NDEBUG
	instanceExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
#endif

	std::vector<const char*> layers;
#if !NDEBUG
	layers.push_back("VK_LAYER_LUNARG_standard_validation");

	uint32_t layerCount{ 0 };
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	LOG("|- AVAILABLE VULKAN LAYERS:");
	for (auto& availableLayer : availableLayers) {
		LOG("\t |- " + std::string{ availableLayer.layerName });
	}
#endif

	return G_VulkanInstance.Create(appInfo, instanceExtensions, layers);
}

bool VulkanApplication::CreateCommandBuffers() noexcept
{
	m_DrawCommandBuffers.resize(m_SwapChain.GetImages().size());

	VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.commandPool = m_CommandPool;
	commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferAllocateInfo.commandBufferCount = static_cast<ui32>(m_DrawCommandBuffers.size());

	VkResult result{ vkAllocateCommandBuffers(G_VulkanDevice,
	                                          &commandBufferAllocateInfo,
	                                          m_DrawCommandBuffers.data()) };

	if (result != VK_SUCCESS) {
		ERROR_LOG("Failed to allocate command buffers.");
		return false;
	}

	return true;
}

bool VulkanApplication::CreateRenderPasses() noexcept
{
	std::vector<VkAttachmentDescription> attachments;

	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = m_SwapChain.GetFormat();
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	attachments.push_back(colorAttachment);

	VkAttachmentDescription depthAttachment{};
	depthAttachment.format = m_DepthStencil.GetFormat();
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
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

	VkResult result{ vkCreateRenderPass(G_VulkanDevice,
	                                    &renderPassInfo,
	                                    nullptr,
	                                    &m_RenderPass) };

	if (result != VK_SUCCESS) {
		ERROR_LOG("Failed to create render pass.");
		return false;
	}

	return true;
}

bool VulkanApplication::CreateFramebuffers() noexcept
{
	m_SwapChainFrameBuffers.resize(m_SwapChain.GetImages().size());

	const auto& swapChainImageViews = m_SwapChain.GetImageViews();

	auto swapChainExtent = m_SwapChain.GetExtent();

	for (int i = 0; i < swapChainImageViews.size(); ++i) {
		std::vector<VkImageView> attachments{ swapChainImageViews[i], m_DepthStencil.GetImageView() };

		if (!m_SwapChainFrameBuffers[i].Create(attachments,
		                                       Vec2ui{ swapChainExtent.width, swapChainExtent.height },
		                                       m_RenderPass)) {
			return false;
		}
	}

	return true;
}

// -------------------------------------------------

VulkanApplication::VulkanApplication(const ApplicationSettings& settings)
		: Application{ settings }
{
}

VulkanApplication::~VulkanApplication()
{
	vkDestroyRenderPass(G_VulkanDevice, m_RenderPass, nullptr);
}

VulkanWindow& VulkanApplication::GetWindow() noexcept
{
	return m_Window;
}

VkPhysicalDeviceFeatures& VulkanApplication::GetFeaturesToEnable() noexcept
{
	return m_FeaturesToEnable;
}

const VulkanSwapChain& VulkanApplication::GetSwapChain() const noexcept
{
	return m_SwapChain;
}

const std::vector<VkCommandBuffer>& VulkanApplication::GetCommandBuffers() const noexcept
{
	return m_DrawCommandBuffers;
}

const std::vector<VulkanFramebuffer>& VulkanApplication::GetFramebuffers() const noexcept
{
	return m_SwapChainFrameBuffers;
}

const VulkanPipelineCache& VulkanApplication::GetPipelineCache() const noexcept
{
	return m_PipelineCache;
}

VkRenderPass VulkanApplication::GetRenderPass() const noexcept
{
	return m_RenderPass;
}

VkSubmitInfo& VulkanApplication::GetSubmitInfo() noexcept
{
	return m_SubmitInfo;
}

ui32 VulkanApplication::GetCurrentBufferIndex() const noexcept
{
	return m_CurrentBuffer;
}

bool VulkanApplication::Reshape(const Vec2ui& size) noexcept
{
	m_SwapChain.Create(size, GetSettings().vsync);

	m_DepthStencil.Destroy();

	VkExtent2D extent = m_SwapChain.GetExtent();
	if (!m_DepthStencil.Create(Vec2ui{extent.width, extent.height},
	                           G_VulkanDevice.GetPhysicalDevice().GetSupportedDepthFormat())) {
		return false;
	}

	for (ui32 i = 0; i < m_SwapChainFrameBuffers.size(); ++i) {
		m_SwapChainFrameBuffers[i].Destroy();

		std::vector<VkImageView> imageViews{ m_SwapChain.GetImageViews()[i],
		                                     m_DepthStencil.GetImageView() };

		if (!m_SwapChainFrameBuffers[i].Create(imageViews,
		                                       Vec2ui{extent.width, extent.height},
		                                       m_RenderPass)) {
			return false;
		}
	}

	vkFreeCommandBuffers(G_VulkanDevice,
	                     m_CommandPool,
	                     static_cast<ui32>(m_DrawCommandBuffers.size()),
	                     m_DrawCommandBuffers.data());

	m_DrawCommandBuffers.clear();

	if (!CreateCommandBuffers()) {
		return false;
	}

	if (!BuildCommandBuffers()) {
		return false;
	}

	vkDeviceWaitIdle(G_VulkanDevice);

	OnResize(Vec2ui{extent.width, extent.height});

	return true;
}

bool VulkanApplication::Initialize() noexcept
{
	VulkanInfrastructureContext::RegisterApplication(this);

	const auto& settings = GetSettings();
	if (!m_Window.Create(settings.name,
	                     settings.windowResolution,
	                     settings.windowPosition,
	                     this)) {
		return false;
	}

	if (!CreateInstance()) {
		return false;
	}

#if !defined(NDEBUG) && !defined(__APPLE__)
	if (!G_VulkanDebug.Initialize()) {
		return false;
	}
#endif

	if (!G_VulkanDevice.Initialize(G_VulkanInstance)) {
		return false;
	}

	EnableFeatures();

	if (!G_VulkanDevice.CreateLogicalDevice(m_FeaturesToEnable, m_ExtensionsToEnable)) {
		return false;
	}

	if (!m_SwapChain.Initialize(m_Window)) {
		return false;
	}

	if (!m_SwapChain.Create(settings.windowResolution, settings.vsync)) {
		return false;
	}

	VkFormat depthStencilFormat{ G_VulkanDevice.GetPhysicalDevice().GetSupportedDepthFormat() };

	if (depthStencilFormat == VK_FORMAT_UNDEFINED) {
		ERROR_LOG("Could not find supported depth format.");
		return false;
	}

	if (!m_DepthStencil.Create(m_Window.GetSize(), depthStencilFormat)) {
		return false;
	}

	if (!m_PresentComplete.Create()) {
		return false;
	}

	if (!m_DrawComplete.Create()) {
		return false;
	}

	m_SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	m_SubmitInfo.pWaitDstStageMask = &m_PipelineStageFlags;
	m_SubmitInfo.waitSemaphoreCount = 1;
	m_SubmitInfo.pWaitSemaphores = m_PresentComplete.Get();
	m_SubmitInfo.signalSemaphoreCount = 1;
	m_SubmitInfo.pSignalSemaphores = m_DrawComplete.Get();

	if (!m_CommandPool.Create(m_SwapChain.GetQueueIndex(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT)) {
		return false;
	}

	if (!CreateCommandBuffers()) {
		return false;
	}

	if (!m_PipelineCache.Create()) {
		return false;
	}

	if (!CreateRenderPasses()) {
		return false;
	}

	return CreateFramebuffers();
}

i32 VulkanApplication::Run() noexcept
{
	while (!glfwWindowShouldClose(m_Window)) {
		glfwPollEvents();

		Update();
		Draw();
	}

	return 0;
}

void VulkanApplication::PreDraw() noexcept
{
	VkResult result{ m_SwapChain.GetNextImageIndex(m_PresentComplete,
	                                               m_CurrentBuffer) };

	while(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
		const auto& extent = m_SwapChain.GetExtent();
		Reshape(Vec2i{extent.width, extent.height});

		result = m_SwapChain.GetNextImageIndex(m_PresentComplete,
		                                       m_CurrentBuffer);
	}
}

void VulkanApplication::PostDraw() noexcept
{
	VkResult result{ m_SwapChain.Present(G_VulkanDevice.GetQueue(QueueFamily::PRESENT),
	                                     m_CurrentBuffer,
	                                     m_DrawComplete) };

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
		const auto& extent = m_SwapChain.GetExtent();
		Reshape(Vec2i{extent.width, extent.height});
	}

	vkQueueWaitIdle(G_VulkanDevice.GetQueue(QueueFamily::PRESENT));
}
