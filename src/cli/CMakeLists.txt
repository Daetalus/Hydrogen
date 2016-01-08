
#
#  Command Line Interface Build File
#

file(GLOB_RECURSE CLI_SOURCES *)
add_executable(cli ${CLI_SOURCES})
target_link_libraries(cli hydrogen hystdlib)
