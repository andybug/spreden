# set up compiler
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11 -g -Wall -Wextra -pedantic")
include_directories("${CMAKE_SOURCE_DIR}/contrib/jsmn")

add_executable(
  spreden
  list.c
  main.c
)

target_link_libraries(
  spreden
  jsmn
  guile-2.0
)

install(
  TARGETS spreden
  RUNTIME DESTINATION bin
)
