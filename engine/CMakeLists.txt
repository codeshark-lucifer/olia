file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

file(GLOB_RECURSE ENGINE_HEADERS CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp"
)

add_library(olia SHARED
    ${ENGINE_SOURCES}
    ${ENGINE_HEADERS}
)

target_include_directories(olia
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_link_libraries(olia
    PUBLIC
        vendor
)

set_target_properties(olia PROPERTIES
    OUTPUT_NAME "olia"
)

if(OLIA_ENABLE_VALIDATION)

    target_compile_definitions(olia
        PRIVATE
            OLIA_ENABLE_VALIDATION=1
    )

endif()