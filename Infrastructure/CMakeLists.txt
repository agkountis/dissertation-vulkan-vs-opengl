project(Infrastructure)

set(SOURCE_FILES vulkan_vbo.h
        vulkan_renderer.cpp
        vulkan_device.cpp
        vulkan_instance.cpp
        types.h
        vbo.h
        vulkan_utilities.h
        vulkan_window.h
        vulkan_application.cpp
        vulkan_debug.h
        mesh.h
        vulkan_debug.cpp
        ibo.h
        vulkan_instance.h
        logger.h
        vulkan_utilities.cpp
        vulkan_renderer.h
        window.h
        vulkan_window.cpp
        application.h
        application.cpp
        vulkan_device.h
        vulkan_application.h
        timer.h
        vulkan_vbo.cpp
        window.cpp
        vulkan_physical_device.h
        vulkan_swapchain.h
        vulkan_swapchain.cpp
        vulkan_physical_device.cpp
        vulkan_buffer.h
        vulkan_buffer.cpp
        vulkan_pipeline_cache.cpp
        vulkan_command_pool.cpp
        vulkan_render_pass.cpp
        vulkan_depth_stencil.cpp
        vulkan_command_pool.h
        vulkan_depth_stencil.h
        vulkan_render_pass.h
        vulkan_semaphore.cpp
        vulkan_pipeline_cache.h
        vulkan_semaphore.h vulkan_framebuffer.h vulkan_framebuffer.cpp)

if(APPLE)
    include_directories(~/Downloads/Molten-0.18.0/MoltenVK/include)
    link_directories(~/Downloads/Molten-0.18.0/MoltenVK/macOS)

    find_library(COCOA Cocoa REQUIRED)
    find_library(METAL Metal REQUIRED)
    find_library(QUARTZ_CORE QuartzCore REQUIRED)

    set(MOLTEN_VK "/Users/gordath/Downloads/Molten-0.18.0/MoltenVK/macOS/libMoltenVK.dylib")
elseif(MSVC)
    #setup visual studio paths
elseif(MINGW)
    #handle mingw
else()
    #handle linux
endif()




find_package(glfw3 3.3 REQUIRED)

add_library(Infrastructure ${SOURCE_FILES})

target_link_libraries(Infrastructure ${COCOA} ${METAL} ${QUARTZ_CORE} stdc++ ${MOLTEN_VK} glfw)
