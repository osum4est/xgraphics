cmake_minimum_required(VERSION 3.19)

option(XGRAPHICS_DEBUG "Enable debug mode" OFF)

include(FetchContent)
FetchContent_Declare(
        result
        GIT_REPOSITORY https://github.com/osum4est/result
        GIT_TAG 2309602ab42c41bb42b7005a85b324e371a4d31d
)
FetchContent_Declare(
        glslang
        GIT_REPOSITORY https://github.com/KhronosGroup/glslang
        GIT_TAG 728c689574fba7e53305b475cd57f196c1a21226
)
FetchContent_Declare(
        spirv-cross
        GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Cross
        GIT_TAG c77b09b57c27837dc2d41aa371ed3d236ce9ce47
)
FetchContent_Declare(
        spirv-tools
        GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Tools
        GIT_TAG d9446130d5165f7fafcb3599252a22e264c7d4bd
)
FetchContent_Declare(
        spirv-headers
        GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Headers
        GIT_TAG c214f6f2d1a7253bb0e9f195c2dc5b0659dc99ef
)
FetchContent_Declare(
        shaderc
        GIT_REPOSITORY https://github.com/google/shaderc
        GIT_TAG e3846cda59a85acb0c47a6cb9e6b4adbb111e54b
)
FetchContent_Declare(
        vulkan-memory-allocator
        GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator
        GIT_TAG v3.0.1
)

project(xgraphics)
set(TARGET_NAME ${PROJECT_NAME})
set(CMAKE_CXX_STANDARD 20)

add_library(${TARGET_NAME}
        include/xgraphics/graphics_config.h
        include/xgraphics/interfaces/graphics_buffer.h
        include/xgraphics/interfaces/graphics_command_buffer.h
        include/xgraphics/interfaces/graphics_device.h
        include/xgraphics/interfaces/graphics_device_def.h
        include/xgraphics/interfaces/graphics_image.h
        include/xgraphics/interfaces/graphics_instance.h
        include/xgraphics/interfaces/graphics_pipeline.h
        include/xgraphics/interfaces/graphics_render_pass.h
        include/xgraphics/interfaces/graphics_resource_layout.h
        include/xgraphics/interfaces/graphics_resource_set.h
        include/xgraphics/interfaces/graphics_sampler.h
        include/xgraphics/interfaces/graphics_shader.h
        include/xgraphics/interfaces/graphics_swapchain.h
        include/xgraphics/interfaces/graphics_uniform_buffer.h
        include/xgraphics/shaders/intermediate_shader.h
        include/xgraphics/shaders/shader_binary.h
        include/xgraphics/shaders/shader_compiler.h
        include/xgraphics/shaders/shader_data.h
        include/xgraphics/shaders/shader_info.h
        include/xgraphics/xgraphics.h
        include/xgraphics/xgraphics.h
        include/xgraphics/xgraphics_backend.h
        src/backends/common/macos/macos_graphics_helpers.h
        src/backends/common/macos/macos_graphics_helpers.mm
        src/backends/common/xgraphics_utils.cpp
        src/backends/common/xgraphics_utils.h
        src/interfaces/graphics_buffer.cpp
        src/interfaces/graphics_command_buffer.cpp
        src/interfaces/graphics_device.cpp
        src/interfaces/graphics_image.cpp
        src/interfaces/graphics_instance.cpp
        src/interfaces/graphics_pipeline.cpp
        src/interfaces/graphics_render_pass.cpp
        src/interfaces/graphics_resource_layout.cpp
        src/interfaces/graphics_resource_set.cpp
        src/interfaces/graphics_sampler.cpp
        src/interfaces/graphics_shader.cpp
        src/interfaces/graphics_swapchain.cpp
        src/interfaces/graphics_uniform_buffer.cpp
        src/shaders/backends/metal_shader_compiler.cpp
        src/shaders/backends/vulkan_shader_compiler.cpp
        src/shaders/intermediate_shader.cpp
        src/shaders/shader_binary.cpp
        src/shaders/shader_compiler.cpp
        src/shaders/shader_data.cpp
        src/xgraphics.cpp)

if (${XGRAPHICS_DEBUG})
    target_compile_definitions(${TARGET_NAME} PUBLIC XGRAPHICS_DEBUG)
endif ()

set(CMAKE_MACOSX_RPATH ON)
set(SHADERC_SKIP_TESTS ON)
set(SHADERC_SKIP_INSTALL ON)
set(SHADERC_SKIP_EXAMPLES ON)
set(SHADERC_SKIP_COPYRIGHT_CHECK ON)
set(SKIP_GLSLANG_INSTALL ON)
set(SPIRV_HEADERS_SKIP_EXAMPLES ON)
set(SPIRV_HEADERS_SKIP_INSTALL ON)
FetchContent_MakeAvailable(spirv-headers spirv-tools glslang shaderc)
target_link_libraries(${TARGET_NAME} PRIVATE shaderc)

set(SPIRV_CROSS_ENABLE_TESTS OFF)
set(SPIRV_CROSS_SKIP_INSTALL ON)
FetchContent_MakeAvailable(spirv-cross)
target_link_libraries(${TARGET_NAME} PRIVATE spirv-cross-msl)

FetchContent_MakeAvailable(result)
target_link_libraries(${TARGET_NAME} PUBLIC result)

set(XGRAPHICS_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(${TARGET_NAME} PUBLIC ${XGRAPHICS_INCLUDE_DIRS})

if (APPLE)
    add_subdirectory(src/backends/metal)
    target_link_libraries(${TARGET_NAME} PRIVATE ${PROJECT_NAME}-backend-metal)
    target_link_libraries(${PROJECT_NAME}-backend-metal PRIVATE result)
    target_compile_definitions(${TARGET_NAME} PUBLIC XGRAPHICS_MACOS)
    target_compile_definitions(${TARGET_NAME} PUBLIC XGRAPHICS_METAL_SUPPORTED)
endif ()

# TODO: Optional
find_package(Vulkan REQUIRED)
add_subdirectory(src/backends/vulkan)
target_link_libraries(${TARGET_NAME} PRIVATE ${PROJECT_NAME}-backend-vulkan)
target_link_libraries(${PROJECT_NAME}-backend-vulkan PRIVATE result)
target_compile_definitions(${TARGET_NAME} PUBLIC XGRAPHICS_VULKAN_SUPPORTED)