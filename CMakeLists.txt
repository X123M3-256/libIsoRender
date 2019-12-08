cmake_minimum_required (VERSION 3.1)
project (IsoRender C)

find_package(PNG REQUIRED)
include_directories(${PNG_INCLUDE_DIR} ../libImage/src)
link_directories(../libImage)


add_library(IsoRender STATIC src/vectormath.c src/palette.c src/renderer.c src/model.c src/objLoader/list.c src/objLoader/obj_parser.c src/objLoader/string_extra.c)
set_property(TARGET IsoRender PROPERTY C_STANDARD 99)
target_link_libraries (IsoRender Image ${PNG_LIBRARY} m)

add_executable(TestIsoRender src/main.c)
set_property(TARGET TestIsoRender PROPERTY C_STANDARD 99)
target_link_libraries (TestIsoRender IsoRender)
