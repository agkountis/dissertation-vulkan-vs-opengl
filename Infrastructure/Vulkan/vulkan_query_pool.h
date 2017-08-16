#ifndef VULKAN_QUERY_POOL_H_
#define VULKAN_QUERY_POOL_H_

#include <vulkan/vulkan.h>
#include <types.h>
#include <vector>
#include "vulkan_infrastructure_context.h"

class VulkanQueryPool {
private:
	VkQueryPool m_QueryPool{ VK_NULL_HANDLE };

	ui32 m_QueryCount{ 0 };

	VkQueryType m_QueryType{};

public:
	~VulkanQueryPool();

	bool Initialize(VkQueryType queryType,
	                ui32 queryCount,
	                VkQueryPipelineStatisticFlags pipelineStatisticFlags) noexcept;


	template<typename T>
	VkResult GetResults(std::vector<T>& results, bool shouldWait = true) noexcept
	{
		assert(m_QueryCount > 0);
		static_assert((sizeof(T) == sizeof(ui32) || sizeof(T) == sizeof(ui64))
		              && !std::is_signed<T>::value,
		              "Template parameter type can only be unsigned int 32bit or 64bit");

		VkQueryResultFlags queryResultFlags{};

		if (shouldWait) {
			queryResultFlags |= VK_QUERY_RESULT_WAIT_BIT;
		}

		if (sizeof(T) == sizeof(ui64)) {
			queryResultFlags |= VK_QUERY_RESULT_64_BIT;
		}

		results.resize(m_QueryCount);

		return vkGetQueryPoolResults(G_VulkanDevice,
		                             m_QueryPool,
		                             0,
		                             m_QueryCount,
		                             m_QueryCount * sizeof(T),
		                             results.data(),
		                             sizeof(T),
		                             queryResultFlags);

	}

	operator VkQueryPool() noexcept;
};

#endif //VULKAN_QUERY_POOL_H_
