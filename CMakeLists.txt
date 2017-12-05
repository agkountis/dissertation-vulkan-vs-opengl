cmake_minimum_required(VERSION 3.6)

project(dissertation)

set(CMAKE_CXX_STANDARD 17)

set(VULKAN_APP_LIBRARIES)
set(OPENGL_APP_LIBRARIES)

if(APPLE)
    include_directories(~/Downloads/Molten-0.18.0/MoltenVK/include)
    link_directories(~/Downloads/Molten-0.18.0/MoltenVK/macOS)

    find_library(COCOA Cocoa REQUIRED)
    find_library(METAL Metal REQUIRED)
    find_library(QUARTZ_CORE QuartzCore REQUIRED)
    find_package(glfw3 3.3 REQUIRED)

    set(VULKAN_LIB "/Users/gordath/Downloads/Molten-0.18.0/MoltenVK/macOS/libMoltenVK.dylib")
    set(VULKAN_APP_LIBRARIES ${LIBRARIES} ${COCOA} ${METAL} ${QUARTZ_CORE} ${VULKAN_LIB})
elseif(MSVC)
	add_compile_options("/std:c++17")
	set_property(GLOBAL PROPERTY USE_FOLDERS ON)

    include_directories(C:/usr/include)
    include_directories(C:/VulkanSDK/1.0.65.0/Include)

    set(VULKAN_LIB C:/VulkanSDK/1.0.65.0/Lib/vulkan-1.lib)
    set(GLFW_LIB C:/usr/lib/glfw3x64.lib)

    set(VULKAN_APP_LIBRARIES ${VULKAN_LIB} ${GLFW_LIB})
elseif(MINGW)
    #handle mingw
else()
    find_package(Threads REQUIRED)
    find_package(Vulkan 1.0.65.0 REQUIRED)
    include_directories(${Vulkan_INCLUDE_DIR})

    set(VULKAN_APP_LIBRARIES ${Vulkan_LIBRARY} glfw)
endif()

set(IMGUI_SOURCE ${CMAKE_SOURCE_DIR}/Dependencies/imgui/imconfig.h
${CMAKE_SOURCE_DIR}/Dependencies/imgui/imgui.cpp
${CMAKE_SOURCE_DIR}/Dependencies/imgui/imgui.h
${CMAKE_SOURCE_DIR}/Dependencies/imgui/imgui_demo.cpp
${CMAKE_SOURCE_DIR}/Dependencies/imgui/imgui_draw.cpp
${CMAKE_SOURCE_DIR}/Dependencies/imgui/imgui_internal.h
${CMAKE_SOURCE_DIR}/Dependencies/imgui/stb_rect_pack.h
${CMAKE_SOURCE_DIR}/Dependencies/imgui/stb_textedit.h
${CMAKE_SOURCE_DIR}/Dependencies/imgui/stb_truetype.h)

set(IMGUI_VULKAN_IMPL ${CMAKE_SOURCE_DIR}/Dependencies/imgui/imgui_impl_glfw_vulkan.h ${CMAKE_SOURCE_DIR}/Dependencies/imgui/imgui_impl_glfw_vulkan.cpp)

set(IMGUI_INCLUDE_PATH ${CMAKE_SOURCE_DIR}/Dependencies/imgui)

add_subdirectory(Infrastructure)
add_subdirectory(VulkanBenchmarks)
