#include "vk/graphics.h"
#include "vk/exception.h"
#include "vk/shader.h"

#include <array>
#include <vector>

namespace vk::graphics {

namespace {

constexpr std::array kDynamicStates = {
  VK_DYNAMIC_STATE_VIEWPORT,
  VK_DYNAMIC_STATE_SCISSOR
};

} // namespace

Pipeline::Layout::Layout(VkDevice logical_device) : logical_device_(logical_device), layout_() {
  VkPipelineLayoutCreateInfo pipeline_layout_info = {};
  pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipeline_layout_info.setLayoutCount = 0;
  pipeline_layout_info.pushConstantRangeCount = 0;

  if (vkCreatePipelineLayout(logical_device_, &pipeline_layout_info, nullptr, &layout_) != VK_SUCCESS) {
    THROW_UNEXPECTED("failed to create pipeline layout");
  }
}

Pipeline::Layout::~Layout() {
  vkDestroyPipelineLayout(logical_device_, layout_, nullptr);
}

Pipeline::Pipeline(
  VkDevice logical_device,
  VkPipelineLayout layout,
  VkRenderPass pass
) : logical_device_(logical_device), pipeline_() {
  auto vert_shader_module = shader::Module(logical_device_, "shaders/vert.spv");
  auto frag_shader_module = shader::Module(logical_device_,"shaders/frag.spv");
  std::vector<VkPipelineShaderStageCreateInfo> shader_stages;
  for(const shader::Stage& stage : {
    shader::Stage{ VK_SHADER_STAGE_VERTEX_BIT,
      vert_shader_module.Get(),
      "main" },
    shader::Stage{ VK_SHADER_STAGE_FRAGMENT_BIT,
      frag_shader_module.Get(),
      "main" }
  }) {
    VkPipelineShaderStageCreateInfo stage_info = {};
    stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stage_info.stage = stage.bits;
    stage_info.module = stage.module;
    stage_info.pName = stage.name.data();
    shader_stages.push_back(stage_info);
  }
  VkPipelineVertexInputStateCreateInfo vertex_input_info = {};
  vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertex_input_info.vertexBindingDescriptionCount = 0;
  vertex_input_info.vertexAttributeDescriptionCount = 0;

  VkPipelineInputAssemblyStateCreateInfo input_assembly = {};
  input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  input_assembly.primitiveRestartEnable = VK_FALSE;

  VkPipelineViewportStateCreateInfo viewport_state = {};
  viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewport_state.viewportCount = 1;
  viewport_state.scissorCount = 1;

  VkPipelineRasterizationStateCreateInfo rasterizer = {};
  rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rasterizer.depthClampEnable = VK_FALSE;
  rasterizer.rasterizerDiscardEnable = VK_FALSE;
  rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
  rasterizer.lineWidth = 1.0f;
  rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
  rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
  rasterizer.depthBiasEnable = VK_FALSE;

  VkPipelineMultisampleStateCreateInfo multisampling = {};
  multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisampling.sampleShadingEnable = VK_FALSE;
  multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

  VkPipelineColorBlendAttachmentState color_blend_attachment = {};
  color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  color_blend_attachment.blendEnable = VK_FALSE;

  VkPipelineColorBlendStateCreateInfo color_blending = {};
  color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  color_blending.logicOpEnable = VK_FALSE;
  color_blending.logicOp = VK_LOGIC_OP_COPY;
  color_blending.attachmentCount = 1;
  color_blending.pAttachments = &color_blend_attachment;
  color_blending.blendConstants[0] = 0.0f;
  color_blending.blendConstants[1] = 0.0f;
  color_blending.blendConstants[2] = 0.0f;
  color_blending.blendConstants[3] = 0.0f;

  VkPipelineDynamicStateCreateInfo dynamic_state = {};
  dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamic_state.dynamicStateCount = static_cast<uint32_t>(kDynamicStates.size());
  dynamic_state.pDynamicStates = kDynamicStates.data();

  VkGraphicsPipelineCreateInfo pipelineInfo = {};
  pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipelineInfo.stageCount = 2;
  pipelineInfo.pStages = shader_stages.data();
  pipelineInfo.pVertexInputState = &vertex_input_info;
  pipelineInfo.pInputAssemblyState = &input_assembly;
  pipelineInfo.pViewportState = &viewport_state;
  pipelineInfo.pRasterizationState = &rasterizer;
  pipelineInfo.pMultisampleState = &multisampling;
  pipelineInfo.pColorBlendState = &color_blending;
  pipelineInfo.pDynamicState = &dynamic_state;
  pipelineInfo.layout = layout;
  pipelineInfo.renderPass = pass;
  pipelineInfo.subpass = 0;
  pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

  if (vkCreateGraphicsPipelines(logical_device_, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline_) != VK_SUCCESS) {
    THROW_UNEXPECTED("failed to create graphics pipeline");
  }
}

Pipeline::~Pipeline() {
  vkDestroyPipeline(logical_device_, pipeline_, nullptr);
}


} // nanamespace vk