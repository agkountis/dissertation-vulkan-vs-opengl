#ifndef VULKAN_APPLICATION_H_
#define VULKAN_APPLICATION_H_

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "application.h"
#include <vector>
#include "vulkan_window.h"
#include "vulkan_physical_device.h"
#include "vulkan_debug.h"
#include "vulkan_swapchain.h"
#include "vulkan_device.h"
#include "vulkan_depth_stencil.h"

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

#if !defined(NDEBUG) && !defined(__APPLE__)
	VulkanDebug m_VulkanDebug;
#endif

	/**
	 * \brief Encapsulates both the physical and the logical device.
	 */
	VulkanDevice m_Device;

	/**
	 * \brief The physical device features to be enabled for this application.
	 */
	VkPhysicalDeviceFeatures m_FeaturesToEnable{};

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
	 * \brief Struct used to submit commands to the queues.
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

	VulkanDepthStencil m_DepthStencil;

	std::vector<VkShaderModule> m_ShaderModules;

	// Pipeline cache object
	VkPipelineCache m_PipelineCache{ VK_NULL_HANDLE };

	// Wraps the swap chain to present images (framebuffers) to the windowing system
	VulkanSwapChain m_SwapChain;
	
	/**
	 * \brief Semaphore used to signal that the presentation of an image
	 * is complete.
	 */
	VkSemaphore m_PresentComplete{ VK_NULL_HANDLE };

	/**
	 * \brief Semaphore used to signal that the drawing is complete.
	 */
	VkSemaphore m_DrawComplete{ VK_NULL_HANDLE };

	/**
	 * \brief Creates the Vulkan instance.
	 * \details Derived classes can override for
	 * application specific initialization.
	 * \return The corresponding Vulkan result code.
	 */
	virtual VkResult CreateInstance() noexcept;

	bool CreateCommandPool() noexcept;

	bool CreateCommandBuffers() noexcept;

	/**
	 * \brief Function for derived classes to override in order to enable
	 * specific physical device features if supported.
	 * \details Must be called after the physical device has been picked.
	 */
	virtual void EnableFeatures() noexcept = 0;

	virtual bool SetupDepthStencil() noexcept;

public:

	/**
	 * \brief VulkanApplication's constructor.
	 * \param settings The settings of the application.
	 */
	explicit VulkanApplication(const ApplicationSettings& settings);

	/**
	 * \brief VulkanApplication's destructor.
	 * \details Cleans up allocated Vulkan resources.
	 */
	~VulkanApplication();

	/**
	 * \brief Returns the window of the application.
	 * \return The window of the application.
	 */
	const std::unique_ptr<VulkanWindow>& GetWindow() const noexcept;

	/**
	 * \brief Returns the vulkan device instance.
	 * \return The vulkan device instance.
	 */
	const VulkanDevice& GetDevice() const noexcept;

	VkPhysicalDeviceFeatures& GetFeaturesToEnable() noexcept;

	/**
	 * \brief Initializes the application
	 * \details This is the base class implementation and it
	 * initializes Vulkan. Derived classes can override and implement
	 * application specific initialization.
	 * \return TRUE if successful, FALSE otherwise.
	 */
	bool Initialize() noexcept override;

	/**
	 * \brief Runs the application.
	 * \details Executes the main loop.
	 * \return The application's exit code.
	 */
	i32 Run() const noexcept final override;
};

#endif // VULKAN_APPLICATION_H_
