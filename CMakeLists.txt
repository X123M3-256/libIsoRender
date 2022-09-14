cmake_minimum_required (VERSION 3.1)
project (IsoRender C)

find_package(ZLIB REQUIRED)
find_package(PNG REQUIRED)

include_directories(${PNG_INCLUDE_DIR})

add_library(IsoRender STATIC src/vectormath.c src/renderer.c src/model.c src/palette.c src/raytrace.c src/image.c src/pack.c)
set_property(TARGET IsoRender PROPERTY C_STANDARD 99)
target_link_libraries (IsoRender assimp embree3 libpng.a libz.a ${MATH_LIB})

add_executable(TestIsoRender src/main.c)
set_property(TARGET TestIsoRender PROPERTY C_STANDARD 99)
target_link_libraries (TestIsoRender IsoRender)

