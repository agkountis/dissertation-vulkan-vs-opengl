set(SOURCE_FILES gl_application.h
				 gl_application.cpp
				 gl_window.h
				 gl_window.cpp
				 gl_mesh.h
				 gl_mesh.cpp
				 gl_shader.h
				 gl_shader.cpp
				 gl_program_pipeline.h
				 gl_program_pipeline.cpp
				 gl_texture.h
				 gl_texture.cpp
				 gl_texture_sampler.h
				 gl_texture_sampler.cpp
				 gl_infrastructure_context.h
				 gl_infrastructure_context.cpp
				 gl_render_target.h
				 gl_render_target.cpp
				 gl_buffer.h)

include_directories(../Core)

add_library(OpenGLInfrastructure ${SOURCE_FILES})

if(MSVC)
	set_target_properties(OpenGLInfrastructure PROPERTIES FOLDER Infrastructure)
endif()

target_link_libraries(OpenGLInfrastructure CoreInfrastructure ${OPENGL_APP_LIBRARIES})