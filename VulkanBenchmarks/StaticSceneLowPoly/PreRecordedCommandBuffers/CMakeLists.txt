set(SOURCE_FILES main.cpp
        demo_application.cpp
        demo_application.h
        demo_entity.h
        demo_entity.cpp
        demo_material.h
        demo_scene.h
        demo_scene.cpp)

include_directories(../../../Infrastructure/Core)
include_directories(../../../Infrastructure/Vulkan)

add_executable(VulkanStaticSceneLowPolyPreRecordedCmdBuffers ${SOURCE_FILES})

target_link_libraries(VulkanStaticSceneLowPolyPreRecordedCmdBuffers VulkanInfrastructure)
