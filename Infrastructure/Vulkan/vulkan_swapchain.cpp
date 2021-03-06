#include "vulkan_swapchain.h"
#include "logger.h"
#include "vulkan_window.h"
#include "vulkan_infrastructure_context.h"
#include <algorithm>

// Private functions ------------------------------------------
bool VulkanSwapChain::InitializeSurface(const VulkanWindow& window) noexcept
{
	VkResult result{ glfwCreateWindowSurface(G_VulkanInstance, window, nullptr, &m_Surface) };

	if (result != VK_SUCCESS) {
		ERROR_LOG("Failed to create Vulkan Surface.");
		return false;
	}

	ui32 queueCount{ 0 };
	vkGetPhysicalDeviceQueueFamilyProperties(G_VulkanDevice.GetPhysicalDevice(), &queueCount, nullptr);

	if (!queueCount) {
		ERROR_LOG("No available queue families.");
		return false;
	}

	std::vector<VkQueueFamilyProperties> queueProperties{ queueCount };
	vkGetPhysicalDeviceQueueFamilyProperties(G_VulkanDevice.GetPhysicalDevice(), &queueCount, queueProperties.data());

	// Iterate over each queue to learn whether it supports presenting:
	// Find a queue with present support
	// Will be used to present the swap chain images to the windowing system
	std::vector<VkBool32> supportsPresent;
	supportsPresent.resize(queueCount);
	for (ui32 i = 0; i < queueCount; ++i) {
		vkGetPhysicalDeviceSurfaceSupportKHR(G_VulkanDevice.GetPhysicalDevice(), i, m_Surface, &supportsPresent[i]);
	}

	// Search for a graphics and a present queue in the array of queue
	// families, try to find one that supports both
	ui32 graphicsQueueIndex = std::numeric_limits<ui32>::max();
	ui32 presentQueueIndex = std::numeric_limits<ui32>::max();

	for (ui32 i = 0; i < queueCount; i++) {

		if ((queueProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) {

			if (graphicsQueueIndex == std::numeric_limits<ui32>::max()) {
				graphicsQueueIndex = i;
			}

			if (supportsPresent[i] == VK_TRUE) {
				graphicsQueueIndex = i;
				presentQueueIndex = i;
				break;
			}
		}
	}

	// Exit if either a graphics or a presenting queue hasn't been found
	const auto graphicsQueueIndexInvalid{ graphicsQueueIndex == std::numeric_limits<ui32>::max() };
	const auto presentQueueIndexInvalid{ presentQueueIndex == std::numeric_limits<ui32>::max() };

	if (graphicsQueueIndexInvalid || presentQueueIndexInvalid) {
		ERROR_LOG("Could not find a queue that supports graphics/presenting.");
		return false;
	}

	if (graphicsQueueIndex != presentQueueIndex) {
		ERROR_LOG("Separate graphics and presenting queues are not supported.");
		return false;
	}

	m_GraphicsAndPresentQueueIndex = graphicsQueueIndex;

	// Get list of supported surface formats
	ui32 formatCount{ 0 };
	result = vkGetPhysicalDeviceSurfaceFormatsKHR(G_VulkanDevice.GetPhysicalDevice(),
	                                              m_Surface,
	                                              &formatCount,
	                                              nullptr);

	if (result != VK_SUCCESS) {
		ERROR_LOG("Could not get the surface format count.");
		return false;
	}

	if (!formatCount) {
		ERROR_LOG("No available surface formats.");
		return false;
	}

	std::vector<VkSurfaceFormatKHR> surfaceFormats{ formatCount };
	vkGetPhysicalDeviceSurfaceFormatsKHR(G_VulkanDevice.GetPhysicalDevice(),
	                                     m_Surface,
	                                     &formatCount,
	                                     surfaceFormats.data());

	// If the surface format list only includes one entry with VK_FORMAT_UNDEFINED,
	// there is no preferred format, so we assume VK_FORMAT_B8G8R8A8_UNORM
	if (formatCount == 1 && surfaceFormats[0].format == VK_FORMAT_UNDEFINED) {
		m_Format = VK_FORMAT_B8G8R8A8_UNORM;
		m_ColorSpace = surfaceFormats[0].colorSpace;
		return true;
	}

	// iterate over the list of available surface format and
	// check for the presence of VK_FORMAT_B8G8R8A8_UNORM
	for (const auto& surfaceFormat : surfaceFormats) {

		if (surfaceFormat.format == VK_FORMAT_B8G8R8A8_UNORM) {
			m_Format = surfaceFormat.format;
			m_ColorSpace = surfaceFormat.colorSpace;
			return true;
		}

	}

	// If VK_FORMAT_B8G8R8A8_UNORM is not available
	// select the first available color format
	m_Format = surfaceFormats[0].format;
	m_ColorSpace = surfaceFormats[0].colorSpace;

	LOG("Surface successfully initialized.");

	return true;
}
// ------------------------------------------------------------

VulkanSwapChain::~VulkanSwapChain()
{
	LOG("Cleaning up VulkanSwapchain(destroying surface)");
	Destroy();

	vkDestroySurfaceKHR(G_VulkanInstance, m_Surface, nullptr);
}

bool VulkanSwapChain::Initialize(const VulkanWindow& window) noexcept
{
	if (!InitializeSurface(window)) {
		return false;
	}

	return true;
}

bool VulkanSwapChain::Create(const Vec2i& size, bool vsync) noexcept
{
	VkSwapchainKHR oldSwapChain = m_SwapChain;

	// Get physical device surface properties and formats
	VkSurfaceCapabilitiesKHR surfaceCapabilities{};
	VkResult result{ vkGetPhysicalDeviceSurfaceCapabilitiesKHR(G_VulkanDevice.GetPhysicalDevice(),
	                                                           m_Surface,
	                                                           &surfaceCapabilities) };

	if (result != VK_SUCCESS) {
		ERROR_LOG("Failed to get physical device surface capabilities.");
		return false;
	}

	// Get available present modes
	ui32 presentModeCount;
	result = vkGetPhysicalDeviceSurfacePresentModesKHR(G_VulkanDevice.GetPhysicalDevice(),
	                                                   m_Surface,
	                                                   &presentModeCount,
	                                                   nullptr);

	if (result != VK_SUCCESS) {
		ERROR_LOG("Failed to get physical device surface present modes.");
		return false;
	}

	if (!presentModeCount) {
		ERROR_LOG("No available physical device surface present modes.");
		return false;
	}

	std::vector<VkPresentModeKHR> presentModes{ presentModeCount };
	result = vkGetPhysicalDeviceSurfacePresentModesKHR(G_VulkanDevice.GetPhysicalDevice(),
	                                                   m_Surface,
	                                                   &presentModeCount,
	                                                   presentModes.data());

	if (result != VK_SUCCESS) {
		ERROR_LOG("Failed to get physical device surface present modes.");
		return false;
	}

	// If width (and height) equals the special value 0xFFFFFFFF, the size of the surface will be set by the swapchain
	if (surfaceCapabilities.currentExtent.width != std::numeric_limits<ui32>::max()) {
		m_Extent = surfaceCapabilities.currentExtent;
	} else {
		VkExtent2D actualExtent{ static_cast<ui32>(size.x), static_cast<ui32>(size.y) };

		actualExtent.width = std::max(surfaceCapabilities.minImageExtent.width,
		                              std::min(surfaceCapabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = std::max(surfaceCapabilities.minImageExtent.height,
		                               std::min(surfaceCapabilities.maxImageExtent.height, actualExtent.height));

		m_Extent = actualExtent;
	}


	// Select a present mode for the swapchain

	// The VK_PRESENT_MODE_FIFO_KHR mode must always be present as per spec
	// This mode waits for the vertical blank ("v-sync")
	VkPresentModeKHR swapChainPresentMode{ VK_PRESENT_MODE_FIFO_KHR };

	// If v-sync is not requested, try to find a mailbox mode
	// It's the lowest latency non-tearing present mode available
	if (!vsync) {
		for (size_t i = 0; i < presentModeCount; i++) {
			if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
				swapChainPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
				break;
			}

			if (swapChainPresentMode != VK_PRESENT_MODE_MAILBOX_KHR &&
			    presentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR) {
				swapChainPresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
			}
		}
	}

	// Determine the number of images
	ui32 desiredNumberOfSwapChainImages{ surfaceCapabilities.minImageCount + 1 };

	if (surfaceCapabilities.maxImageCount > 0 &&
	    desiredNumberOfSwapChainImages > surfaceCapabilities.maxImageCount) {
		desiredNumberOfSwapChainImages = surfaceCapabilities.maxImageCount;
	}

	// Find the transformation of the surface
	VkSurfaceTransformFlagsKHR preTransform;
	if (surfaceCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) {
		// We prefer a non-rotated transform
		preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	} else {
		preTransform = surfaceCapabilities.currentTransform;
	}

	// Find a supported composite alpha format (not all devices support alpha opaque)
	VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

	// Simply select the first composite alpha format available
	std::vector<VkCompositeAlphaFlagBitsKHR> compositeAlphaFlags{
			VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
			VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
			VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
			VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
	};

	for (const auto& compositeAlphaFlag : compositeAlphaFlags) {

		if (surfaceCapabilities.supportedCompositeAlpha & compositeAlphaFlag) {
			compositeAlpha = compositeAlphaFlag;
			break;
		}

	}

	VkSwapchainCreateInfoKHR swapChainCreateInfo{};
	swapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapChainCreateInfo.surface = m_Surface;
	swapChainCreateInfo.minImageCount = desiredNumberOfSwapChainImages;
	swapChainCreateInfo.imageFormat = m_Format;
	swapChainCreateInfo.imageColorSpace = m_ColorSpace;
	swapChainCreateInfo.imageExtent = m_Extent;
	swapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapChainCreateInfo.preTransform = static_cast<VkSurfaceTransformFlagBitsKHR>(preTransform);
	swapChainCreateInfo.imageArrayLayers = 1;
	swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapChainCreateInfo.queueFamilyIndexCount = 0;
	swapChainCreateInfo.pQueueFamilyIndices = nullptr;
	swapChainCreateInfo.presentMode = swapChainPresentMode;
	swapChainCreateInfo.oldSwapchain = oldSwapChain;
	swapChainCreateInfo.clipped = VK_TRUE;
	swapChainCreateInfo.compositeAlpha = compositeAlpha;

	// Set additional usage flag for blitting from the swapchain images if supported
	VkFormatProperties formatProperties;
	vkGetPhysicalDeviceFormatProperties(G_VulkanDevice.GetPhysicalDevice(), m_Format, &formatProperties);

	if (formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_DST_BIT) {
		swapChainCreateInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	}

	result = vkCreateSwapchainKHR(G_VulkanDevice, &swapChainCreateInfo, nullptr, &m_SwapChain);

	if (result != VK_SUCCESS) {
		ERROR_LOG("Failed to create swap chain.");
		return false;
	}

	ui32 imageCount{ static_cast<ui32>(m_Images.size()) };

	// If an existing swap chain is re-created, destroy the old swap chain
	// This also cleans up all the presentable images
	if (oldSwapChain != VK_NULL_HANDLE) {

		LOG("Destroying old swapchain.");

		for (ui32 i = 0; i < imageCount; i++) {
			vkDestroyImageView(G_VulkanDevice, m_ImageViews[i], nullptr);
		}

		m_ImageViews.clear();

		vkDestroySwapchainKHR(G_VulkanDevice, oldSwapChain, nullptr);

		m_Images.clear();
	}

	result = vkGetSwapchainImagesKHR(G_VulkanDevice, m_SwapChain, &imageCount, nullptr);

	if (result != VK_SUCCESS) {
		ERROR_LOG("Failed to get swap chain image count.");
		return false;
	}

	// Get the swap chain images
	m_Images.resize(imageCount);
	result = vkGetSwapchainImagesKHR(G_VulkanDevice, m_SwapChain, &imageCount, m_Images.data());

	if (result != VK_SUCCESS) {
		ERROR_LOG("Failed to get the swap chain images.");
		return false;
	}

	// Get the swap chain buffers containing the image and imageview
	m_ImageViews.resize(imageCount);
	for (ui32 i = 0; i < imageCount; ++i) {
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.format = m_Format;
		createInfo.components = {
				VK_COMPONENT_SWIZZLE_R,
				VK_COMPONENT_SWIZZLE_G,
				VK_COMPONENT_SWIZZLE_B,
				VK_COMPONENT_SWIZZLE_A
		};

		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.flags = 0;
		createInfo.image = m_Images[i];

		result = vkCreateImageView(G_VulkanDevice, &createInfo, nullptr, &m_ImageViews[i]);

		if (result != VK_SUCCESS) {
			ERROR_LOG("Failed to create swap chain image view");
			return false;
		}
	}

	LOG("Successfully created the swap chain.");

	return true;
}

const std::vector<VkImage>& VulkanSwapChain::GetImages() const noexcept
{
	return m_Images;
}

const std::vector<VkImageView>& VulkanSwapChain::GetImageViews() const noexcept
{
	return m_ImageViews;
}

ui32 VulkanSwapChain::GetQueueIndex() const noexcept
{
	return m_GraphicsAndPresentQueueIndex;
}

VkFormat VulkanSwapChain::GetFormat() const noexcept
{
	return m_Format;
}

const VkExtent2D& VulkanSwapChain::GetExtent() const noexcept
{
	return m_Extent;
}

VkResult VulkanSwapChain::GetNextImageIndex(VkSemaphore presentComplete, ui32& index) const noexcept
{
	return vkAcquireNextImageKHR(G_VulkanDevice,
	                             m_SwapChain,
	                             std::numeric_limits<ui64>::max(),
	                             presentComplete,
	                             VK_NULL_HANDLE,
	                             &index);
}

VkResult VulkanSwapChain::Present(VkQueue presentQueue, ui32 imageIndex, VkSemaphore waitSemaphore) const noexcept
{
	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext = nullptr;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &m_SwapChain;
	presentInfo.pImageIndices = &imageIndex;

	if (waitSemaphore != VK_NULL_HANDLE) {
		presentInfo.pWaitSemaphores = &waitSemaphore;
		presentInfo.waitSemaphoreCount = 1;
	}

	return vkQueuePresentKHR(presentQueue, &presentInfo);
}

void VulkanSwapChain::Destroy() const noexcept
{
	for (const auto& imageView : m_ImageViews) {
		vkDestroyImageView(G_VulkanDevice, imageView, nullptr);
	}

	vkDestroySwapchainKHR(G_VulkanDevice, m_SwapChain, nullptr);
}
