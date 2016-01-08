
#
#  Library Build File
#

file(GLOB_RECURSE VM_SOURCES *)
add_library(hydrogen STATIC ${VM_SOURCES})
