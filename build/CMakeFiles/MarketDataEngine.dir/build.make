# CMAKE generated file: DO NOT EDIT!
# Generated by "MinGW Makefiles" Generator, CMake Version 3.29

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

SHELL = cmd.exe

# The CMake executable.
CMAKE_COMMAND = "C:\Program Files\CMake\bin\cmake.exe"

# The command to remove a file.
RM = "C:\Program Files\CMake\bin\cmake.exe" -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = C:\Users\katka\source\market_data_handler\Real-Time-Market-Data-Feed-Handler-and-Order-Matching-Engine

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = C:\Users\katka\source\market_data_handler\Real-Time-Market-Data-Feed-Handler-and-Order-Matching-Engine\build

# Include any dependencies generated for this target.
include CMakeFiles/MarketDataEngine.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/MarketDataEngine.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/MarketDataEngine.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/MarketDataEngine.dir/flags.make

CMakeFiles/MarketDataEngine.dir/src/main.cpp.obj: CMakeFiles/MarketDataEngine.dir/flags.make
CMakeFiles/MarketDataEngine.dir/src/main.cpp.obj: CMakeFiles/MarketDataEngine.dir/includes_CXX.rsp
CMakeFiles/MarketDataEngine.dir/src/main.cpp.obj: C:/Users/katka/source/market_data_handler/Real-Time-Market-Data-Feed-Handler-and-Order-Matching-Engine/src/main.cpp
CMakeFiles/MarketDataEngine.dir/src/main.cpp.obj: CMakeFiles/MarketDataEngine.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=C:\Users\katka\source\market_data_handler\Real-Time-Market-Data-Feed-Handler-and-Order-Matching-Engine\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/MarketDataEngine.dir/src/main.cpp.obj"
	C:\Users\katka\Downloads\winlibs-x86_64-posix-seh-gcc-14.2.0-llvm-18.1.8-mingw-w64ucrt-12.0.0-r1\mingw64\bin\c++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/MarketDataEngine.dir/src/main.cpp.obj -MF CMakeFiles\MarketDataEngine.dir\src\main.cpp.obj.d -o CMakeFiles\MarketDataEngine.dir\src\main.cpp.obj -c C:\Users\katka\source\market_data_handler\Real-Time-Market-Data-Feed-Handler-and-Order-Matching-Engine\src\main.cpp

CMakeFiles/MarketDataEngine.dir/src/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/MarketDataEngine.dir/src/main.cpp.i"
	C:\Users\katka\Downloads\winlibs-x86_64-posix-seh-gcc-14.2.0-llvm-18.1.8-mingw-w64ucrt-12.0.0-r1\mingw64\bin\c++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E C:\Users\katka\source\market_data_handler\Real-Time-Market-Data-Feed-Handler-and-Order-Matching-Engine\src\main.cpp > CMakeFiles\MarketDataEngine.dir\src\main.cpp.i

CMakeFiles/MarketDataEngine.dir/src/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/MarketDataEngine.dir/src/main.cpp.s"
	C:\Users\katka\Downloads\winlibs-x86_64-posix-seh-gcc-14.2.0-llvm-18.1.8-mingw-w64ucrt-12.0.0-r1\mingw64\bin\c++.exe $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S C:\Users\katka\source\market_data_handler\Real-Time-Market-Data-Feed-Handler-and-Order-Matching-Engine\src\main.cpp -o CMakeFiles\MarketDataEngine.dir\src\main.cpp.s

# Object files for target MarketDataEngine
MarketDataEngine_OBJECTS = \
"CMakeFiles/MarketDataEngine.dir/src/main.cpp.obj"

# External object files for target MarketDataEngine
MarketDataEngine_EXTERNAL_OBJECTS =

MarketDataEngine.exe: CMakeFiles/MarketDataEngine.dir/src/main.cpp.obj
MarketDataEngine.exe: CMakeFiles/MarketDataEngine.dir/build.make
MarketDataEngine.exe: C:/curl/lib/libcurl.a
MarketDataEngine.exe: CMakeFiles/MarketDataEngine.dir/linkLibs.rsp
MarketDataEngine.exe: CMakeFiles/MarketDataEngine.dir/objects1.rsp
MarketDataEngine.exe: CMakeFiles/MarketDataEngine.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir=C:\Users\katka\source\market_data_handler\Real-Time-Market-Data-Feed-Handler-and-Order-Matching-Engine\build\CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable MarketDataEngine.exe"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles\MarketDataEngine.dir\link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/MarketDataEngine.dir/build: MarketDataEngine.exe
.PHONY : CMakeFiles/MarketDataEngine.dir/build

CMakeFiles/MarketDataEngine.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles\MarketDataEngine.dir\cmake_clean.cmake
.PHONY : CMakeFiles/MarketDataEngine.dir/clean

CMakeFiles/MarketDataEngine.dir/depend:
	$(CMAKE_COMMAND) -E cmake_depends "MinGW Makefiles" C:\Users\katka\source\market_data_handler\Real-Time-Market-Data-Feed-Handler-and-Order-Matching-Engine C:\Users\katka\source\market_data_handler\Real-Time-Market-Data-Feed-Handler-and-Order-Matching-Engine C:\Users\katka\source\market_data_handler\Real-Time-Market-Data-Feed-Handler-and-Order-Matching-Engine\build C:\Users\katka\source\market_data_handler\Real-Time-Market-Data-Feed-Handler-and-Order-Matching-Engine\build C:\Users\katka\source\market_data_handler\Real-Time-Market-Data-Feed-Handler-and-Order-Matching-Engine\build\CMakeFiles\MarketDataEngine.dir\DependInfo.cmake "--color=$(COLOR)"
.PHONY : CMakeFiles/MarketDataEngine.dir/depend

