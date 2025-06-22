CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(PA4 CXX)

IF(NOT CMAKE_BUILD_TYPE)
	#SET(CMAKE_BUILD_TYPE Release)
    SET(CMAKE_BUILD_TYPE Debug)
ENDIF()

ADD_SUBDIRECTORY(deps/vecmath)

find_package(OpenMP REQUIRED)

SET(PA4_SOURCES
        src/direction.cpp
	src/fxaa.cpp
	src/image.cpp
        src/main.cpp
        src/mesh.cpp
        src/scene_parser.cpp
	src/texture.cpp
	src/tracing_Whitted.cpp
	src/tracing_MC.cpp)

SET(PA4_INCLUDES
	include/stb_image.h
        include/camera.hpp
	include/curve.hpp
	include/direction.hpp
	include/fxaa.hpp
        include/group.hpp
        include/hit.hpp
        include/image.hpp
        include/light.hpp
        include/material.hpp
        include/mesh.hpp
        include/object3d.hpp
        include/plane.hpp
        include/ray.hpp
	include/revsurface.hpp
        include/scene_parser.hpp
        include/sphere.hpp
	include/texture.hpp
        include/transform.hpp
	include/tracing_Whitted.hpp
	include/tracing_MC.hpp
        include/triangle.hpp)

SET(CMAKE_CXX_STANDARD 17)
ADD_EXECUTABLE(${PROJECT_NAME} ${PA4_SOURCES} ${PA4_INCLUDES})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} vecmath gomp)
TARGET_COMPILE_OPTIONS(${PROJECT_NAME} PRIVATE -O3 -Wno-unused-result -fopenmp)
TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME} PRIVATE include)
