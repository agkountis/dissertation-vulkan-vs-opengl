project(Infrastructure)

set(SOURCE_FILES
        Vulkan/vulkan_vbo.h
        Vulkan/vulkan_renderer.cpp
        Vulkan/vulkan_device.cpp
        Vulkan/vulkan_instance.cpp
        types.h
        Vulkan/vulkan_utilities.h
        Vulkan/vulkan_window.h
        Vulkan/vulkan_application.cpp
        Vulkan/vulkan_debug.h
        Vulkan/vulkan_debug.cpp
        Vulkan/vulkan_instance.h
        logger.h
        Vulkan/vulkan_utilities.cpp
        Vulkan/vulkan_renderer.h
        window.h
        Vulkan/vulkan_window.cpp
        application.h
        application.cpp
        Vulkan/vulkan_device.h
        Vulkan/vulkan_application.h
        timer.h
        Vulkan/vulkan_vbo.cpp
        window.cpp
        Vulkan/vulkan_physical_device.h
        Vulkan/vulkan_swapchain.h
        Vulkan/vulkan_swapchain.cpp
        Vulkan/vulkan_physical_device.cpp
        Vulkan/vulkan_buffer.h
        Vulkan/vulkan_buffer.cpp
        Vulkan/vulkan_pipeline_cache.cpp
        Vulkan/vulkan_command_pool.cpp
        Vulkan/vulkan_render_pass.cpp
        Vulkan/vulkan_depth_stencil.cpp
        Vulkan/vulkan_command_pool.h
        Vulkan/vulkan_depth_stencil.h
        Vulkan/vulkan_render_pass.h
        Vulkan/vulkan_semaphore.cpp
        Vulkan/vulkan_pipeline_cache.h
        Vulkan/vulkan_semaphore.h
        Vulkan/vulkan_framebuffer.h
        Vulkan/vulkan_framebuffer.cpp
        entity.h
        resource.h
        resource_manager.h Vulkan/vulkan_shader.h Vulkan/vulkan_shader.cpp)

if(APPLE)
    include_directories(~/Downloads/Molten-0.18.0/MoltenVK/include)
    link_directories(~/Downloads/Molten-0.18.0/MoltenVK/macOS)

    find_library(COCOA Cocoa REQUIRED)
    find_library(METAL Metal REQUIRED)
    find_library(QUARTZ_CORE QuartzCore REQUIRED)
    find_package(glfw3 3.3 REQUIRED)

    set(VULKAN_LIB "/Users/gordath/Downloads/Molten-0.18.0/MoltenVK/macOS/libMoltenVK.dylib")
    set(LIBRARIES ${COCOA} ${METAL} ${QUARTZ_CORE} ${VULKAN_LIB})
elseif(MSVC)
    #setup visual studio paths
elseif(MINGW)
    #handle mingw
else()
    set(VULKAN_LIB vulkan)
    find_package(glfw3 3.2 REQUIRED)
    set(LIBRARIES ${VULKAN_LIB} glfw)
endif()

add_library(Infrastructure ${SOURCE_FILES})

target_link_libraries(Infrastructure ${LIBRARIES})
