set(SOURCE_FILES main.cpp
        demo_application.cpp
        demo_application.h
        demo_entity.h
        demo_entity.cpp
        demo_material.h
        demo_scene.h
        demo_scene.cpp
		sdr/default.vert
		sdr/default.frag)

include_directories(../../../Infrastructure/Core)
include_directories(../../../Infrastructure/Vulkan)

if(MSVC)
	set(SHADER_FILES sdr/default.vert
		sdr/default.frag)

	set(TEXTURE_FILES textures/tunnelDiff5.png
		textures/tunnelSpec5.png
		textures/tunnelNorm5.png)

	add_executable(PreRecordedCmdBuffers ${SOURCE_FILES} ${SHADER_FILES} ${TEXTURE_FILES})

	source_group(Shaders FILES ${SHADER_FILES})
	source_group(Textures FILES ${TEXTURE_FILES})

	set_target_properties(PreRecordedCmdBuffers PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
	set_target_properties(PreRecordedCmdBuffers PROPERTIES FOLDER VulkanBenchmarks/StaticSceneLowPoly)
else()
	add_executable(PreRecordedCmdBuffers ${SOURCE_FILES})
endif()

target_link_libraries(PreRecordedCmdBuffers VulkanInfrastructure)
