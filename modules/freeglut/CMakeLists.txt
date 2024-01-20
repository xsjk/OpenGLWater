project(freeglut)

set(FREEGLUT_HEADERS
    include/GL/freeglut.h
    include/GL/freeglut_ucall.h
    include/GL/freeglut_ext.h
    include/GL/freeglut_std.h
    include/GL/glut.h
)

set(FREEGLUT_SRCS
    ${FREEGLUT_HEADERS}
    src/fg_callbacks.c
    src/fg_cursor.c
    src/fg_display.c
    src/fg_ext.c
    src/fg_font_data.c
    src/fg_gamemode.c
    src/fg_geometry.c
    src/fg_gl2.c
    src/fg_gl2.h
    src/fg_init.c
    src/fg_init.h
    src/fg_internal.h
    src/fg_callback_macros.h
    src/fg_input_devices.c
    src/fg_joystick.c
    src/fg_main.c
    src/fg_misc.c
    src/fg_overlay.c
    src/fg_spaceball.c
    src/fg_state.c
    src/fg_stroke_mono_roman.c
    src/fg_stroke_roman.c
    src/fg_structure.c
    src/fg_teapot.c
    src/fg_teapot_data.h
    src/fg_videoresize.c
    src/fg_window.c
    src/fg_font.c
    src/fg_menu.c
)

list(APPEND FREEGLUT_SRCS
    src/mswin/fg_cursor_mswin.c
    src/mswin/fg_display_mswin.c
    src/mswin/fg_ext_mswin.c
    src/mswin/fg_gamemode_mswin.c
    src/mswin/fg_init_mswin.c
    src/mswin/fg_internal_mswin.h
    src/mswin/fg_input_devices_mswin.c
    src/mswin/fg_joystick_mswin.c
    src/mswin/fg_main_mswin.c
    src/mswin/fg_menu_mswin.c
    src/mswin/fg_spaceball_mswin.c
    src/mswin/fg_state_mswin.c
    src/mswin/fg_structure_mswin.c
    src/mswin/fg_window_mswin.c
    src/mswin/fg_cmap_mswin.c
)

list(APPEND FREEGLUT_SRCS
    src/util/xparsegeometry_repl.c
    src/util/xparsegeometry_repl.h
)

add_library(freeglut SHARED ${FREEGLUT_SRCS})
target_compile_definitions(freeglut PUBLIC FREEGLUT_EXPORTS _CRT_SECURE_NO_WARNINGS)
target_include_directories(freeglut PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
find_package(OpenGL REQUIRED COMPONENTS OpenGL)
target_link_libraries(freeglut PUBLIC OpenGL::GL winmm gdi32)
