# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.22

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
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
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/alex/Operating-Systems-labs/Sachuk.Alex/lab2

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/alex/Operating-Systems-labs/Sachuk.Alex/lab2/build

# Utility rule file for host_fifo_autogen.

# Include any custom commands dependencies for this target.
include CMakeFiles/host_fifo_autogen.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/host_fifo_autogen.dir/progress.make

CMakeFiles/host_fifo_autogen:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/alex/Operating-Systems-labs/Sachuk.Alex/lab2/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Automatic MOC and UIC for target host_fifo"
	/usr/bin/cmake -E cmake_autogen /home/alex/Operating-Systems-labs/Sachuk.Alex/lab2/build/CMakeFiles/host_fifo_autogen.dir/AutogenInfo.json Debug

host_fifo_autogen: CMakeFiles/host_fifo_autogen
host_fifo_autogen: CMakeFiles/host_fifo_autogen.dir/build.make
.PHONY : host_fifo_autogen

# Rule to build all files generated by this target.
CMakeFiles/host_fifo_autogen.dir/build: host_fifo_autogen
.PHONY : CMakeFiles/host_fifo_autogen.dir/build

CMakeFiles/host_fifo_autogen.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/host_fifo_autogen.dir/cmake_clean.cmake
.PHONY : CMakeFiles/host_fifo_autogen.dir/clean

CMakeFiles/host_fifo_autogen.dir/depend:
	cd /home/alex/Operating-Systems-labs/Sachuk.Alex/lab2/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/alex/Operating-Systems-labs/Sachuk.Alex/lab2 /home/alex/Operating-Systems-labs/Sachuk.Alex/lab2 /home/alex/Operating-Systems-labs/Sachuk.Alex/lab2/build /home/alex/Operating-Systems-labs/Sachuk.Alex/lab2/build /home/alex/Operating-Systems-labs/Sachuk.Alex/lab2/build/CMakeFiles/host_fifo_autogen.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/host_fifo_autogen.dir/depend

