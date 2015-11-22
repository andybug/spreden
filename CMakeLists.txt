cmake_minimum_required(VERSION 3.0)
project(spreden C)

# program versioning
set(SPREDEN_VERSION_MAJOR 0)
set(SPREDEN_VERSION_MINOR 1)

# setup output paths
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(src)
