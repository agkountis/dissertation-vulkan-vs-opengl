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
		stb_image.h entity.cpp mesh_utilities.h mesh_utilities.cpp)

if(MSVC)
	include_directories(C:/usr/include)
endif()

add_library(CoreInfrastructure ${SOURCE_FILES})
