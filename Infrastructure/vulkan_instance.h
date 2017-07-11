#ifndef VULKAN_INSTANCE_H_
#define VULKAN_INSTANCE_H_
#include <vulkan/vulkan.h>
#include <vector>

class VulkanInstance {
private:
	VkInstance m_Instance{ nullptr };

	std::vector<const char*> m_Extensions;

	std::vector<const char*> m_Layers;

public:
	~VulkanInstance();

	bool Create(const VkApplicationInfo& applicationInfo,
	            const std::vector<const char*>& extensions,
	            const std::vector<const char*>& layers) noexcept;

	operator VkInstance() const noexcept;
};

#endif // VULAKN_INSTANCE_H_
