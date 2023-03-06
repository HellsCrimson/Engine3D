cmake_minimum_required(VERSION 3.22)

project(3DEngine)

find_package(SDL2 NAMES SDL2 REQUIRED)

add_executable(3DEngine src/Engine3D.cpp src/Matrix.cpp src/Utils.cpp
                        src/Vector.cpp)

target_include_directories(3DEngine PUBLIC include/)

target_link_libraries(3DEngine SDL2::SDL2)

set_target_properties(3DEngine PROPERTIES CXX_STANDARD 20 CXX_EXTENSIONS OFF)

target_compile_options(3DEngine PRIVATE -Wall -Wextra -pedantic -std=c++20
                                        -DUNICODE)

file(COPY font DESTINATION ${CMAKE_BINARY_DIR})
