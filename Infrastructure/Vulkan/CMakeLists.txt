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
        vulkan_renderer.cpp
        vulkan_renderer.h
        vulkan_semaphore.cpp
        vulkan_semaphore.h
        vulkan_shader.cpp
        vulkan_shader.h
        vulkan_swapchain.cpp
        vulkan_swapchain.h
        vulkan_utilities.cpp
        vulkan_utilities.h
        vulkan_vbo.cpp
        vulkan_vbo.h
        vulkan_window.cpp
        vulkan_window.h vulkan_mesh.h mesh.cpp)

include_directories(../Core)

set(LIBRARIES CoreInfrastructure)

if(APPLE)
    include_directories(~/Downloads/Molten-0.18.0/MoltenVK/include)
    link_directories(~/Downloads/Molten-0.18.0/MoltenVK/macOS)

    find_library(COCOA Cocoa REQUIRED)
    find_library(METAL Metal REQUIRED)
    find_library(QUARTZ_CORE QuartzCore REQUIRED)
    find_package(glfw3 3.3 REQUIRED)

    set(VULKAN_LIB "/Users/gordath/Downloads/Molten-0.18.0/MoltenVK/macOS/libMoltenVK.dylib")
    set(LIBRARIES ${LIBRARIES} ${COCOA} ${METAL} ${QUARTZ_CORE} ${VULKAN_LIB})
elseif(MSVC)
	include_directories(C:/usr/include)
    include_directories(C:/VulkanSDK/1.0.54.0/Include)

	set(VULKAN_LIB C:/VulkanSDK/1.0.54.0/Lib32/vulkan-1.lib)
	set(GLFW_LIB C:/usr/lib/glfw3.lib)

	set(LIBRARIES ${LIBRARIES} ${VULKAN_LIB} ${GLFW_LIB})
elseif(MINGW)
    #handle mingw
else()
    set(VULKAN_LIB vulkan)
    find_package(glfw3 3.2 REQUIRED)
    set(LIBRARIES ${LIBRARIES} ${VULKAN_LIB} glfw)
endif()

add_library(VulkanInfrastructure ${SOURCE_FILES})

target_link_libraries(VulkanInfrastructure ${LIBRARIES})
