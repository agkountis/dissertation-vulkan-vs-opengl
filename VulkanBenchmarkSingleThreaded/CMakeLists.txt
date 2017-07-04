project(VulkanBenchmarkSingleThreaded)

set(SOURCE_FILES main.cpp
        vulkan_single_threaded_application.cpp
        vulkan_single_threaded_application.h)

find_package(glfw3 3.2 REQUIRED)

include_directories(../Infrastructure)
include_directories(~/Downloads/Molten-0.18.0/MoltenVK/include)

add_executable(VulkanBenchmarkSingleThreaded ${SOURCE_FILES})

target_link_libraries(VulkanBenchmarkSingleThreaded Infrastructure)
