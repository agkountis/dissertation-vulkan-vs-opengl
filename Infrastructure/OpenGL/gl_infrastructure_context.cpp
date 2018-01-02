#include "gl_infrastructure_context.h"

ResourceManager* GLInfrastructureContext::s_pResourceManager{ nullptr };

GLApplication* GLInfrastructureContext::s_Application{ nullptr };

ResourceManager& GLInfrastructureContext::GetResourceManager() noexcept
{
	return *s_pResourceManager;
}

GLApplication& GLInfrastructureContext::GetApplication() noexcept
{
	return *s_Application;
}

void GLInfrastructureContext::Register(ResourceManager* resourceManager,
                                           GLApplication* application) noexcept
{
	assert(resourceManager);
	assert(application);

	s_pResourceManager = resourceManager;

	s_Application = application;
}
