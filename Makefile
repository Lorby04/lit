# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.22

# Default target executed when no arguments are given to make.
default_target: all
.PHONY : default_target

# Allow only one "make -f Makefile2" at a time, but pass parallelism.
.NOTPARALLEL:

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
CMAKE_SOURCE_DIR = /mnt/c/Projects/sgx/lit

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /mnt/c/Projects/sgx/lit

#=============================================================================
# Targets provided globally by CMake.

# Special rule for the target edit_cache
edit_cache:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "No interactive CMake dialog available..."
	/usr/bin/cmake -E echo No\ interactive\ CMake\ dialog\ available.
.PHONY : edit_cache

# Special rule for the target edit_cache
edit_cache/fast: edit_cache
.PHONY : edit_cache/fast

# Special rule for the target rebuild_cache
rebuild_cache:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "Running CMake to regenerate build system..."
	/usr/bin/cmake --regenerate-during-build -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR)
.PHONY : rebuild_cache

# Special rule for the target rebuild_cache
rebuild_cache/fast: rebuild_cache
.PHONY : rebuild_cache/fast

# The main all target
all: cmake_check_build_system
	$(CMAKE_COMMAND) -E cmake_progress_start /mnt/c/Projects/sgx/lit/CMakeFiles /mnt/c/Projects/sgx/lit//CMakeFiles/progress.marks
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 all
	$(CMAKE_COMMAND) -E cmake_progress_start /mnt/c/Projects/sgx/lit/CMakeFiles 0
.PHONY : all

# The main clean target
clean:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 clean
.PHONY : clean

# The main clean target
clean/fast: clean
.PHONY : clean/fast

# Prepare targets for installation.
preinstall: all
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 preinstall
.PHONY : preinstall

# Prepare targets for installation.
preinstall/fast:
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 preinstall
.PHONY : preinstall/fast

# clear depends
depend:
	$(CMAKE_COMMAND) -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR) --check-build-system CMakeFiles/Makefile.cmake 1
.PHONY : depend

#=============================================================================
# Target rules for targets named /mnt/c/Projects/sgx/lit/bin/lit

# Build rule for target.
bin/lit: cmake_check_build_system
	$(MAKE) $(MAKESILENT) -f CMakeFiles/Makefile2 bin/lit
.PHONY : bin/lit

# fast build rule for target.
bin/lit/fast:
	$(MAKE) $(MAKESILENT) -f CMakeFiles//mnt/c/Projects/sgx/lit/bin/lit.dir/build.make CMakeFiles//mnt/c/Projects/sgx/lit/bin/lit.dir/build
.PHONY : bin/lit/fast

src/li_service.o: src/li_service.cpp.o
.PHONY : src/li_service.o

# target to build an object file
src/li_service.cpp.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles//mnt/c/Projects/sgx/lit/bin/lit.dir/build.make CMakeFiles//mnt/c/Projects/sgx/lit/bin/lit.dir/src/li_service.cpp.o
.PHONY : src/li_service.cpp.o

src/li_service.i: src/li_service.cpp.i
.PHONY : src/li_service.i

# target to preprocess a source file
src/li_service.cpp.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles//mnt/c/Projects/sgx/lit/bin/lit.dir/build.make CMakeFiles//mnt/c/Projects/sgx/lit/bin/lit.dir/src/li_service.cpp.i
.PHONY : src/li_service.cpp.i

src/li_service.s: src/li_service.cpp.s
.PHONY : src/li_service.s

# target to generate assembly for a file
src/li_service.cpp.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles//mnt/c/Projects/sgx/lit/bin/lit.dir/build.make CMakeFiles//mnt/c/Projects/sgx/lit/bin/lit.dir/src/li_service.cpp.s
.PHONY : src/li_service.cpp.s

src/li_target.o: src/li_target.cpp.o
.PHONY : src/li_target.o

# target to build an object file
src/li_target.cpp.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles//mnt/c/Projects/sgx/lit/bin/lit.dir/build.make CMakeFiles//mnt/c/Projects/sgx/lit/bin/lit.dir/src/li_target.cpp.o
.PHONY : src/li_target.cpp.o

src/li_target.i: src/li_target.cpp.i
.PHONY : src/li_target.i

# target to preprocess a source file
src/li_target.cpp.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles//mnt/c/Projects/sgx/lit/bin/lit.dir/build.make CMakeFiles//mnt/c/Projects/sgx/lit/bin/lit.dir/src/li_target.cpp.i
.PHONY : src/li_target.cpp.i

src/li_target.s: src/li_target.cpp.s
.PHONY : src/li_target.s

# target to generate assembly for a file
src/li_target.cpp.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles//mnt/c/Projects/sgx/lit/bin/lit.dir/build.make CMakeFiles//mnt/c/Projects/sgx/lit/bin/lit.dir/src/li_target.cpp.s
.PHONY : src/li_target.cpp.s

src/lit.o: src/lit.cpp.o
.PHONY : src/lit.o

# target to build an object file
src/lit.cpp.o:
	$(MAKE) $(MAKESILENT) -f CMakeFiles//mnt/c/Projects/sgx/lit/bin/lit.dir/build.make CMakeFiles//mnt/c/Projects/sgx/lit/bin/lit.dir/src/lit.cpp.o
.PHONY : src/lit.cpp.o

src/lit.i: src/lit.cpp.i
.PHONY : src/lit.i

# target to preprocess a source file
src/lit.cpp.i:
	$(MAKE) $(MAKESILENT) -f CMakeFiles//mnt/c/Projects/sgx/lit/bin/lit.dir/build.make CMakeFiles//mnt/c/Projects/sgx/lit/bin/lit.dir/src/lit.cpp.i
.PHONY : src/lit.cpp.i

src/lit.s: src/lit.cpp.s
.PHONY : src/lit.s

# target to generate assembly for a file
src/lit.cpp.s:
	$(MAKE) $(MAKESILENT) -f CMakeFiles//mnt/c/Projects/sgx/lit/bin/lit.dir/build.make CMakeFiles//mnt/c/Projects/sgx/lit/bin/lit.dir/src/lit.cpp.s
.PHONY : src/lit.cpp.s

# Help Target
help:
	@echo "The following are some of the valid targets for this Makefile:"
	@echo "... all (the default if no target is provided)"
	@echo "... clean"
	@echo "... depend"
	@echo "... edit_cache"
	@echo "... rebuild_cache"
	@echo "... /mnt/c/Projects/sgx/lit/bin/lit"
	@echo "... src/li_service.o"
	@echo "... src/li_service.i"
	@echo "... src/li_service.s"
	@echo "... src/li_target.o"
	@echo "... src/li_target.i"
	@echo "... src/li_target.s"
	@echo "... src/lit.o"
	@echo "... src/lit.i"
	@echo "... src/lit.s"
.PHONY : help



#=============================================================================
# Special targets to cleanup operation of make.

# Special rule to run CMake to check the build system integrity.
# No rule that depends on this can have commands that come from listfiles
# because they might be regenerated.
cmake_check_build_system:
	$(CMAKE_COMMAND) -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR) --check-build-system CMakeFiles/Makefile.cmake 0
.PHONY : cmake_check_build_system

