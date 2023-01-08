#include "vulkan_pipeline.h"

#include "vulkan_render_pass.h"
#include "vulkan_resource_layout.h"
#include "vulkan_resource_set.h"
#include "vulkan_shader.h"
#include "vulkan_utils.h"

vulkan_pipeline::vulkan_pipeline(const graphics_pipeline_init& init, VkDevice device, VkPipeline pipeline)
    : graphics_pipeline(init), _device(device), _pipeline(pipeline) { }

vulkan_pipeline::~vulkan_pipeline() {
    vkDestroyPipeline(_device, _pipeline, nullptr);
}

result::ptr<graphics_pipeline> vulkan_pipeline::create(const graphics_pipeline_init& init, VkDevice device) {
    const auto& render_pass = (const vulkan_render_pass&) init.render_pass;

    // Create shader stage state
    std::vector<VkPipelineShaderStageCreateInfo> shader_stage_info;
    for (const auto& stage : init.layout.stages()) {
        const auto& native_shader = (const vulkan_shader*) stage;
        VkPipelineShaderStageCreateInfo stage_info = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = vulkan_utils::vk_shader_stage(stage->info().kind).get(),
            .module = native_shader->module(),
            .pName = native_shader->info().entry_point.c_str(),
        };

        shader_stage_info.push_back(stage_info);
    }

    // Create dynamic state
    std::vector<VkDynamicState> dynamic_states = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dynamic_state_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = (uint32_t) dynamic_states.size(),
        .pDynamicStates = dynamic_states.data(),
    };

    // Create vertex input state
    std::vector<VkVertexInputBindingDescription> binding_descriptions;
    std::vector<VkVertexInputAttributeDescription> attribute_descriptions;

    for (int i = 0; i < init.vertex_bindings.size(); i++) {
        const auto& binding = init.vertex_bindings[i];
        binding_descriptions.push_back({
            .binding = (uint32_t) i,
            .stride = binding.stride,
            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
        });

        for (const auto& attribute : binding.attributes) {
            VkFormat format;
            switch (attribute.format) {
                case attribute_format::float_32:
                    format = VK_FORMAT_R32_SFLOAT;
                    break;
                case attribute_format::int_32:
                    format = VK_FORMAT_R32_SINT;
                    break;
                case attribute_format::uint_32:
                    format = VK_FORMAT_R32_UINT;
                    break;
                default:
                    return result::err("Unsupported attribute format");
            }

            attribute_descriptions.push_back({
                .location = attribute.ref->backend_location,
                .binding = (uint32_t) i,
                .format = (VkFormat) (format + ((int) attribute.dimension - 1) * 3),
                .offset = attribute.offset,
            });
        }
    }

    VkPipelineVertexInputStateCreateInfo vertex_input_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = (uint32_t) binding_descriptions.size(),
        .pVertexBindingDescriptions = binding_descriptions.data(),
        .vertexAttributeDescriptionCount = (uint32_t) attribute_descriptions.size(),
        .pVertexAttributeDescriptions = attribute_descriptions.data(),
    };

    // Create input assembly state
    VkPipelineInputAssemblyStateCreateInfo input_assembly_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, // TODO: Save in init?
        .primitiveRestartEnable = VK_FALSE,
    };

    // Create viewport state
    VkPipelineViewportStateCreateInfo viewport_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .pViewports = nullptr,
        .scissorCount = 1,
        .pScissors = nullptr,
    };

    // Create rasterizer state
    VkPipelineRasterizationStateCreateInfo rasterizer_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable = VK_FALSE, // TODO: Save in init?
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,  // TODO: Save in init?
        .cullMode = VK_CULL_MODE_NONE,        // TODO: Save in init?
        .frontFace = VK_FRONT_FACE_CLOCKWISE, // TODO: Save in init?
        .depthBiasEnable = VK_FALSE,
        .depthBiasConstantFactor = 0.0f,
        .depthBiasClamp = 0.0f,
        .depthBiasSlopeFactor = 0.0f,
        .lineWidth = 1.0f,
    };

    // Create multisampling state
    VkPipelineMultisampleStateCreateInfo multisampling_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        .sampleShadingEnable = VK_FALSE, // TODO: Save in init?
        .minSampleShading = 1.0f,
        .pSampleMask = nullptr,
        .alphaToCoverageEnable = VK_FALSE,
        .alphaToOneEnable = VK_FALSE,
    };

    // Create color blending attachment state
    VkPipelineColorBlendAttachmentState color_blend_attachment = {
        .blendEnable = VK_TRUE,
        .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
        .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        .colorBlendOp = VK_BLEND_OP_ADD,
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
        .alphaBlendOp = VK_BLEND_OP_ADD,
        .colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
    };

    // Create color blending state
    VkPipelineColorBlendStateCreateInfo color_blend_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable = VK_FALSE,
        .logicOp = VK_LOGIC_OP_COPY,
        .attachmentCount = 1,
        .pAttachments = &color_blend_attachment,
        .blendConstants = {0.0f, 0.0f, 0.0f, 0.0f},
    };

    // Create depth stencil state
    VkPipelineDepthStencilStateCreateInfo depth_stencil_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .depthTestEnable = VK_TRUE,
        .depthWriteEnable = VK_TRUE,
        .depthCompareOp = VK_COMPARE_OP_LESS,
        .depthBoundsTestEnable = VK_FALSE,
        .stencilTestEnable = VK_FALSE,
    };

    // Create pipeline
    VkGraphicsPipelineCreateInfo pipeline_info = {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .stageCount = (uint32_t) shader_stage_info.size(),
        .pStages = shader_stage_info.data(),
        .pVertexInputState = &vertex_input_info,
        .pInputAssemblyState = &input_assembly_info,
        .pViewportState = &viewport_info,
        .pRasterizationState = &rasterizer_info,
        .pMultisampleState = &multisampling_info,
        .pDepthStencilState = &depth_stencil_info,
        .pColorBlendState = &color_blend_info,
        .pDynamicState = &dynamic_state_info,
        .layout = ((const vulkan_resource_layout&) init.layout).layout(),
        .renderPass = render_pass.render_pass(),
        .subpass = 0,
        .basePipelineHandle = VK_NULL_HANDLE,
        .basePipelineIndex = -1,
    };

    VkPipeline pipeline;
    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &pipeline) != VK_SUCCESS)
        return result::err("Failed to create graphics pipeline");

    return result::ok(new vulkan_pipeline(init, device, pipeline));
}

VkPipeline vulkan_pipeline::pipeline() const {
    return _pipeline;
}