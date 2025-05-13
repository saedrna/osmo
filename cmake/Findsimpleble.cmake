include(FetchContent)
FetchContent_Declare(
    simpleble
    GIT_REPOSITORY https://github.com/simpleble/simpleble.git
    GIT_TAG v0.9.1
    GIT_SHALLOW YES
)

# Note that here we manually do what FetchContent_MakeAvailable() would do,
# except to ensure that the dependency can also get what it needs, we add
# custom logic between the FetchContent_Populate() and add_subdirectory()
# calls.
FetchContent_GetProperties(simpleble)
if(NOT simpleble_POPULATED)
    FetchContent_MakeAvailable(simpleble)
    list(APPEND CMAKE_MODULE_PATH "${simpleble_SOURCE_DIR}/cmake/find")
    add_subdirectory("${simpleble_SOURCE_DIR}/simpleble" "${simpleble_BINARY_DIR}")
endif()

set(simpleble_FOUND 1)