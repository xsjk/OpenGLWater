add_library(gl_core SHARED gl_core_3_3.h gl_core_3_3.c)
target_link_libraries(gl_core freeglut)
target_include_directories(gl_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
