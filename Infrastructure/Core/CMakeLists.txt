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
		math_utilities.h
		texture.h
		stb_image.h)

if(MSVC)
	include_directories(C:/usr/include)
endif()

add_library(CoreInfrastructure ${SOURCE_FILES})
