file(GLOB_RECURSE EDITOR_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

file(GLOB_RECURSE EDITOR_HEADERS CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp"
)

add_executable(application
    ${EDITOR_SOURCES}
    ${EDITOR_HEADERS}
)

target_include_directories(application
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_link_libraries(application
    PRIVATE
        olia
)

# ============================================================
# Vulkan Validation
# ============================================================

# if(OLIA_ENABLE_VALIDATION)

#     target_compile_definitions(application
#         PRIVATE
#             OLIA_ENABLE_VALIDATION=1
#     )

# endif()

# ============================================================
# Editor Assets
# ============================================================

if(EXISTS "${CMAKE_SOURCE_DIR}/assets")

    add_custom_command(
        TARGET application
        POST_BUILD

        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/assets"
            "$<TARGET_FILE_DIR:application>/assets"

        COMMENT "Copying assets to application directory..."
    )

endif()

# ============================================================
# Copy Engine DLL & Validation Layers
# ============================================================

add_custom_command(
    TARGET application
    POST_BUILD

    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:olia>"
        "$<TARGET_FILE_DIR:application>/"

    COMMENT "Copying olia.dll to application directory..."
)

if(EXISTS "${CMAKE_SOURCE_DIR}/vendor/vulkan/bin/VkLayer_khronos_validation.dll")

    add_custom_command(
        TARGET application
        POST_BUILD

        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_SOURCE_DIR}/vendor/vulkan/bin/VkLayer_khronos_validation.dll"
            "$<TARGET_FILE_DIR:application>/"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_SOURCE_DIR}/vendor/vulkan/bin/VkLayer_khronos_validation.json"
            "$<TARGET_FILE_DIR:application>/"

        COMMENT "Copying Vulkan validation layer binaries to application directory..."
    )

endif()