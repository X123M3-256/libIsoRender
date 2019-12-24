cmake_minimum_required (VERSION 3.1)
project (IsoRender C)

find_package(PNG REQUIRED)
include_directories(${PNG_INCLUDE_DIR} ../libImage/src)
link_directories(../libImage)


add_library(IsoRender STATIC src/vectormath.c src/palette.c src/model.c src/renderer.c src/raytrace.c)
set_property(TARGET IsoRender PROPERTY C_STANDARD 99)
target_link_libraries (IsoRender Image ${PNG_LIBRARY} embree3 assimp m)

add_executable(TestIsoRender src/main.c)
set_property(TARGET TestIsoRender PROPERTY C_STANDARD 99)
target_link_libraries (TestIsoRender IsoRender)
