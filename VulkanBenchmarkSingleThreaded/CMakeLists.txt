set(SOURCE_FILES main.cpp
        vulkan_single_threaded_application.cpp
        vulkan_single_threaded_application.h
        demo_entity.h
        demo_entity.cpp demo_material.h demo_scene.h demo_scene.cpp)

include_directories(../Infrastructure/Core)
include_directories(../Infrastructure/Vulkan)

add_executable(VulkanBenchmarkSingleThreaded ${SOURCE_FILES})

target_link_libraries(VulkanBenchmarkSingleThreaded VulkanInfrastructure)
