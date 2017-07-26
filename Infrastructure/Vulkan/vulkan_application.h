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
#include "vulkan_semaphore.h"
#include "vulkan_command_pool.h"
#include "vulkan_pipeline_cache.h"
#include "vulkan_framebuffer.h"
#include "vulkan_shader.h"

class VulkanApplication : public Application {
private:
	/**
	 * \brief The Vulkan instance. Manages per-application states.
	 */
	VulkanInstance m_Instance;

	/**
	 * \brief The application's window.
	 */
	VulkanWindow m_Window;

	/**
	 * \brief Encapsulates both the physical and the logical device.
	 */
	VulkanDevice m_Device;

#if !defined(NDEBUG) && !defined(__APPLE__)
	VulkanDebug m_VulkanDebug;
#endif

	/**
	 * \brief The physical device features to be enabled for this application.
	 */
	VkPhysicalDeviceFeatures m_FeaturesToEnable{};

	/**
	 * \brief The device extensions to be enabled for this application.
	 */
	std::vector<const char*> m_ExtensionsToEnable;

	/**
	 * \brief The command pool from which the application can allocate command buffers.
	 */
	VulkanCommandPool m_CommandPool;

	/**
	 * \brief The pipeline stages to wait at for graphics queue submissions.
	 */
	VkPipelineStageFlags m_PipelineStageFlags{ VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

	/**
	 * \brief Struct used to submit commands to the queues.
	 */
	VkSubmitInfo m_SubmitInfo{};

	/**
	 * \brief The command buffers to use for drawing.
	 */
	std::vector<VkCommandBuffer> m_DrawCommandBuffers;

	/**
	 * \brief The available framebuffers.
	 * \details Same as the number of swap chain images.
	 */
	std::vector<VulkanFramebuffer> m_SwapChainFrameBuffers;

	/**
	 * \brief A struct that wraps the functionality of a 
	 *  Vulkan depth stencil.
	 */
	VulkanDepthStencil m_DepthStencil;

	/**
	 * \brief The shader stages to be used in the pipeline.
	 */
	std::vector<VulkanShader*> m_Shaders;

	/**
	 * \brief Pipeline cache object used to accelerate pipeline creation.
	 */
	VulkanPipelineCache m_PipelineCache;

	VkRenderPass m_RenderPass{ VK_NULL_HANDLE };
	
	/**
	 * \brief A class that encapsulates the functionality of a 
	 * Vulkan swap chain.
	 */
	VulkanSwapChain m_SwapChain;

	/**
	 * \brief Semaphore used to signal that the presentation of an image
	 * is complete.
	 */
	VulkanSemaphore m_PresentComplete;

	/**
	 * \brief Semaphore used to signal that the drawing is complete.
	 */
	VulkanSemaphore m_DrawComplete;

	/**
	 * \brief Creates the Vulkan instance.
	 * \details Derived classes can override for
	 * application specific initialization.
	 * \return The corresponding Vulkan result code.
	 */
	virtual bool CreateInstance() noexcept;

	/**
	 * \brief Creates the command buffers used to record rendering commands.
	 * \return TRUE if successful, FALSE otherwise.
	 */
	bool CreateCommandBuffers() noexcept;

	virtual bool BuildCommandBuffers() noexcept = 0;

	/**
	 * \brief Function for derived classes to override in order to enable
	 * specific physical device features if supported.
	 * \details Must be called after the physical device has been picked.
	 */
	virtual void EnableFeatures() noexcept = 0;

	/**
	 * \brief Function for derived classes to override to set up the application
	 * specific render passes.
	 * \return TRUE if successfull, FALSE otherwise.
	 */
	virtual bool CreateRenderPasses() noexcept;

	/**
	 * \brief Function for derived classes to override to set up the application
	 * specific pipelines.
	 * \return TRUE if successfull, FALSE otherwise.
	 */
	virtual bool CreatePipelines() noexcept = 0;

	/**
	 * \brief Function for derived classes to override to set up the application
	 * specific framebuffers and the swap chain's framebuffers.
	 * \details Swap chain framebuffer creation is deferred to the derived classes
	 * due to the fact that it depends on a render pass, and render passes are application
	 * specific.
	 * \return TRUE if successful, FALSE otherwise.
	 */
	virtual bool CreateFramebuffers() noexcept;

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
	VulkanWindow& GetWindow() noexcept;

	/**
	 * \brief Returns the application's Vulkan instance.
	 * \return The application's VulkanInstance.
	 */
	const VulkanInstance& GetVulkanInstance() const noexcept;

	/**
	 * \brief Returns the vulkan device instance.
	 * \return The vulkan device instance.
	 */
	const VulkanDevice& GetDevice() const noexcept;

	/**
	 * \brief Returns a reference to the VkPhysicalDeviceFeatures struct
	 * in order for features to be turned on and/or off.
	 * \return A reference to the VkPhysicalDeviceFeatures struct.
	 */
	VkPhysicalDeviceFeatures& GetFeaturesToEnable() noexcept;

	const VulkanSwapChain& GetSwapChain() const noexcept;

	const std::vector<VkCommandBuffer>& GetCommandBuffers() const noexcept;

	const std::vector<VulkanFramebuffer>& GetFramebuffers() const noexcept;

	const VulkanPipelineCache& GetPipelineCache() const noexcept;

	const VulkanDepthStencil& GetDepthStencil() const noexcept;

	VkQueue GetGraphicsQueue() const noexcept;

	VkRenderPass GetRenderPass() const noexcept;

	const VulkanSemaphore& GetPresentCompleteSemaphore() const noexcept;

	const VulkanSemaphore& GetDrawCompleteSemaphore() const noexcept;

	VkSubmitInfo& GetSubmitInfo() noexcept;

	VulkanShader* LoadShader(const std::string& fileName) noexcept;

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
	i32 Run() noexcept final override;
};

#endif // VULKAN_APPLICATION_H_
