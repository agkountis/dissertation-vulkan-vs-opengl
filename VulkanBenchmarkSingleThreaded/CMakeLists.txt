set(SOURCE_FILES main.cpp
        vulkan_single_threaded_application.cpp
        vulkan_single_threaded_application.h)

include_directories(../Infrastructure/Core)
include_directories(../Infrastructure/Vulkan)

add_executable(VulkanBenchmarkSingleThreaded ${SOURCE_FILES})

target_link_libraries(VulkanBenchmarkSingleThreaded VulkanInfrastructure)
