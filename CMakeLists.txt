cmake_minimum_required(VERSION 3.13)
project(CosmoGarden)

set(CMAKE_CXX_STANDARD 14)

add_executable(CosmoGarden src/main.cpp src/branch.cpp src/branch.h src/common.cpp src/common.h src/shader_utils.cpp src/shader_utils.h src/consts.h src/math_utils.cpp src/math_utils.h src/drawable.h src/bezier.cpp src/bezier.h src/tree.cpp src/tree.h src/Arcball.cpp src/Arcball.h src/batchdrawer.cpp src/batchdrawer.h src/drawable.cpp src/plant.cpp src/plant.h src/Camera.cpp src/Camera.h)

find_package(OpenGL REQUIRED)
find_package(SDL2 REQUIRED)
find_package(GLEW REQUIRED)
include_directories( ${OPENGL_INCLUDE_DIRS} ${SDL2_INCLUDE_DIRS} ${GLEW_INCLUDE_DIR} )

target_link_libraries( CosmoGarden ${OPENGL_LIBRARIES} ${SDL2_LIBRARIES} ${GLEW_LIBRARY} )
