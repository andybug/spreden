# set up compiler
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11 -g -Wall -Wextra -pedantic")

add_executable(
  spreden
  main.c
)

target_link_libraries(
  spreden
  guile-2.0
)

install(
  TARGETS spreden
  RUNTIME DESTINATION bin
)
