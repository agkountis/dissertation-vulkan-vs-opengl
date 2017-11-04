set(SOURCE_FILES vulkan_application.cpp
        vulkan_application.h
        vulkan_buffer.cpp
        vulkan_buffer.h
        vulkan_command_pool.cpp
        vulkan_command_pool.h
        vulkan_debug.cpp
        vulkan_debug.h
        vulkan_depth_stencil.cpp
        vulkan_depth_stencil.h
        vulkan_device.cpp
        vulkan_device.h
        vulkan_framebuffer.cpp
        vulkan_framebuffer.h
        vulkan_instance.cpp
        vulkan_instance.h
        vulkan_physical_device.cpp
        vulkan_physical_device.h
        vulkan_pipeline_cache.cpp
        vulkan_pipeline_cache.h
        vulkan_semaphore.cpp
        vulkan_semaphore.h
        vulkan_shader.cpp
        vulkan_shader.h
        vulkan_swapchain.cpp
        vulkan_swapchain.h
        vulkan_utilities.cpp
        vulkan_utilities.h
        vulkan_window.cpp
        vulkan_window.h
        vulkan_mesh.h
        vulkan_mesh.cpp
        vulkan_texture.h
        vulkan_texture.cpp
        vulkan_infrastructure_context.h
        vulkan_infrastructure_context.cpp
		vulkan_query_pool.h
		vulkan_query_pool.cpp
		vulkan_render_target.h
		vulkan_render_target.cpp)

include_directories(../Core)

add_library(VulkanInfrastructure ${SOURCE_FILES})

if(MSVC)
	set_target_properties(VulkanInfrastructure PROPERTIES FOLDER Infrastructure)
endif()

target_link_libraries(VulkanInfrastructure CoreInfrastructure ${VULKAN_APP_LIBRARIES})
