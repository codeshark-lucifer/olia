cmake_minimum_required(VERSION 3.16)

project(olia
    VERSION 1.0.0
    LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY
    ${CMAKE_BINARY_DIR}/bin
)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY
    ${CMAKE_BINARY_DIR}/bin
)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY
    ${CMAKE_BINARY_DIR}/lib
)

option(
    OLIA_ENABLE_VALIDATION
    "Enable Vulkan validation layers"
    OFF
)

add_subdirectory(vendor)
add_subdirectory(engine)
add_subdirectory(editor)