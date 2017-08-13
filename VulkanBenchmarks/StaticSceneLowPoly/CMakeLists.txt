set(SOURCE_FILES main.cpp
        static_scene_low_poly_application.cpp
        static_scene_low_poly_application.h
        demo_entity.h
        demo_entity.cpp demo_material.h demo_scene.h demo_scene.cpp)

include_directories(../../Infrastructure/Core)
include_directories(../../Infrastructure/Vulkan)

add_executable(VulkanStaticSceneLowPoly ${SOURCE_FILES})

target_link_libraries(VulkanStaticSceneLowPoly VulkanInfrastructure)
