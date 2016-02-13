# set up compiler
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11 -g -Wall -Wextra -pedantic")
add_definitions("-D_POSIX_SOURCE")
add_definitions("-D_XOPEN_SOURCE=500")
include_directories("${CMAKE_SOURCE_DIR}/contrib")

# build subtrees
add_subdirectory(database)
add_subdirectory(dstruct)
add_subdirectory(runcontrol)

add_executable(
  spreden
  spreden.c
)

target_link_libraries(
  spreden
  spreden-database
  spreden-runcontrol
  spreden-dstruct
  uuid
  yajl
  guile-2.0
)

install(
  TARGETS spreden
  RUNTIME DESTINATION bin
)
