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
	//TODO: Enable layers here.

	return vkCreateInstance(&instanceCreateInfo, nullptr, &m_Instance);
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

	//TODO: This function call is not implemented.
	if (!m_Device.CreateLogicalDevice(m_FeaturesToEnable, m_ExtensionsToEnable)) {
		return false;
	}

	if (!m_SwapChain.Initialize(m_Instance, m_Device.GetPhysicalDevice(), nullptr, m_Window)) {
		return false;
	}

	if (!m_SwapChain.Create(settings.windowResolution, settings.vsync)) {
		return false;
	}

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
