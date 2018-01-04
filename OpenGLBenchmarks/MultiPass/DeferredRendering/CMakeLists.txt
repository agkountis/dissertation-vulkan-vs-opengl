set(SOURCE_FILES main.cpp
        demo_entity.cpp
        demo_scene.h
        demo_scene.cpp
        demo_material.h
        demo_entity.h
        demo_application.cpp
        demo_application.h
	   ${IMGUI_SOURCE}
	   ${IMGUI_OPENGL_IMPL})

include_directories(../../../Infrastructure/Core)
include_directories(../../../Infrastructure/OpenGL)
include_directories(${IMGUI_INCLUDE_PATH})

if(MSVC)
	set(SHADER_FILES sdr/display.vert
		sdr/display.frag sdr/deferred.vert sdr/deferred.frag)

	set(TEXTURE_FILES ../../../Assets/diff2.jpg
		)

	set(CONFIGURATION_FILES config/config.cfg)

	add_executable(GL_DeferredRendering ${SOURCE_FILES} ${SHADER_FILES} ${TEXTURE_FILES} ${CONFIGURATION_FILES})

	source_group(Shaders FILES ${SHADER_FILES})
	source_group(Textures FILES ${TEXTURE_FILES})
	source_group(Configuration FILES ${CONFIGURATION_FILES})
	source_group(ImGUI FILES ${IMGUI_SOURCE} ${IMGUI_OPENGL_IMPL})

	set_target_properties(GL_DeferredRendering PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
	set_target_properties(GL_DeferredRendering PROPERTIES FOLDER OpenGLBenchmarks/MultiPass)
else()
	add_executable(GL_DeferredRendering ${SOURCE_FILES})
endif()

target_link_libraries(GL_DeferredRendering OpenGLInfrastructure ${ASSIMP_LIBRARIES})
