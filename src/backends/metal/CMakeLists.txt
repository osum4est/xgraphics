set(TARGET_NAME ${PROJECT_NAME}-backend-metal)

add_library(${TARGET_NAME}
        metal_buffer.h
        metal_buffer.mm
        metal_command_buffer.h
        metal_command_buffer.mm
        metal_device.h
        metal_device.mm
        metal_device_def.h
        metal_image.h
        metal_image.mm
        metal_instance.h
        metal_instance.mm
        metal_pipeline.h
        metal_pipeline.mm
        metal_render_pass.h
        metal_render_pass.mm
        metal_resource_layout.h
        metal_resource_layout.mm
        metal_resource_set.h
        metal_resource_set.mm
        metal_sampler.h
        metal_sampler.mm
        metal_shader.h
        metal_shader.mm
        metal_swapchain.h
        metal_swapchain.mm
        metal_sync_context.h
        metal_sync_context.mm
        metal_uniform_buffer.h
        metal_uniform_buffer.mm
        metal_xgraphics.h
        metal_xgraphics.mm)


target_include_directories(${TARGET_NAME} PRIVATE ${XGRAPHICS_INCLUDE_DIRS})

find_library(QUARTZCORE_LIBRARY QuartzCore)
target_link_libraries(${TARGET_NAME} PRIVATE ${QUARTZCORE_LIBRARY})
find_library(METAL_LIBRARY Metal)
target_link_libraries(${TARGET_NAME} PRIVATE ${METAL_LIBRARY})
