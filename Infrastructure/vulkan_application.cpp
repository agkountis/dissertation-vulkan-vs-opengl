#include "vulkan_application.h"
#include "logger.h"
#include <array>

// Private functions -------------------------------
bool VulkanApplication::CreateInstance() noexcept
{
	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = GetSettings().name.c_str();
	appInfo.pEngineName = GetSettings().name.c_str();

	auto instanceExtensions = m_Window.GetExtensions();

#if !NDEBUG
	instanceExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
#endif

#if !NDEBUG
	std::vector<const char *> layers{ "VK_LAYER_LUNARG_standard_validation" };
#endif

	if (!m_Instance.Create(appInfo, instanceExtensions, layers)) {
		return false;
	}

	return true;
}

bool VulkanApplication::CreateCommandBuffers() noexcept
{
	m_DrawCommandBuffers.resize(m_SwapChain.GetImages().size());

	VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.commandPool = m_CommandPool;
	commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferAllocateInfo.commandBufferCount = static_cast<ui32>(m_DrawCommandBuffers.size());

	VkResult result{ vkAllocateCommandBuffers(m_Device, &commandBufferAllocateInfo, m_DrawCommandBuffers.data()) };

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
	depthAttachment.format = m_DepthBufferFormat;
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

	VkSubpassDescription subpassDescription = {};
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
	std::array<VkSubpassDependency, 2> dependencies;

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

	VkResult result{ vkCreateRenderPass(GetDevice(), &renderPassInfo, nullptr, &m_RenderPass) };

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

	auto swapchainExtent = m_SwapChain.GetExtent();

	for (int i = 0; i < swapChainImageViews.size(); ++i) {
		std::vector<VkImageView> attachments{ swapChainImageViews[i], m_DepthStencil.GetImageView() };

		if (!m_SwapChainFrameBuffers[i].Create(m_Device,
		                                       attachments,
		                                       Vec2ui{ swapchainExtent.width, swapchainExtent.height },
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
	vkDestroyRenderPass(m_Device, m_RenderPass, nullptr);
}

VulkanWindow& VulkanApplication::GetWindow() noexcept
{
	return m_Window;
}

const VulkanInstance& VulkanApplication::GetVulkanInstance() const noexcept
{
	return m_Instance;
}

const VulkanDevice& VulkanApplication::GetDevice() const noexcept
{
	return m_Device;
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

VkQueue VulkanApplication::GetGraphicsQueue() const noexcept
{
	return m_GraphicsQueue;
}

VkFormat VulkanApplication::GetDepthBufferFormat() const noexcept
{
	return m_DepthBufferFormat;
}

bool VulkanApplication::Initialize() noexcept
{
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
	if (!m_VulkanDebug.Initialize(m_Instance)) {
		return false;
	}
#endif

	if (!m_Device.Initialize(m_Instance)) {
		return false;
	}

	EnableFeatures();

	if (!m_Device.CreateLogicalDevice(m_FeaturesToEnable, m_ExtensionsToEnable)) {
		return false;
	}

	if (!m_SwapChain.Initialize(m_Instance, m_Device.GetPhysicalDevice(), m_Device, m_Window)) {
		return false;
	}

	if (!m_SwapChain.Create(settings.windowResolution, settings.vsync)) {
		return false;
	}

	m_DepthBufferFormat = m_Device.GetPhysicalDevice().GetSupportedDepthFormat();

	if (m_DepthBufferFormat == VK_FORMAT_UNDEFINED) {
		ERROR_LOG("Could not find supported depth format.");
		return false;
	}

	if (!m_PresentComplete.Create(m_Device)) {
		return false;
	}

	if (!m_DrawComplete.Create(m_Device)) {
		return false;
	}

	m_SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	m_SubmitInfo.pWaitDstStageMask = &m_PipelineStageFlags;
	m_SubmitInfo.waitSemaphoreCount = 1;
	m_SubmitInfo.pWaitSemaphores = &m_PresentComplete;
	m_SubmitInfo.signalSemaphoreCount = 1;
	m_SubmitInfo.pSignalSemaphores = &m_DrawComplete;

	//TODO: The VulkanDevice class already has a command pool allocated. Do I need 2 of them?
	if (!m_CommandPool.Create(m_Device, m_SwapChain.GetQueueIndex(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT)) {
		return false;
	}

	if (!CreateCommandBuffers()) {
		return false;
	}

	if (!m_DepthStencil.Create(m_Device, m_Window.GetSize(), m_DepthBufferFormat)) {
		return false;
	}

	if (!m_PipelineCache.Create(m_Device)) {
		return false;
	}

	if (!CreateRenderPasses()) {
		return false;
	}

	if (!CreateFramebuffers()) {
		return false;
	}

	return true;
}

i32 VulkanApplication::Run() const noexcept
{
	while (!glfwWindowShouldClose(m_Window)) {
		glfwPollEvents();

		Draw();
	}

	return 0;
}
