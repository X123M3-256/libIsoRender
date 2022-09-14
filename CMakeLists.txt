cmake_minimum_required (VERSION 3.1)
project (IsoRender C)

find_package(ZLIB REQUIRED)
find_package(PNG REQUIRED)

include_directories(${PNG_INCLUDE_DIR})

add_library(IsoRender STATIC src/vectormath.c src/renderer.c src/model.c src/palette.c src/raytrace.c src/image.c src/pack.c)
set_property(TARGET IsoRender PROPERTY C_STANDARD 99)

#Use static libpng and zlib on Windows because the dynamic version doesn't work for some reason
IF (WIN32)
	target_link_libraries (IsoRender png.a z.a)
ELSE()
	target_link_libraries (IsoRender ${PNG_LIBRARIES} ${ZLIB_LIBRARIES})
ENDIF()

target_link_libraries (IsoRender m assimp embree3)

#Test program TODO why doesn't this need to be linked with assimp and embree?
add_executable(TestIsoRender src/main.c)
set_property(TARGET TestIsoRender PROPERTY C_STANDARD 99)
target_link_libraries (TestIsoRender IsoRender)

