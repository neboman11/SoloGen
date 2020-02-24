cmake_minimum_required(VERSION 3.16)
project(SoloGen)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include_directories(~/include)

include(CheckIncludeFileCXX)

check_include_file_cxx("AudioFile/AudioFile.h" AUDIOFILE_FOUND)
check_include_file_cxx("tclap/CmdLine.h" TCLAP_FOUND)

add_executable(SoloGen SoloGen.cpp)