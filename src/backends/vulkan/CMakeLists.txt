set(TARGET_NAME ${PROJECT_NAME}-backend-vulkan)

add_library(${TARGET_NAME}
        vulkan_buffer.cpp
        vulkan_buffer.h
        vulkan_command_buffer.cpp
        vulkan_command_buffer.h
        vulkan_device.cpp
        vulkan_device.h
        vulkan_device_def.h
        vulkan_image.cpp
        vulkan_image.h
        vulkan_instance.cpp
        vulkan_memory_context.cpp
        vulkan_memory_context.h
        vulkan_pipeline.cpp
        vulkan_pipeline.h
        vulkan_render_pass.cpp
        vulkan_render_pass.h
        vulkan_resource_layout.cpp
        vulkan_resource_layout.h
        vulkan_resource_set.cpp
        vulkan_resource_set.h
        vulkan_sampler.cpp
        vulkan_sampler.h
        vulkan_shader.cpp
        vulkan_shader.h
        vulkan_swapchain.cpp
        vulkan_swapchain.h
        vulkan_sync_context.cpp
        vulkan_sync_context.h
        vulkan_uniform_buffer.cpp
        vulkan_uniform_buffer.h
        vulkan_utils.cpp
        vulkan_utils.h
        vulkan_xgraphics.cpp
        vulkan_xgraphics.h)

target_include_directories(${TARGET_NAME} PRIVATE ${XGRAPHICS_INCLUDE_DIRS})

find_package(Vulkan REQUIRED)
target_link_libraries(${TARGET_NAME} PRIVATE Vulkan::Vulkan)

FetchContent_MakeAvailable(vulkan-memory-allocator)
target_link_libraries(${TARGET_NAME} PRIVATE VulkanMemoryAllocator)

