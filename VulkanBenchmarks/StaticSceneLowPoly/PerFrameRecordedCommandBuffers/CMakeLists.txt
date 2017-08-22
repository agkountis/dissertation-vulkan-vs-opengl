set(SOURCE_FILES main.cpp
        demo_entity.cpp
        demo_scene.h
        demo_scene.cpp
        demo_material.h
        demo_entity.h
        demo_application.cpp
        demo_application.h)

include_directories(../../../Infrastructure/Core)
include_directories(../../../Infrastructure/Vulkan)

add_executable(VulkanStaticSceneLowPolyPerFrameCmdBuffers ${SOURCE_FILES})

target_link_libraries(VulkanStaticSceneLowPolyPerFrameCmdBuffers VulkanInfrastructure)
