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

if(MSVC)
	set(SHADER_FILES sdr/default.vert
		sdr/default.frag)

	set(TEXTURE_FILES textures/tunnelDiff5.png
		textures/tunnelSpec5.png
		textures/tunnelNorm5.png)

	set(CONFIGURATION_FILES config/config.cfg)

	add_executable(MTPrimaryCmdBuffers ${SOURCE_FILES} ${SHADER_FILES} ${TEXTURE_FILES} ${CONFIGURATION_FILES})

	source_group(Shaders FILES ${SHADER_FILES})
	source_group(Textures FILES ${TEXTURE_FILES})
	source_group(Configuration FILES ${CONFIGURATION_FILES})

	set_target_properties(MTPrimaryCmdBuffers PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
	set_target_properties(MTPrimaryCmdBuffers PROPERTIES FOLDER VulkanBenchmarks/CommandBufferRecording)
else()
	add_executable(MTPrimaryCmdBuffers ${SOURCE_FILES})
endif()

target_link_libraries(MTPrimaryCmdBuffers VulkanInfrastructure)
