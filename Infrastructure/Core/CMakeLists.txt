set(SOURCE_FILES application.cpp
        application.h
        entity.h
        logger.h
        resource.h
        resource_manager.h
        timer.h
        types.h
        window.cpp
        window.h
		vertex.h
		mesh.h
		mesh.cpp
		texture.h
		stb_image.h 
		entity.cpp 
		mesh_utilities.h 
		mesh_utilities.cpp
		cfg.h
		cfg.cpp
		dllmain.c
		ncf.h
		ncf.cpp
		util.h
		util.cpp
		declspec.h
		thread_pool.h
		thread_pool.cpp)

add_library(CoreInfrastructure ${SOURCE_FILES})

if(UNIX)
	target_link_libraries(CoreInfrastructure Threads::Threads)
endif()

if(MSVC)
	set_target_properties(CoreInfrastructure PROPERTIES FOLDER Infrastructure)
endif()
