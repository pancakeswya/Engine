#include "vk/render.h"
#include "base/io.h"

#include <stdlib.h>

static const VkDynamicState kPipelineDynamicStates[] = {
  VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
static const uint32_t kPipelineDynamicStatesCount =
    sizeof(kPipelineDynamicStates) / sizeof(VkDynamicState);

static Error renderPassCreate(VkDevice logical_device,
                              VkFormat format,
                              VkRenderPass* render_pass) {
  const VkAttachmentDescription color_attachment = {
      .format = format,
      .samples = VK_SAMPLE_COUNT_1_BIT,
      .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
      .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
      .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
      .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
      .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
      .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR};
  const VkAttachmentReference color_attachment_ref = {
      .attachment = 0, .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
  const VkSubpassDescription subpass = {
      .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
      .colorAttachmentCount = 1,
      .pColorAttachments = &color_attachment_ref};
  const VkRenderPassCreateInfo render_pass_info = {
      .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
      .attachmentCount = 1,
      .pAttachments = &color_attachment,
      .subpassCount = 1,
      .pSubpasses = &subpass};
  const VkResult vk_res =
      vkCreateRenderPass(logical_device, &render_pass_info, NULL, render_pass);
  if (vk_res != VK_SUCCESS) {
    return VulkanErrorCreate(vk_res);
  }
  return kSuccess;
}

static Error pipelineLayoutCreate(VkDevice logical_device,
                                  VkPipelineLayout* pipeline_layout) {
  const VkPipelineLayoutCreateInfo pipeline_layout_info = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      .setLayoutCount = 0,
      .pushConstantRangeCount = 0};
  const VkResult vk_res = vkCreatePipelineLayout(
      logical_device, &pipeline_layout_info, NULL, pipeline_layout);
  if (vk_res != VK_SUCCESS) {
    return VulkanErrorCreate(vk_res);
  }
  return kSuccess;
}

static Error shaderModuleCreate(VkDevice logical_device, const char* path,
                                VkShaderModule* module) {
  size_t read = 0;
  char* code = NULL;
  const Error err = ReadFile(path, &code, &read);
  if (!ErrorEqual(err, kSuccess)) {
    return err;
  }
  const VkShaderModuleCreateInfo create_info = {
      .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      .codeSize = read,
      .pCode = (const uint32_t*)code};
  VkResult vk_res =
      vkCreateShaderModule(logical_device, &create_info, NULL, module);
  free(code);
  if (vk_res != VK_SUCCESS) {
    return VulkanErrorCreate(vk_res);
  }
  return kSuccess;
}

static Error pipelineCreate(VkDevice logical_device,
                            VkPipelineLayout pipeline_layout,
                            VkRenderPass render_pass,
                            VkPipeline* pipeline) {
  VkShaderModule vert_shader_module = VK_NULL_HANDLE;
  VkShaderModule frag_shader_module = VK_NULL_HANDLE;
  Error err = shaderModuleCreate(logical_device, "shaders/vert.spv",
                                 &vert_shader_module);
  if (!ErrorEqual(err, kSuccess)) {
    return err;
  }
  err = shaderModuleCreate(logical_device, "shaders/frag.spv",
                           &frag_shader_module);
  if (!ErrorEqual(err, kSuccess)) {
    vkDestroyShaderModule(logical_device, vert_shader_module, NULL);
    return err;
  }
  const VkPipelineShaderStageCreateInfo shader_stages[] = {
      {.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
       .stage = VK_SHADER_STAGE_VERTEX_BIT,
       .module = vert_shader_module,
       .pName = "main"},
      {.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
       .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
       .module = frag_shader_module,
       .pName = "main"}};
  const uint32_t shader_stages_count =
      sizeof(shader_stages) / sizeof(VkPipelineShaderStageCreateInfo);
  const VkPipelineVertexInputStateCreateInfo vertex_input_info = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
      .vertexBindingDescriptionCount = 0,
      .vertexAttributeDescriptionCount = 0};

  const VkPipelineInputAssemblyStateCreateInfo input_assembly = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
      .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
      .primitiveRestartEnable = VK_FALSE};

  const VkPipelineViewportStateCreateInfo viewport_state = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
      .viewportCount = 1,
      .scissorCount = 1};
  VkPipelineRasterizationStateCreateInfo rasterizer = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
      .depthClampEnable = VK_FALSE,
      .rasterizerDiscardEnable = VK_FALSE,
      .polygonMode = VK_POLYGON_MODE_FILL,
      .lineWidth = 1.0f,
      .cullMode = VK_CULL_MODE_BACK_BIT,
      .frontFace = VK_FRONT_FACE_CLOCKWISE,
      .depthBiasEnable = VK_FALSE};
  const VkPipelineMultisampleStateCreateInfo multisampling = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
      .sampleShadingEnable = VK_FALSE,
      .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT};
  const VkPipelineColorBlendAttachmentState color_blend_attachment = {
      .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
      .blendEnable = VK_FALSE};
  const VkPipelineColorBlendStateCreateInfo color_blending = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
      .logicOpEnable = VK_FALSE,
      .logicOp = VK_LOGIC_OP_COPY,
      .attachmentCount = 1,
      .pAttachments = &color_blend_attachment,
      .blendConstants[0] = 0.0f,
      .blendConstants[1] = 0.0f,
      .blendConstants[2] = 0.0f,
      .blendConstants[3] = 0.0f};
  const VkPipelineDynamicStateCreateInfo dynamic_state = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
      .dynamicStateCount = kPipelineDynamicStatesCount,
      .pDynamicStates = kPipelineDynamicStates};
  const VkGraphicsPipelineCreateInfo pipeline_info = {
      .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
      .stageCount = shader_stages_count,
      .pStages = shader_stages,
      .pVertexInputState = &vertex_input_info,
      .pInputAssemblyState = &input_assembly,
      .pViewportState = &viewport_state,
      .pRasterizationState = &rasterizer,
      .pMultisampleState = &multisampling,
      .pColorBlendState = &color_blending,
      .pDynamicState = &dynamic_state,
      .layout = pipeline_layout,
      .renderPass = render_pass,
      .subpass = 0,
      .basePipelineHandle = VK_NULL_HANDLE};
  VkResult vk_res = vkCreateGraphicsPipelines(logical_device, VK_NULL_HANDLE, 1,
                                              &pipeline_info, NULL, pipeline);
  vkDestroyShaderModule(logical_device, vert_shader_module, NULL);
  vkDestroyShaderModule(logical_device, frag_shader_module, NULL);
  if (vk_res != VK_SUCCESS) {
    return VulkanErrorCreate(vk_res);
  }
  return kSuccess;
}

Error VulkanRenderCreate(VkDevice logical_device, VkFormat format, VulkanRender* render) {
  Error err = renderPassCreate(logical_device, format, &render->pass);
  if (!ErrorEqual(err, kSuccess)) {
    return err;
  }
  err = pipelineLayoutCreate(logical_device, &render->pipeline_layout);
  if (!ErrorEqual(err, kSuccess)) {
    vkDestroyRenderPass(logical_device, render->pass, NULL);
    return err;
  }
  err = pipelineCreate(logical_device, render->pipeline_layout,
                       render->pass, &render->pipeline);
  if (!ErrorEqual(err, kSuccess)) {
    vkDestroyPipelineLayout(logical_device, render->pipeline_layout, NULL);
    vkDestroyRenderPass(logical_device, render->pass, NULL);
    return err;
  }
  return kSuccess;
}

void VulkanRenderDestroy(VkDevice logical_device, VulkanRender* render) {
  if (render->pipeline != VK_NULL_HANDLE) {
    vkDestroyPipeline(logical_device, render->pipeline, NULL);
  }
  if (render->pipeline_layout != VK_NULL_HANDLE) {
    vkDestroyPipelineLayout(logical_device, render->pipeline_layout, NULL);
  }
  if (render->pass != VK_NULL_HANDLE) {
    vkDestroyRenderPass(logical_device, render->pass, NULL);
  }
}
