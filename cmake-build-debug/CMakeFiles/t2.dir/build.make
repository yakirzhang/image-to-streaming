# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.12

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


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
CMAKE_SOURCE_DIR = /home/test/workspace/image-to-streaming

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/test/workspace/image-to-streaming/cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/t2.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/t2.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/t2.dir/flags.make

CMakeFiles/t2.dir/basic_gstreamer/t2.cpp.o: CMakeFiles/t2.dir/flags.make
CMakeFiles/t2.dir/basic_gstreamer/t2.cpp.o: ../basic_gstreamer/t2.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/test/workspace/image-to-streaming/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/t2.dir/basic_gstreamer/t2.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/t2.dir/basic_gstreamer/t2.cpp.o -c /home/test/workspace/image-to-streaming/basic_gstreamer/t2.cpp

CMakeFiles/t2.dir/basic_gstreamer/t2.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/t2.dir/basic_gstreamer/t2.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/test/workspace/image-to-streaming/basic_gstreamer/t2.cpp > CMakeFiles/t2.dir/basic_gstreamer/t2.cpp.i

CMakeFiles/t2.dir/basic_gstreamer/t2.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/t2.dir/basic_gstreamer/t2.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/test/workspace/image-to-streaming/basic_gstreamer/t2.cpp -o CMakeFiles/t2.dir/basic_gstreamer/t2.cpp.s

# Object files for target t2
t2_OBJECTS = \
"CMakeFiles/t2.dir/basic_gstreamer/t2.cpp.o"

# External object files for target t2
t2_EXTERNAL_OBJECTS =

t2: CMakeFiles/t2.dir/basic_gstreamer/t2.cpp.o
t2: CMakeFiles/t2.dir/build.make
t2: CMakeFiles/t2.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/test/workspace/image-to-streaming/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable t2"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/t2.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/t2.dir/build: t2

.PHONY : CMakeFiles/t2.dir/build

CMakeFiles/t2.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/t2.dir/cmake_clean.cmake
.PHONY : CMakeFiles/t2.dir/clean

CMakeFiles/t2.dir/depend:
	cd /home/test/workspace/image-to-streaming/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/test/workspace/image-to-streaming /home/test/workspace/image-to-streaming /home/test/workspace/image-to-streaming/cmake-build-debug /home/test/workspace/image-to-streaming/cmake-build-debug /home/test/workspace/image-to-streaming/cmake-build-debug/CMakeFiles/t2.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/t2.dir/depend

