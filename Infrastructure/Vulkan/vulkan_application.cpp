#include "vulkan_application.h"
#include "vulkan_infrastructure_context.h"
#include <array>
#include <algorithm>
#include <fstream>

// Private functions -------------------------------
bool VulkanApplication::CreateInstance() noexcept
{
	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = GetSettings().name.c_str();
	appInfo.pEngineName = GetSettings().name.c_str();
	appInfo.apiVersion = VK_MAKE_VERSION(1, 0, 54);

	auto instanceExtensions = VulkanWindow::GetExtensions();

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
	return m_Instance.Create(appInfo, instanceExtensions, layers);
}

bool VulkanApplication::CreateCommandBuffers() noexcept
{
	m_DrawCommandBuffers.resize(m_SwapChain.GetImages().size());

	VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.commandPool = m_CommandPool;
	commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferAllocateInfo.commandBufferCount = static_cast<ui32>(m_DrawCommandBuffers.size());

	const auto result = vkAllocateCommandBuffers(m_Device,
	                                             &commandBufferAllocateInfo,
	                                             m_DrawCommandBuffers.data());

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

	const auto result = vkCreateRenderPass(m_Device,
	                                       &renderPassInfo,
	                                       nullptr,
	                                       &m_RenderPass);

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

	const auto swapChainExtent = m_SwapChain.GetExtent();

	for (int i = 0; i < swapChainImageViews.size(); ++i) {
		const std::vector<VkImageView> attachments{ swapChainImageViews[i], m_DepthStencil.GetImageView() };

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
	vkDeviceWaitIdle(m_Device);
	vkDestroyRenderPass(m_Device, m_RenderPass, nullptr);
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

const VulkanDepthStencil& VulkanApplication::GetDepthStencil() const noexcept
{
	return m_DepthStencil;
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

const VulkanSemaphore& VulkanApplication::GetPresentCompleteSemaphore() const noexcept
{
	return m_PresentComplete;
}

const VulkanSemaphore& VulkanApplication::GetDrawCompleteSemaphore() const noexcept
{
	return m_DrawComplete;
}

bool VulkanApplication::Reshape(const Vec2ui& size) noexcept
{
	vkDeviceWaitIdle(m_Device);

	m_SwapChain.Create(size, GetSettings().vsync);

	m_DepthStencil.Destroy();

	const VkExtent2D extent = m_SwapChain.GetExtent();
	if (!m_DepthStencil.Create(Vec2ui{ extent.width, extent.height },
	                           m_Device.GetPhysicalDevice().GetSupportedDepthFormat())) {
		return false;
	}

	for (auto& swapChainFrameBuffer : m_SwapChainFrameBuffers) {
		swapChainFrameBuffer.Destroy();
	}

	for (ui32 i = 0; i < m_SwapChainFrameBuffers.size(); ++i) {
		const std::vector<VkImageView> imageViews{
			m_SwapChain.GetImageViews()[i],
			m_DepthStencil.GetImageView()
		};

		if (!m_SwapChainFrameBuffers[i].Create(imageViews,
		                                       Vec2ui{ extent.width, extent.height },
		                                       m_RenderPass)) {
			return false;
		}
	}

	vkFreeCommandBuffers(m_Device,
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

	vkDeviceWaitIdle(m_Device);

	OnResize(Vec2ui{ extent.width, extent.height });

	return true;
}

bool VulkanApplication::Initialize() noexcept
{
	if (!Application::Initialize()) {
		return false;
	}

	VulkanInfrastructureContext::Register(&m_Instance,
	                                      &m_Device,
	                                      &m_ResourceManager,
	                                      this);

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
	if (!m_VulkanDebug.Initialize()) {
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

	if (!m_SwapChain.Initialize(m_Window)) {
		return false;
	}

	if (!m_SwapChain.Create(settings.windowResolution, settings.vsync)) {
		return false;
	}

	const auto depthStencilFormat = m_Device.GetPhysicalDevice().GetSupportedDepthFormat();

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

	if (!CreateRenderPasses()) {
		return false;
	}

	queryPools.resize(m_SwapChain.GetImages().size());

	for (auto& queryPool : queryPools) {
		queryPool.Initialize(VK_QUERY_TYPE_TIMESTAMP, 2, VK_NULL_HANDLE);
	}

	if (!CreateFramebuffers()) {
		ERROR_LOG("Failed to create framebuffers.");
		return false;
	}

	return true;
}

i32 VulkanApplication::Run() noexcept
{
	GetTimer().Start();
	while (!glfwWindowShouldClose(m_Window) && !ShouldTerminate()) {
		glfwPollEvents();

		static auto prev = 0.0;

		Update();
		Draw();

		const auto now = GetTimer().GetSec();
		wholeFrameTime = (now - prev) * 1000.0;

		std::vector<ui64> gpuResults;
		queryPools[m_CurrentBuffer].GetResults(gpuResults);

		const auto nanosInAnIncrement{ m_Device.GetPhysicalDevice().properties.limits.timestampPeriod };

		gpuTime = (gpuResults[1] - gpuResults[0]) * nanosInAnIncrement * 1e-6;
		cpuTime = wholeFrameTime - gpuTime;

		if (cpuTime < 0.0f) {
			cpuTime = 0.0f;
		}

		if (!benchmarkComplete) {
			// Calculate moving averages
			static float accum{ 0 };
			accum += wholeFrameTime;
			wholeFrameTimeSamples.push_back(wholeFrameTime);
			cpuTimeSamples.push_back(cpuTime);
			gpuTimeSamples.push_back(gpuTime);

			totalFrameTimeSamples.push_back(wholeFrameTime);
			totalCpuTimeSamples.push_back(cpuTime);
			totalGpuTimeSamples.push_back(gpuTime);

			if (accum > 1000.0f || calculateResults) {
				const auto size = wholeFrameTimeSamples.size();
				auto wholeFrameTimeSum{ 0.0 };
				auto cpuTimeSum{ 0.0 };
				auto gpuTimeSum{ 0.0 };

				for (auto i = 0u; i < size; ++i) {
					wholeFrameTimeSum += wholeFrameTimeSamples[i];
					cpuTimeSum += cpuTimeSamples[i];
					gpuTimeSum += gpuTimeSamples[i];
				}
				wholeFrameAverage = wholeFrameTimeSum / static_cast<f32>(size);
				averageFps = 1000.0f / wholeFrameAverage;

				totalFpsSamples.push_back(averageFps);

				cpuTimeAverage = cpuTimeSum / static_cast<f32>(size);
				gpuTimeAverage = gpuTimeSum / static_cast<f32>(size);


				minWholeFrame = std::min(minWholeFrame, wholeFrameTime);
				maxWholeFrame = std::max(wholeFrameTime, maxWholeFrame);

				minFps = std::min(minFps, averageFps);
				maxFps = std::max(maxFps, averageFps);

				minCpuTime = std::min(minCpuTime, cpuTimeAverage);
				maxCpuTime = std::max(maxCpuTime, cpuTimeAverage);

				minGpuTime = std::min(minGpuTime, gpuTimeAverage);
				maxGpuTime = std::max(maxGpuTime, gpuTimeAverage);

				std::rotate(fpsAverages.begin(), fpsAverages.begin() + 1, fpsAverages.end());
				std::rotate(wholeFrameAverages.begin(), wholeFrameAverages.begin() + 1, wholeFrameAverages.end());
				std::rotate(cpuTimeAverages.begin(), cpuTimeAverages.begin() + 1, cpuTimeAverages.end());
				std::rotate(gpuTimeAverages.begin(), gpuTimeAverages.begin() + 1, gpuTimeAverages.end());

				fpsAverages.back() = averageFps;
				wholeFrameAverages.back() = wholeFrameAverage;
				cpuTimeAverages.back() = cpuTimeAverage;
				gpuTimeAverages.back() = gpuTimeAverage;

				wholeFrameTimeSamples.clear();
				cpuTimeSamples.clear();
				gpuTimeSamples.clear();

				accum = 0;
			}

			prev = now;
			++frameCount;
		}

		if (calculateResults) {

			maxTotalFrameTime = *std::max_element(totalFrameTimeSamples.cbegin(), totalFrameTimeSamples.cend());
			minTotalFrameTime = *std::min_element(totalFrameTimeSamples.cbegin(), totalFrameTimeSamples.cend());

			maxTotalCpuTime = *std::max_element(totalCpuTimeSamples.cbegin(), totalCpuTimeSamples.cend());
			minTotalCpuTime = *std::min_element(totalCpuTimeSamples.cbegin(), totalCpuTimeSamples.cend());

			maxTotalGpuTime = *std::max_element(totalGpuTimeSamples.cbegin(), totalGpuTimeSamples.cend());
			minTotalGpuTime = *std::min_element(totalGpuTimeSamples.cbegin(), totalGpuTimeSamples.cend());

			for (auto i = 0u; i < totalFrameTimeSamples.size(); ++i) {
				avgTotalFrameTime += totalFrameTimeSamples[i];
				avgTotalCpuTime += totalCpuTimeSamples[i];
				avgTotalGpuTime += totalGpuTimeSamples[i];
			}

			avgTotalFrameTime /= static_cast<f32>(totalFrameTimeSamples.size());
			avgTotalCpuTime /= static_cast<f32>(totalCpuTimeSamples.size());
			avgTotalGpuTime /= static_cast<f32>(totalGpuTimeSamples.size());

			auto frameTimeVecCopy = totalFrameTimeSamples;

			std::sort(frameTimeVecCopy.begin(), frameTimeVecCopy.end(), [](auto a, auto b) { return a < b; });

			auto index = 0.99f * frameTimeVecCopy.size();

			auto integral = 0.0f;
			const auto fractional = modff(index, &integral);

			if (fractional == 0.0f) {
				percentile99th = frameTimeVecCopy[static_cast<i32>(index)];
			} else {
				index = std::round(index);
				percentile99th = frameTimeVecCopy[index];
			}

			benchmarkComplete = true;
			calculateResults = false;
		}

		if (!frameRateTermination) {
			if (GetDuration() > 0.0f && now > GetDuration() && !benchmarkComplete) {
				totalAppDuration = now;
				calculateResults = true;
			}
		}
		else {
			if (wholeFrameAverage > 33.3 && !benchmarkComplete) {
				totalAppDuration = now;
				calculateResults = true;
			}
		}
	}

	return 0;
}

void VulkanApplication::PreDraw() noexcept
{
	VkResult result{ m_SwapChain.GetNextImageIndex(m_PresentComplete, m_CurrentBuffer) };

	while (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
		const auto& extent = m_SwapChain.GetExtent();
		Reshape(Vec2i{ extent.width, extent.height });

		result = m_SwapChain.GetNextImageIndex(m_PresentComplete,
		                                       m_CurrentBuffer);
	}

	vkQueueWaitIdle(m_Device.GetQueue(QueueFamily::GRAPHICS));
}

void VulkanApplication::PostDraw() noexcept
{
	VkResult result{
		m_SwapChain.Present(m_Device.GetQueue(QueueFamily::PRESENT),
		                    m_CurrentBuffer,
		                    m_DrawComplete)
	};

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
		const auto& extent = m_SwapChain.GetExtent();
		Reshape(Vec2i{ extent.width, extent.height });
	}

	w2 = GetTimer().GetSec();
}

void VulkanApplication::SaveToCsv(const std::string& fname)
{
	std::ofstream stream{ fname + ".csv" };

	stream << "Whole Frame Time,CPU Time,GPU Time\n";

	for (auto i = 0u; i < totalFrameTimeSamples.size(); ++i) {
		stream << totalFrameTimeSamples[i] << "," << totalCpuTimeSamples[i] << "," << totalGpuTimeSamples[i] << "\n";
	}

	stream << "\nFPS,Whole Frame Time,CPU Time,GPU Time\n";

	for (auto i = 0u; i < fpsAverages.size(); ++i) {
		stream << fpsAverages[i] << "," << wholeFrameAverages[i] << "," << cpuTimeAverages[i] << "," << gpuTimeAverages[i] <<
				"\n";
	}

	stream << "\nAverage FPS,Average Frame Time,Average CPU Time,Average GPU Time\n";
	stream << 1000.0f / avgTotalFrameTime << "," << avgTotalFrameTime << "," << avgTotalCpuTime << "," << avgTotalGpuTime;

	stream << "\n99th percentile\n";
	stream << percentile99th;

	stream.close();
}
