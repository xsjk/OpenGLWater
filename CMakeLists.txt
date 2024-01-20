cmake_minimum_required(VERSION 3.5)

project(OpenGLWater)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})
foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_SOURCE_DIR})
endforeach(OUTPUTCONFIG)

add_subdirectory(modules)

file(GLOB SOURCES src/*.cpp)
add_executable(main ${SOURCES})
target_link_libraries(main PRIVATE gl_core stb_image glm)
target_include_directories(main PRIVATE include)
