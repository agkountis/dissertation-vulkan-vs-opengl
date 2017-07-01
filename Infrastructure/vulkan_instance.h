#ifndef VULKAN_INSTANCE_H_
#define VULKAN_INSTANCE_H_
#include <vulkan/vulkan.h>
#include <vector>

class VulkanInstance {
private:
	VkInstance m_Instance{ nullptr };

public:
	VulkanInstance(const VkApplicationInfo& applicationInfo,
	               const std::vector<const char*>& extensions,
	               const std::vector<const char*>& layers);

	~VulkanInstance();

	VkInstance GetHandle() const noexcept;
};

#endif // VULAKN_INSTANCE_H_
