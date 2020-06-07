cmake_minimum_required (VERSION 3.1)
project (IsoRender C)

find_package(PNG REQUIRED)
find_package(embree 3 REQUIRED)
find_package(assimp CONFIG REQUIRED)
add_subdirectory(libImage)

if (MSVC) # assumes vcpkg
set(EMBREE embree3)
set(ASSIMP assimp::assimp)
set(MATH_LIB "")
else()
set(EMBREE embree3)
set(ASSIMP assimp)
set(MATH_LIB m)
endif()

include_directories(${PNG_INCLUDE_DIR} libImage/src)
link_directories(/usr/local/Cellar/embree/3.6.1/lib)
link_directories(/usr/local/Cellar/assimp/5.0.0/lib)
link_directories(${EMBREE_ROOT_DIR}/lib)

add_library(IsoRender STATIC src/vectormath.c src/palette.c src/model.c src/renderer.c src/raytrace.c)
set_property(TARGET IsoRender PROPERTY C_STANDARD 99)
target_link_libraries (IsoRender Image ${PNG_LIBRARY} ${EMBREE} ${ASSIMP} ${MATH_LIB})

add_executable(TestIsoRender src/main.c)
set_property(TARGET TestIsoRender PROPERTY C_STANDARD 99)
target_link_libraries (TestIsoRender IsoRender)
