cmake_minimum_required (VERSION 3.1)
project (IsoRender C)

find_package(ZLIB REQUIRED)
find_package(PNG REQUIRED)
find_package(embree REQUIRED)
find_package(assimp REQUIRED)

link_directories(${PNG_LIBRARY_DIRS} ${ZLIB_LIBRARY_DIRS})
include_directories(${PNG_INCLUDE_DIRS} ${PNG_INCLUDE_DIRS})

add_library(IsoRender STATIC src/vectormath.c src/renderer.c src/model.c src/palette.c src/raytrace.c src/image.c src/pack.c)
set_property(TARGET IsoRender PROPERTY C_STANDARD 99)

#Use static libpng and zlib on Windows because the dynamic version doesn't work for some reason
IF (WIN32)
	target_link_libraries (IsoRender -static)
ENDIF()

target_link_libraries(IsoRender m ${PNG_LIBRARIES} ${ZLIB_LIBRARIES})

add_executable(TestIsoRender src/main.c)
set_property(TARGET TestIsoRender PROPERTY C_STANDARD 99)
target_link_libraries (TestIsoRender IsoRender assimp::assimp embree)
