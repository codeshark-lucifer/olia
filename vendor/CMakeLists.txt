set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)

set(SDL_TESTS OFF CACHE BOOL "" FORCE)
set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
set(SDL_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(SDL)
add_subdirectory(vulkan)
add_subdirectory(glm)
add_subdirectory(stb)

add_library(vendor INTERFACE)

target_link_libraries(vendor
    INTERFACE
        SDL3::SDL3-static
        vulkan
        glm
        stb
)

target_include_directories(vendor
    INTERFACE
        "${CMAKE_CURRENT_SOURCE_DIR}/vulkan/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/SDL/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/glm"
        "${CMAKE_CURRENT_SOURCE_DIR}/stb/include"
)