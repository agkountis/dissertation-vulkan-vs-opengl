#ifndef VULKAN_APPLICATION_H_
#define VULKAN_APPLICATION_H_

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "application.h"
#include <vector>
#include "vulkan_window.h"

struct VulkanPhysicalDevice {
	VkPhysicalDevice device{ nullptr };

	VkPhysicalDeviceProperties properties;

	VkPhysicalDeviceFeatures features;

	VkPhysicalDeviceMemoryProperties memoryProperties;

	explicit operator VkPhysicalDevice() const
	{
		return device;
	}
};

class VulkanApplication : public Application {
private:
	
	/**
	 * \brief The application's window.
	 */
	std::unique_ptr<VulkanWindow> m_Window;

	/**
	 * \brief The Vulkan instance. Manages per-application states.
	 */
	VkInstance m_Instance{ nullptr };

	/**
	 * \brief A struct that encapsulates a Vulkan physical device with it's
	 * properties and supported features.
	 */
	VulkanPhysicalDevice m_PhysicalDevice;

	/**
	 * \brief The physical device features to be enabled for this application.
	 */
	VkPhysicalDeviceFeatures m_FeaturesToEnable;

	/**
	 * \brief The device extensions to be enabled for this application.
	 */
	std::vector<const char*> m_ExtensionsToEnable;

	/**
	 * \brief Handle to the graphics queue.
	 */
	VkQueue m_GraphicsQueue{ nullptr };

	/**
	 * \brief The depth buffer's image format.
	 */
	VkFormat m_DepthBufferFormat{ VK_FORMAT_UNDEFINED };

	/**
	 * \brief The command pool from which the application can allocate command buffers.
	 */
	VkCommandPool m_CommandPool{ VK_NULL_HANDLE };

	/**
	 * \brief The pipeline stages to wait at for graphics queue submissions.
	 */
	VkPipelineStageFlags m_PipelineStageFlags{ VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

	/**
	 * \brief 
	 */
	VkSubmitInfo m_SubmitInfo;

	/**
	 * \brief The command buffers to use for drawing.
	 */
	std::vector<VkCommandBuffer> m_DrawCommandBuffers;

	/**
	 * \brief The default global render pass.
	 */
	VkRenderPass m_RenderPass{ VK_NULL_HANDLE };

	/**
	 * \brief The available framebuffers.
	 * \details Same as the number of swap chain images.
	 */
	std::vector<VkFramebuffer> m_FrameBuffers;


public:

	/**
	 * \brief Initializes the application
	 * \details This is the base class implementation and it
	 * initializes Vulkan. Derived classes can override and implement
	 * application specific initialization.
	 * \return TRUE if successfull, FALSE otherwise.
	 */
	bool Initialize() noexcept override;

	/**
	 * \brief Runs the application.
	 * \details Executes the main loop.
	 * \return The application's exit code.
	 */
	i32 Run() const noexcept final override;

	/**
	 * \brief The main draw function.
	 * \details Derived classes have to implement their specific drawing code.
	 */
	void Draw() const noexcept override = 0;
};

#endif // VULKAN_APPLICATION_H_
