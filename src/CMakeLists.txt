# Source files configuration for Sticky Paws
#
# This CMakeLists.txt automatically discovers all .c and .h files
# in the src directory and adds them to the project.

file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS *.c)
file(GLOB_RECURSE HEADER_FILES CONFIGURE_DEPENDS *.h)

target_sources(${PROJECT_NAME} PRIVATE ${SOURCE_FILES} ${HEADER_FILES})
