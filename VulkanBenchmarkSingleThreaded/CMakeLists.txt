set(SOURCE_FILES main.cpp
        vulkan_single_threaded_application.cpp
        vulkan_single_threaded_application.h)

include_directories(../Infrastructure/Core)
include_directories(../Infrastructure/Vulkan)

if(MSVC)
	include_directories(C:/usr/include)
    include_directories(C:/VulkanSDK/1.0.54.0/Include)
endif()

add_executable(VulkanBenchmarkSingleThreaded ${SOURCE_FILES})

target_link_libraries(VulkanBenchmarkSingleThreaded VulkanInfrastructure)
