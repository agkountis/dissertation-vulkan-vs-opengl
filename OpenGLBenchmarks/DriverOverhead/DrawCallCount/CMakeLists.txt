set(SOURCE_FILES main.cpp
		demo_application.h
		demo_application.cpp
		demo_entity.h
		demo_entity.cpp
		demo_material.h
		demo_scene.h
		demo_scene.cpp
		${IMGUI_SOURCE}
	    ${IMGUI_OPENGL_IMPL})

include_directories(../../../Infrastructure/Core)
include_directories(../../../Infrastructure/OpenGL)
include_directories(${IMGUI_INCLUDE_PATH})

if(MSVC)
	set(SHADER_FILES sdr/default.vert
		sdr/default.frag)

	set(TEXTURE_FILES ../../../Assets/opengl.jpg
		../../../Assets/opengl_spec.png
		../../../Assets/opengl_norm.png)

	set(CONFIGURATION_FILES config/config.cfg)

	source_group(Shaders FILES ${SHADER_FILES})
	source_group(Textures FILES ${TEXTURE_FILES})
	source_group(Configuration FILES ${CONFIGURATION_FILES})
	source_group(ImGUI FILES ${IMGUI_SOURCE} ${IMGUI_OPENGL_IMPL})

	add_executable(GL_DrawCallCount ${SOURCE_FILES} ${SHADER_FILES} ${TEXTURE_FILES} ${CONFIGURATION_FILES})

	set_target_properties(GL_DrawCallCount PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
	set_target_properties(GL_DrawCallCount PROPERTIES FOLDER OpenGLBenchmarks/DriverOverhead)
else()
	add_executable(GL_DrawCallCount ${SOURCE_FILES})
endif()

target_link_libraries(GL_DrawCallCount OpenGLInfrastructure)