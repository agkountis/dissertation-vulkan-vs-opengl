#include "vulkan_single_threaded_application.h"
#include <vector>
#include "vulkan_utilities.h"
#include "logger.h"

using namespace VulkanUtilities;

VkPhysicalDevice VulkanSingleThreadedApplication::PickPhysicalDevice(VkInstance instance, VkSurfaceKHR surface, const std::vector<std::string>& requiredExtensions) const noexcept
{
	ui32 deviceCount{ 0 };

	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

	std::vector<VkPhysicalDevice> physicalDevices{ deviceCount };

	vkEnumeratePhysicalDevices(instance, &deviceCount, physicalDevices.data());

	for (const auto& physicalDevice : physicalDevices) {
		if (DeviceIsSuitable(physicalDevice, surface, requiredExtensions)) {
			return physicalDevice;
		}
	}

	return nullptr;
}

bool VulkanSingleThreadedApplication::Initialize()
{
	m_Window = std::make_unique<VulkanWindow>("Vulkan single-threaded benchmark",
	                                          Vec2i{ 1024, 768 },
	                                          Vec2i{ 0, 0 },
	                                          this);

	VkApplicationInfo applicationInfo{};
	applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	applicationInfo.pApplicationName = "Vulkan single-threaded benchmark";
	applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	applicationInfo.pEngineName = "No Engine";
	applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);

	std::vector<const char*> extensions{ m_Window->GetExtensions() };

	std::vector<const char*> validationLayers;

#if !NDEBUG
	extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
	validationLayers.push_back("VK_LAYER_LUNARG_standard_validation");
#endif

	try {
		m_Instance = std::make_unique<VulkanInstance>(applicationInfo, extensions, validationLayers);
	}
	catch(std::runtime_error runtimeError) {
		ERROR_LOG(runtimeError.what());
		return false;
	}

#if !NDEBUG
	try {
		m_VulkanDebug = std::make_unique<VulkanDebug>(m_Instance);
	}
	catch(std::runtime_error runtimeError) {
		ERROR_LOG(runtimeError.what());
		return false;
	}
	
#endif

	if (!m_Window->CreateSurface(m_Instance)) {
		return false;
	}

	return true;
}

i32 VulkanSingleThreadedApplication::Run()
{
	return m_Window->MainLoop();
}

void VulkanSingleThreadedApplication::Draw() noexcept
{
}
