
#
#  Standard Library Build File
#

file(GLOB_RECURSE STDLIB_SOURCES *)
add_library(hystdlib STATIC ${STDLIB_SOURCES})
