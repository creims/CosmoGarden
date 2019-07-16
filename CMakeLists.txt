cmake_minimum_required(VERSION 3.13)
project(CosmoGarden)

set(CMAKE_CXX_STANDARD 14)

add_executable(CosmoGarden main.cpp branch.cpp branch.h common.cpp common.h shader_utils.cpp shader_utils.h consts.h math_utils.cpp math_utils.h drawable.h bezier.cpp bezier.h tree.cpp tree.h arcball.cpp arcball.h batchdrawer.cpp batchdrawer.h)

find_package(OpenGL REQUIRED)
find_package(SDL2 REQUIRED)
find_package(GLEW REQUIRED)
include_directories( ${OPENGL_INCLUDE_DIRS} ${SDL2_INCLUDE_DIRS} ${GLEW_INCLUDE_DIR} )

target_link_libraries( CosmoGarden ${OPENGL_LIBRARIES} ${SDL2_LIBRARIES} ${GLEW_LIBRARY} )
