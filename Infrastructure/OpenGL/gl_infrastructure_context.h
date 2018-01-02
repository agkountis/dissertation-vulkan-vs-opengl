#ifndef GL_INFRASTRUCTURE_CONTEXT_H_
#define GL_INFRASTRUCTURE_CONTEXT_H_
#include "resource_manager.h"

class GLApplication;

class GLInfrastructureContext {
private:
	static ResourceManager* s_pResourceManager;

	static GLApplication* s_Application;

public:
	static ResourceManager& GetResourceManager() noexcept;

	static GLApplication& GetApplication() noexcept;

	static void Register(ResourceManager* resourceManager,
						 GLApplication* application) noexcept;
};

#define G_ResourceManager GLInfrastructureContext::GetResourceManager()
#define G_Application GLInfrastructureContext::GetApplication()

#endif //GL_INFRASTRUCTURE_CONTEXT_H_
