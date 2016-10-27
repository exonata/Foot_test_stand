cmake_minimum_required(VERSION 2.8.3)
project(foot_test_stand)
#Sends the -std=c99 flag to the gcc compiler
add_definitions(-std=c99)
# Use the package PkgConfig to detect GTK+ headers/library files
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)

include_directories(include)

# Setup CMake to use GTK+, tell the compiler where to look for headers
# and to the linker where to look for libraries
include_directories(${GTK3_INCLUDE_DIRS})
link_directories(${GTK3_LIBRARY_DIRS})

# Add other flags to the compiler
add_definitions(${GTK3_CFLAGS_OTHER})

add_library(foot_stand
  src/CLI.c
  src/state.c
  src/gui.c
)
find_package( Threads )
add_executable(main_app src/gui.c)
target_link_libraries(main_app
${GTK3_INCLUDE_DIR}
rt
 BBBio
 foot_stand
 ${GLIB_LIBRARIES}
 ${GTK3_LINK_DIR}
 ${GTK3_LIBRARIES}
 ${CMAKE_THREAD_LIBS_INIT}
 )
 
 add_executable(ADC src/ADC_Example.c)
target_link_libraries(ADC
${GTK3_INCLUDE_DIR}
rt
 BBBio
 foot_stand
 ${GLIB_LIBRARIES}
 ${GTK3_LINK_DIR}
 ${GTK3_LIBRARIES}
 ${CMAKE_THREAD_LIBS_INIT}
 )