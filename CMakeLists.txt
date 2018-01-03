cmake_minimum_required(VERSION 3.6)

project(dissertation)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(VULKAN_APP_LIBRARIES)
set(OPENGL_APP_LIBRARIES)

set(DEPENDENCIES_PATH ${CMAKE_SOURCE_DIR}/Dependencies)

set(IMGUI_SOURCE ${DEPENDENCIES_PATH}/imgui/imconfig.h
${DEPENDENCIES_PATH}/imgui/imgui.cpp
${DEPENDENCIES_PATH}/imgui/imgui.h
${DEPENDENCIES_PATH}/imgui/imgui_demo.cpp
${DEPENDENCIES_PATH}/imgui/imgui_draw.cpp
${DEPENDENCIES_PATH}/imgui/imgui_internal.h
${DEPENDENCIES_PATH}/imgui/stb_rect_pack.h
${DEPENDENCIES_PATH}/imgui/stb_textedit.h
${DEPENDENCIES_PATH}/imgui/stb_truetype.h)

set(IMGUI_VULKAN_IMPL ${DEPENDENCIES_PATH}/imgui/imgui_impl_glfw_vulkan.h ${DEPENDENCIES_PATH}/imgui/imgui_impl_glfw_vulkan.cpp)
set(IMGUI_OPENGL_IMPL ${DEPENDENCIES_PATH}/imgui/imgui_impl_glfw_gl3.h ${DEPENDENCIES_PATH}/imgui/imgui_impl_glfw_gl3.cpp)
set(IMGUI_INCLUDE_PATH ${DEPENDENCIES_PATH}/imgui)

include_directories(${DEPENDENCIES_PATH}/include)

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

    set(VULKAN_LIB C:/VulkanSDK/1.0.65.0/Lib32/vulkan-1.lib)
	set(GL_LIB opengl32.lib)
	set(GLEW_LIB ${DEPENDENCIES_PATH}/lib/glew32.lib)
    set(GLFW_LIB C:/usr/lib/glfw3.lib)

    set(VULKAN_APP_LIBRARIES ${VULKAN_LIB} ${GLFW_LIB})
	set(OPENGL_APP_LIBRARIES ${GL_LIB} ${GLEW_LIB} ${GLFW_LIB})
	set(ASSIMP_LIBRARIES ${DEPENDENCIES_PATH}/lib/assimp.lib)

	set(CMAKE_MSVCIDE_RUN_PATH ${DEPENDENCIES_PATH}/bin)
elseif(MINGW)
    #handle mingw
else()
    set(CMAKE_CXX_FLAGS -std=gnu++17)

    find_package(Threads REQUIRED)
    find_package(Vulkan 1.0.65.0 REQUIRED)
    find_package(assimp REQUIRED)
    find_package(GLEW REQUIRED)
    find_package(OpenGL REQUIRED)
    include_directories(${Vulkan_INCLUDE_DIR})
    include_directories(${GLEW_INCLUDE_DIRS})
    include_directories(${OPENGL_INCLUDE_DIR})

    set(VULKAN_APP_LIBRARIES ${Vulkan_LIBRARY} glfw)
    set(GLEW_LIB ${GLEW_LIBRARIES})
    set(OPENGL_APP_LIBRARIES ${OPENGL_LIBRARIES} ${GLEW_LIB} glfw)
endif()

add_subdirectory(Infrastructure)
add_subdirectory(VulkanBenchmarks)
add_subdirectory(OpenGLBenchmarks)
