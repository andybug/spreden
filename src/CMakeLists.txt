# set up compiler
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11 -g -Wall -Wextra -pedantic")
add_definitions("-D_POSIX_SOURCE")
add_definitions("-D_XOPEN_SOURCE=500")
include_directories("${CMAKE_SOURCE_DIR}/contrib")

add_executable(
  spreden
  db.c
  list.c
  main.c
  rc.c
)

target_link_libraries(
  spreden
  yajl
  guile-2.0
)

install(
  TARGETS spreden
  RUNTIME DESTINATION bin
)
