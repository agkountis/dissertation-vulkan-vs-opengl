#ifndef VULKAN_APPLICATION_H_
#define VULKAN_APPLICATION_H_

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>

#include "application.h"
#include <vector>
#include "vulkan_window.h"
#include "vulkan_physical_device.h"

#if !defined(NDEBUG) && !defined(__APPLE__)
#include "vulkan_debug.h"
#endif

#include "vulkan_instance.h"
#include "vulkan_swapchain.h"
#include "vulkan_device.h"
#include "vulkan_depth_stencil.h"
#include "vulkan_semaphore.h"
#include "vulkan_command_pool.h"
#include "vulkan_framebuffer.h"
#include "vulkan_shader.h"
#include "vulkan_query_pool.h"

class VulkanApplication : public Application {
private:
	/**
	 * \brief The application's window.
	 */
	VulkanWindow m_Window;

	/**
	 * \brief The Vulkan instance.
	 */
	VulkanInstance m_Instance;

	/**
	 * \brief The vulkan device.
	 * \details This class encapsulates both the physical
	 * and the logical device. Many actions such is enabling
	 * physical device features, creating and copying buffers or
	 * creating and copying images are performed using this class.
	 */
	VulkanDevice m_Device;

#if !defined(NDEBUG) && !defined(__APPLE__)
	/**
	 * \brief The Vulkan debug instance.
	 * \details This class is responsible for creating the
	 * Vulkan debug callback as well as destroying it.
	 * It handles and prints messages from the validation layer
	 * to the console for debugging.
	 */
	VulkanDebug m_VulkanDebug;
#endif

	/**
 	 * \brief The resource manager.
 	 * \details This class handles the loading of resources.
 	 * It loads and manages any class that is a Resource.
 	 * \note If a Resource has already been loaded and registered to the
 	 * manager the manager will not reload the Resource.
 	 */
	ResourceManager m_ResourceManager;

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
	 * \brief Structure used to submit commands to the queues.
	 */
	VkSubmitInfo m_SubmitInfo{};

	/**
	 * \brief The command buffers to use for drawing.
	 * \details This vector will contain 1 command buffer per
	 * swap chain image. This is convenient when command buffer recording
	 * is done on initialization. That way the gpu doesn't have to wait for an
	 * available command buffer.
	 * \note If command buffers are recorded per frame then 1 command buffer is
	 * sufficient since it will be prepared every frame and is immediately available.
	 */
	std::vector<VkCommandBuffer> m_DrawCommandBuffers;

	/**
	 * \brief The available framebuffers.
	 * \details Same as the number of swap chain images.
	 */
	std::vector<VulkanFramebuffer> m_SwapChainFrameBuffers;

	/**
	 * \brief A structure that wraps the functionality of a
	 *  Vulkan depth stencil.
	 */
	VulkanDepthStencil m_DepthStencil;

	/**
	 * \brief The default render pass.
	 */
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
	 * \brief The current command buffer to use based on the index of
	 * the available swap chain image.
	 */
	ui32 m_CurrentBuffer{ 0 };

	/**
	 * \brief Creates the Vulkan instance.
	 * \details Derived classes can override for
	 * application specific initialization.
	 * \return The corresponding Vulkan result code.
	 */
	virtual bool CreateInstance() noexcept;

	/**
	 * \brief Function to be overridden by derived classes for
	 * command buffer recording.
	 * \return TRUE if successful, FALSE otherwise.
	 */
	virtual bool BuildCommandBuffers() noexcept = 0;

	/**
	 * \brief Function for derived classes to override in order to enable
	 * specific physical device features if supported.
	 * \details Must be called after the physical device has been picked.
	 */
	virtual void EnableFeatures() noexcept = 0;

	/**
 	 * \brief Creates the command buffers used to record rendering commands.
 	 * \return TRUE if successful, FALSE otherwise.
 	 */
	bool CreateCommandBuffers() noexcept;

	/**
	 * \brief Function for derived classes to override to set up the application
	 * specific render passes.
	 * \return TRUE if successful, FALSE otherwise.
	 */
	virtual bool CreateRenderPasses() noexcept;

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
	
	std::vector<VulkanQueryPool> queryPools;
	
	double w1, w2;
	/**
	 * \brief VulkanApplication's constructor.
	 * \param settings The settings of the application.
	 */
	explicit VulkanApplication(const ApplicationSettings& settings);

	/**
	 * \brief VulkanApplication's destructor.
	 * \details Cleans up allocated Vulkan resources.
	 */
	~VulkanApplication() override;

	/**
	 * \brief Returns the window of the application.
	 * \return The window of the application.
	 */
	VulkanWindow& GetWindow() noexcept;

	/**
	 * \brief Returns a reference to the VkPhysicalDeviceFeatures struct
	 * in order for features to be turned on and/or off.
	 * \return A reference to the VkPhysicalDeviceFeatures struct.
	 */
	VkPhysicalDeviceFeatures& GetFeaturesToEnable() noexcept;

	const VulkanSwapChain& GetSwapChain() const noexcept;

	const std::vector<VkCommandBuffer>& GetCommandBuffers() const noexcept;

	const std::vector<VulkanFramebuffer>& GetFramebuffers() const noexcept;

	VkRenderPass GetRenderPass() const noexcept;

	VkSubmitInfo& GetSubmitInfo() noexcept;

	ui32 GetCurrentBufferIndex() const noexcept;

	bool Reshape(const Vec2ui& size) noexcept;

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
	i32 Run() noexcept final;

	void PreDraw() noexcept override;

	void PostDraw() noexcept override;
};

#endif // VULKAN_APPLICATION_H_