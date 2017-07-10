#include "vulkan_application.h"
#include "logger.h"

// Private functions -------------------------------
VkResult VulkanApplication::CreateInstance() noexcept
{
	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = GetSettings().name.c_str();
	appInfo.pEngineName = GetSettings().name.c_str();

	auto instanceExtensions = m_Window->GetExtensions();

	VkInstanceCreateInfo instanceCreateInfo{};
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.pApplicationInfo = &appInfo;

	if (!instanceExtensions.empty()) {
#if !NDEBUG
		instanceExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
#endif
		instanceCreateInfo.enabledExtensionCount = static_cast<ui32>(instanceExtensions.size());
		instanceCreateInfo.ppEnabledExtensionNames = instanceExtensions.data();
	}

#if !NDEBUG
	std::vector<const char *> layers{ "VK_LAYER_LUNARG_standard_validation" };
	instanceCreateInfo.enabledLayerCount = static_cast<ui32>(layers.size());
	instanceCreateInfo.ppEnabledLayerNames = layers.data();
#endif

	return vkCreateInstance(&instanceCreateInfo, nullptr, &m_Instance);
}

bool VulkanApplication::CreateCommandPool() noexcept
{
	VkCommandPoolCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	createInfo.queueFamilyIndex = m_SwapChain.GetQueueIndex();
	createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	VkResult result{ vkCreateCommandPool(m_Device, &createInfo, nullptr, &m_CommandPool) };

	if (result != VK_SUCCESS) {
		ERROR_LOG("Failed to create command pool.");
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

bool VulkanApplication::SetupDepthStencil() noexcept
{
	m_DepthStencil.logicalDevice = m_Device;

	VkImageCreateInfo imageCreateInfo{};
	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.format = m_DepthBufferFormat;
	imageCreateInfo.extent = VkExtent3D{ static_cast<ui32>(m_Window->size.x), static_cast<ui32>(m_Window->size.y), 1 };
	imageCreateInfo.mipLevels = 1;
	imageCreateInfo.arrayLayers = 1;
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	imageCreateInfo.flags = 0;


	VkImageViewCreateInfo depthStencilView{};
	depthStencilView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	depthStencilView.viewType = VK_IMAGE_VIEW_TYPE_2D;
	depthStencilView.format = m_DepthBufferFormat;
	depthStencilView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
	depthStencilView.subresourceRange.baseMipLevel = 0;
	depthStencilView.subresourceRange.levelCount = 1;
	depthStencilView.subresourceRange.baseArrayLayer = 0;
	depthStencilView.subresourceRange.layerCount = 1;

	VkResult result{ vkCreateImage(m_Device, &imageCreateInfo, nullptr, &m_DepthStencil.image) };

	if (result != VK_SUCCESS) {
		ERROR_LOG("Failed to create depth image.");
		return false;
	}

	VkMemoryRequirements memoryRequirements;
	vkGetImageMemoryRequirements(m_Device, m_DepthStencil.image, &memoryRequirements);

	VkMemoryAllocateInfo memoryAllocateInfo{};
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.allocationSize = memoryRequirements.size;
	memoryAllocateInfo.memoryTypeIndex = m_Device.GetMemoryTypeIndex(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	
	result = vkAllocateMemory(m_Device, &memoryAllocateInfo, nullptr, &m_DepthStencil.memory);

	if (result != VK_SUCCESS) {
		ERROR_LOG("Failed to allocate depth stencil memory.");
		return false;
	}

	result = vkBindImageMemory(m_Device, m_DepthStencil.image, m_DepthStencil.memory, 0);

	if (result != VK_SUCCESS) {
		ERROR_LOG("Failed to bind depth image memory.");
		return false;
	}

	depthStencilView.image = m_DepthStencil.image;

	result = vkCreateImageView(m_Device, &depthStencilView, nullptr, &m_DepthStencil.imageView);

	if (result != VK_SUCCESS) {
		ERROR_LOG("Failed to create depth image view.");
		return false;
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

}

const std::unique_ptr<VulkanWindow>& VulkanApplication::GetWindow() const noexcept
{
	return m_Window;
}

const VulkanDevice& VulkanApplication::GetDevice() const noexcept
{
	return m_Device;
}

VkPhysicalDeviceFeatures& VulkanApplication::GetFeaturesToEnable() noexcept
{
	return m_FeaturesToEnable;
}

bool VulkanApplication::Initialize() noexcept
{
	auto settings = GetSettings();
	m_Window = std::make_unique<VulkanWindow>(settings.name,
	                                          settings.windowResolution,
	                                          settings.windowPosition,
	                                          this);

	VkResult result{ CreateInstance() };

	if (result != VK_SUCCESS) {
		return false;
	}

#if !defined(NDEBUG) && !defined(__APPLE__)
	if(!m_VulkanDebug.Initialize(m_Instance)) {
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

	VkSemaphoreCreateInfo semaphoreCreateInfo{};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	result = vkCreateSemaphore(m_Device, &semaphoreCreateInfo, nullptr, &m_PresentComplete);

	if (result != VK_SUCCESS) {
		ERROR_LOG("Failed to create 'present complete' semaphore");
		return false;
	}

	result = vkCreateSemaphore(m_Device, &semaphoreCreateInfo, nullptr, &m_DrawComplete);

	if (result != VK_SUCCESS) {
		ERROR_LOG("Failed to create 'draw complete' semaphore");
		return false;
	}

	m_SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	m_SubmitInfo.pWaitDstStageMask = &m_PipelineStageFlags;
	m_SubmitInfo.waitSemaphoreCount = 1;
	m_SubmitInfo.pWaitSemaphores = &m_PresentComplete;
	m_SubmitInfo.signalSemaphoreCount = 1;
	m_SubmitInfo.pSignalSemaphores = &m_DrawComplete;

	if (!CreateCommandPool()) {
		return false;
	}

	if (!CreateCommandBuffers()){
		return false;
	}

	if (!SetupDepthStencil()) {
		return false;
	}

	//TODO: Implement these.
//	setupRenderPass();
//	createPipelineCache();
//	setupFrameBuffer();

	return true;
}

i32 VulkanApplication::Run() const noexcept
{
	while (!glfwWindowShouldClose(*m_Window)) {
		glfwPollEvents();

		Draw();
	}

	return 0;
}
