# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 2.8

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list

# Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /root/Foot_test_stand

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /root/Foot_test_stand/build

# Include any dependencies generated for this target.
include CMakeFiles/main_app.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/main_app.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/main_app.dir/flags.make

CMakeFiles/main_app.dir/src/gui.c.o: CMakeFiles/main_app.dir/flags.make
CMakeFiles/main_app.dir/src/gui.c.o: ../src/gui.c
	$(CMAKE_COMMAND) -E cmake_progress_report /root/Foot_test_stand/build/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building C object CMakeFiles/main_app.dir/src/gui.c.o"
	/usr/bin/gcc  $(C_DEFINES) $(C_FLAGS) -o CMakeFiles/main_app.dir/src/gui.c.o   -c /root/Foot_test_stand/src/gui.c

CMakeFiles/main_app.dir/src/gui.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/main_app.dir/src/gui.c.i"
	/usr/bin/gcc  $(C_DEFINES) $(C_FLAGS) -E /root/Foot_test_stand/src/gui.c > CMakeFiles/main_app.dir/src/gui.c.i

CMakeFiles/main_app.dir/src/gui.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/main_app.dir/src/gui.c.s"
	/usr/bin/gcc  $(C_DEFINES) $(C_FLAGS) -S /root/Foot_test_stand/src/gui.c -o CMakeFiles/main_app.dir/src/gui.c.s

CMakeFiles/main_app.dir/src/gui.c.o.requires:
.PHONY : CMakeFiles/main_app.dir/src/gui.c.o.requires

CMakeFiles/main_app.dir/src/gui.c.o.provides: CMakeFiles/main_app.dir/src/gui.c.o.requires
	$(MAKE) -f CMakeFiles/main_app.dir/build.make CMakeFiles/main_app.dir/src/gui.c.o.provides.build
.PHONY : CMakeFiles/main_app.dir/src/gui.c.o.provides

CMakeFiles/main_app.dir/src/gui.c.o.provides.build: CMakeFiles/main_app.dir/src/gui.c.o

# Object files for target main_app
main_app_OBJECTS = \
"CMakeFiles/main_app.dir/src/gui.c.o"

# External object files for target main_app
main_app_EXTERNAL_OBJECTS =

main_app: CMakeFiles/main_app.dir/src/gui.c.o
main_app: CMakeFiles/main_app.dir/build.make
main_app: libfoot_stand.a
main_app: CMakeFiles/main_app.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking C executable main_app"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/main_app.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/main_app.dir/build: main_app
.PHONY : CMakeFiles/main_app.dir/build

CMakeFiles/main_app.dir/requires: CMakeFiles/main_app.dir/src/gui.c.o.requires
.PHONY : CMakeFiles/main_app.dir/requires

CMakeFiles/main_app.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/main_app.dir/cmake_clean.cmake
.PHONY : CMakeFiles/main_app.dir/clean

CMakeFiles/main_app.dir/depend:
	cd /root/Foot_test_stand/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /root/Foot_test_stand /root/Foot_test_stand /root/Foot_test_stand/build /root/Foot_test_stand/build /root/Foot_test_stand/build/CMakeFiles/main_app.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/main_app.dir/depend

