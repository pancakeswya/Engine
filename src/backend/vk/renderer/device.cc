#include "backend/vk/renderer/device.h"

#include <array>
#include <set>

#include "backend/vk/renderer/buffer.h"
#include "backend/vk/renderer/error.h"
#include "backend/vk/renderer/image.h"
#include "backend/vk/renderer/instance.h"
#include "backend/vk/renderer/internal.h"
#include "backend/vk/renderer/shader.h"
#include "backend/vk/renderer/swapchain.h"

namespace vk {

SurfaceSupportDetails Device::GetSurfaceSupport(VkPhysicalDevice physical_device, VkSurfaceKHR surface) {
  SurfaceSupportDetails details;
  if (const VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &details.capabilities); result != VK_SUCCESS) {
    throw Error("failed to get physical device surface capabilities").WithCode(result);
  }
  uint32_t format_count;
  if (const VkResult result = vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, nullptr); result != VK_SUCCESS) {
    throw Error("failed to get physical device surface formats count").WithCode(result);
  }

  if (format_count != 0) {
    details.formats.resize(format_count);
    if (const VkResult result = vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, details.formats.data()); result != VK_SUCCESS) {
      throw Error("failed to get physical device surface formats").WithCode(result);
    }
  }
  uint32_t present_mode_count;
  if (const VkResult result = vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, nullptr); result != VK_SUCCESS) {
    throw Error("failed to get physical device surface present modes count").WithCode(result);
  }

  if (present_mode_count != 0) {
    details.present_modes.resize(present_mode_count);
    if (const VkResult result = vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, details.present_modes.data()); result != VK_SUCCESS) {
      throw Error("failed to get physical device surface present modes").WithCode(result);
    }
  }
  return details;
}

Device::Device(VkPhysicalDevice physical_device, const QueueFamilyIndices& indices, const std::vector<const char*>& extensions, const VkAllocationCallbacks* allocator)
  : logical_device_(VK_NULL_HANDLE), physical_device_(physical_device), allocator_(allocator), indices_(indices) {
  std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
  std::set unique_family_ids = {
    indices.graphic,
    indices.present
  };
  constexpr float queue_priority = 1.0f;
  for(const unsigned int family_idx : unique_family_ids) {
    VkDeviceQueueCreateInfo queue_create_info = {};
    queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_info.queueFamilyIndex = family_idx;
    queue_create_info.queueCount = 1;
    queue_create_info.pQueuePriorities = &queue_priority;
    queue_create_infos.push_back(queue_create_info);
  }
#ifdef DEBUG
  const std::vector<const char*> layers = Instance::GetLayers();
#endif // DEBUG
  VkPhysicalDeviceFeatures device_features = {};
  device_features.samplerAnisotropy = VK_TRUE;

  VkDeviceCreateInfo create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());
  create_info.pQueueCreateInfos = queue_create_infos.data();
  create_info.pEnabledFeatures = &device_features;
  create_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
  create_info.ppEnabledExtensionNames = extensions.data();
#ifdef DEBUG
  create_info.enabledLayerCount = static_cast<uint32_t>(layers.size());
  create_info.ppEnabledLayerNames = layers.data();
#endif // DEBUG
  create_info.enabledLayerCount = 0;

  if (const VkResult result = vkCreateDevice(physical_device, &create_info, allocator, &logical_device_); result != VK_SUCCESS) {
    throw Error("failed to create logical device").WithCode(result);
  }
}

Device::Device(Device &&other) noexcept
    : logical_device_(other.logical_device_), physical_device_(other.physical_device_),
      allocator_(other.allocator_), indices_(other.indices_) {
  other.logical_device_ = VK_NULL_HANDLE;
  other.physical_device_ = VK_NULL_HANDLE;
  other.allocator_ = nullptr;
  other.indices_ = {};
}

Device::~Device() {
  if (logical_device_ != nullptr) {
    vkDestroyDevice(logical_device_, allocator_);
  }
}

Device& Device::operator=(Device&& other) noexcept {
  if (this != &other) {
    logical_device_ = std::exchange(other.logical_device_, VK_NULL_HANDLE);
    physical_device_ = std::exchange(other.physical_device_, VK_NULL_HANDLE);
    allocator_ = std::exchange(other.allocator_, nullptr);
    indices_ = std::exchange(other.indices_, {});
  }
  return *this;
}

ShaderModule Device::CreateShaderModule(const ShaderInfo& shader_info) const {
  VkShaderModuleCreateInfo create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  create_info.codeSize = shader_info.spirv.size() * sizeof(uint32_t);
  create_info.pCode = shader_info.spirv.data();

  VkShaderModule module = VK_NULL_HANDLE;
  if (const VkResult result = vkCreateShaderModule(logical_device_, &create_info, allocator_, &module); result != VK_SUCCESS) {
    throw Error("failed to create shader module!").WithCode(result);
  }
  return {
    module,
    logical_device_,
    allocator_,
    shader_info
  };
}

Device::Dispatchable<VkRenderPass> Device::CreateRenderPass(const VkFormat image_format, const VkFormat depth_format) const {
  VkAttachmentDescription color_attachment = {};

  color_attachment.format = image_format;
  color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
  color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  VkAttachmentDescription depth_attachment = {};
  depth_attachment.format = depth_format;
  depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
  depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  VkAttachmentReference color_attachment_ref = {};
  color_attachment_ref.attachment = 0;
  color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkAttachmentReference depth_attachment_ref = {};
  depth_attachment_ref.attachment = 1;
  depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpass = {};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &color_attachment_ref;
  subpass.pDepthStencilAttachment = &depth_attachment_ref;

  VkSubpassDependency dependency = {};
  dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  dependency.dstSubpass = 0;
  dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  dependency.srcAccessMask = 0;
  dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

  const std::array attachments = {color_attachment, depth_attachment};

  VkRenderPassCreateInfo render_pass_info = {};
  render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  render_pass_info.attachmentCount = static_cast<uint32_t>(attachments.size());
  render_pass_info.pAttachments = attachments.data();
  render_pass_info.subpassCount = 1;
  render_pass_info.pSubpasses = &subpass;
  render_pass_info.dependencyCount = 1;
  render_pass_info.pDependencies = &dependency;

  VkRenderPass render_pass = VK_NULL_HANDLE;
  if (const VkResult result = vkCreateRenderPass(logical_device_, &render_pass_info, allocator_, &render_pass); result != VK_SUCCESS) {
    throw Error("failed to create render pass").WithCode(result);
  }
  return {
    render_pass,
    logical_device_,
    vkDestroyRenderPass,
    allocator_
  };
}

Device::Dispatchable<VkPipelineLayout> Device::CreatePipelineLayout(const std::vector<VkDescriptorSetLayout>& descriptor_set_layouts) const {
  VkPipelineLayoutCreateInfo pipeline_layout_info = {};
  pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipeline_layout_info.setLayoutCount = descriptor_set_layouts.size();
  pipeline_layout_info.pSetLayouts = descriptor_set_layouts.data();

  VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;
  if (const VkResult result = vkCreatePipelineLayout(logical_device_, &pipeline_layout_info, allocator_, &pipeline_layout); result != VK_SUCCESS) {
    throw Error("failed to create pipeline layout").WithCode(result);
  }
  return {
    pipeline_layout,
    logical_device_,
    vkDestroyPipelineLayout,
    allocator_
  };
}

Device::Dispatchable<VkPipeline> Device::CreatePipeline(VkPipelineLayout pipeline_layout, VkRenderPass render_pass, const std::vector<VkVertexInputAttributeDescription>& attribute_descriptions, const std::vector<VkVertexInputBindingDescription>& binding_descriptions, const std::vector<ShaderModule>& shaders) const {
  std::vector<VkPipelineShaderStageCreateInfo> shader_stages_infos;
  shader_stages_infos.reserve(shaders.size());
  for(const ShaderModule& shader : shaders) {
    VkPipelineShaderStageCreateInfo shader_stage_info = {};
    shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shader_stage_info.stage = shader.Stage();
    shader_stage_info.module = shader.Handle();
    shader_stage_info.pName = shader.EntryPoint().data();
    shader_stages_infos.push_back(shader_stage_info);
  }
  VkPipelineVertexInputStateCreateInfo vertex_input_info = {};
  vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertex_input_info.vertexBindingDescriptionCount = binding_descriptions.size();
  vertex_input_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(attribute_descriptions.size());
  vertex_input_info.pVertexBindingDescriptions = binding_descriptions.data();
  vertex_input_info.pVertexAttributeDescriptions = attribute_descriptions.data();

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
  rasterizer.cullMode = VK_CULL_MODE_FRONT_BIT;
  rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
  rasterizer.depthBiasEnable = VK_FALSE;

  VkPipelineMultisampleStateCreateInfo multisampling = {};
  multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisampling.sampleShadingEnable = VK_FALSE;
  multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

  VkPipelineDepthStencilStateCreateInfo depth_stencil = {};
  depth_stencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  depth_stencil.depthTestEnable = VK_TRUE;
  depth_stencil.depthWriteEnable = VK_TRUE;
  depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS;
  depth_stencil.depthBoundsTestEnable = VK_FALSE;
  depth_stencil.stencilTestEnable = VK_FALSE;

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

  const std::vector dynamic_states = {
    VK_DYNAMIC_STATE_VIEWPORT,
    VK_DYNAMIC_STATE_SCISSOR
  };

  VkPipelineDynamicStateCreateInfo dynamic_state = {};
  dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamic_state.dynamicStateCount = static_cast<uint32_t>(dynamic_states.size());
  dynamic_state.pDynamicStates = dynamic_states.data();

  VkGraphicsPipelineCreateInfo pipeline_info = {};
  pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipeline_info.stageCount = 2;
  pipeline_info.pStages = shader_stages_infos.data();
  pipeline_info.pVertexInputState = &vertex_input_info;
  pipeline_info.pInputAssemblyState = &input_assembly;
  pipeline_info.pViewportState = &viewport_state;
  pipeline_info.pRasterizationState = &rasterizer;
  pipeline_info.pDepthStencilState = &depth_stencil;
  pipeline_info.pMultisampleState = &multisampling;
  pipeline_info.pColorBlendState = &color_blending;
  pipeline_info.pDynamicState = &dynamic_state;
  pipeline_info.layout = pipeline_layout;
  pipeline_info.renderPass = render_pass;
  pipeline_info.subpass = 0;
  pipeline_info.basePipelineHandle = VK_NULL_HANDLE;

  VkPipeline pipeline = VK_NULL_HANDLE;
  if (const VkResult result = vkCreateGraphicsPipelines(logical_device_, VK_NULL_HANDLE, 1, &pipeline_info, allocator_, &pipeline); result != VK_SUCCESS) {
    throw Error("failed to create graphics pipeline").WithCode(result);
  }
  return {
    pipeline,
    logical_device_,
    vkDestroyPipeline,
    allocator_
  };
}

Device::Dispatchable<VkCommandPool> Device::CreateCommandPool() const {
  VkCommandPoolCreateInfo create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  create_info.queueFamilyIndex = indices_.graphic;
  create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

  VkCommandPool cmd_pool = VK_NULL_HANDLE;
  if (const VkResult result = vkCreateCommandPool(logical_device_, &create_info, allocator_, &cmd_pool); result != VK_SUCCESS) {
    throw Error("failed to create command pool");
  }
  return {
    cmd_pool,
    logical_device_,
    vkDestroyCommandPool,
    allocator_
  };
}

Device::Dispatchable<VkSemaphore> Device::CreateSemaphore() const {
  VkSemaphoreCreateInfo create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  VkSemaphore semaphore = VK_NULL_HANDLE;
  if (const VkResult result = vkCreateSemaphore(logical_device_, &create_info, allocator_, &semaphore); result != VK_SUCCESS) {
    throw Error("failed to create semaphore").WithCode(result);
  }
  return {
    semaphore,
    logical_device_,
    vkDestroySemaphore,
    allocator_
  };
}

Device::Dispatchable<VkFence> Device::CreateFence() const {
  VkFenceCreateInfo create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  VkFence fence = VK_NULL_HANDLE;
  if (const VkResult result = vkCreateFence(logical_device_, &create_info, allocator_, &fence); result != VK_SUCCESS) {
    throw Error("failed to create semaphore").WithCode(result);
  }
  return {
    fence,
    logical_device_,
    vkDestroyFence,
    allocator_
  };
}

Buffer Device::CreateBuffer(const VkBufferUsageFlags usage, const uint32_t data_size) const {
  VkBufferCreateInfo buffer_info = {};
  buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  buffer_info.size = data_size;
  buffer_info.usage = usage;
  buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  VkBuffer buffer = VK_NULL_HANDLE;
  if (const VkResult result = vkCreateBuffer(logical_device_, &buffer_info, allocator_, &buffer); result != VK_SUCCESS) {
    throw Error("failed to create vertex buffer").WithCode(result);
  }
  return {
    buffer,
    logical_device_,
    physical_device_,
    allocator_,
    data_size
  };
}

Device::Dispatchable<VkDescriptorSetLayout> Device::CreateUboDescriptorSetLayout() const {
  VkDescriptorSetLayoutBinding ubo_layout_binding = {};
  ubo_layout_binding.binding = 0;
  ubo_layout_binding.descriptorCount = 1;
  ubo_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  ubo_layout_binding.pImmutableSamplers = nullptr;
  ubo_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

  VkDescriptorSetLayoutCreateInfo layout_info = {};
  layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  layout_info.bindingCount = 1;
  layout_info.pBindings = &ubo_layout_binding;

  VkDescriptorSetLayout descriptor_set_layout = VK_NULL_HANDLE;
  if (const VkResult result = vkCreateDescriptorSetLayout(logical_device_, &layout_info, allocator_, &descriptor_set_layout); result != VK_SUCCESS) {
    throw Error("failed to create uniform descriptor set layout").WithCode(result);
  }
  return {
    descriptor_set_layout,
    logical_device_,
    vkDestroyDescriptorSetLayout,
    allocator_
  };
}

Device::Dispatchable<VkDescriptorSetLayout> Device::CreateSamplerDescriptorSetLayout() const {
  VkDescriptorSetLayoutBinding sampler_layout_binding = {};
  sampler_layout_binding.binding = 0;
  sampler_layout_binding.descriptorCount = 1;
  sampler_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  sampler_layout_binding.pImmutableSamplers = nullptr;
  sampler_layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

  VkDescriptorSetLayoutCreateInfo layout_info = {};
  layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  layout_info.bindingCount = 1;
  layout_info.pBindings = &sampler_layout_binding;

  VkDescriptorSetLayout descriptor_set_layout = VK_NULL_HANDLE;
  if (const VkResult result = vkCreateDescriptorSetLayout(logical_device_, &layout_info, allocator_, &descriptor_set_layout); result != VK_SUCCESS) {
    throw Error("failed to create texture descriptor set layout").WithCode(result);
  }
  return {
    descriptor_set_layout,
    logical_device_,
    vkDestroyDescriptorSetLayout,
    allocator_
  };
}

Device::Dispatchable<VkDescriptorPool> Device::CreateDescriptorPool(const size_t ubo_count, const size_t texture_count) const {
  std::array<VkDescriptorPoolSize, 2> pool_sizes = {};
  pool_sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  pool_sizes[0].descriptorCount = static_cast<uint32_t>(ubo_count);
  pool_sizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  pool_sizes[1].descriptorCount = static_cast<uint32_t>(texture_count);

  VkDescriptorPoolCreateInfo pool_info = {};
  pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  pool_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
  pool_info.pPoolSizes = pool_sizes.data();
  pool_info.maxSets = static_cast<uint32_t>(ubo_count + texture_count);

  VkDescriptorPool descriptor_pool = VK_NULL_HANDLE;
  if (const VkResult result = vkCreateDescriptorPool(logical_device_, &pool_info, allocator_, &descriptor_pool); result != VK_SUCCESS) {
    throw Error("failed to create descriptor pool").WithCode(result);
  }
  return {
    descriptor_pool,
    logical_device_,
    vkDestroyDescriptorPool,
    allocator_
  };
}

Image Device::CreateImage(const VkImageUsageFlags usage, const VkExtent2D extent, const VkFormat format, const VkImageTiling tiling, const uint32_t mip_levels) const {
  return internal::CreateImage(logical_device_, physical_device_, allocator_, usage, extent, format, tiling, mip_levels);
}

Swapchain Device::CreateSwapchain(const VkExtent2D size, VkSurfaceKHR surface) const {
  const SurfaceSupportDetails device_support_details = GetSurfaceSupport(physical_device_, surface);

  VkSurfaceFormatKHR surface_format = Swapchain::ChooseSurfaceFormat(device_support_details.formats);
  VkPresentModeKHR present_mode = Swapchain::ChoosePresentMode(device_support_details.present_modes);

  VkExtent2D extent = Swapchain::ChooseExtent(size, device_support_details.capabilities);
  VkFormat format = surface_format.format;

  uint32_t image_count = device_support_details.capabilities.minImageCount + 1;
  if (device_support_details.capabilities.maxImageCount > 0 && image_count > device_support_details.capabilities.maxImageCount) {
    image_count = device_support_details.capabilities.maxImageCount;
  }

  VkSwapchainCreateInfoKHR create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  create_info.surface = surface;
  create_info.minImageCount = image_count;
  create_info.imageFormat = surface_format.format;
  create_info.imageColorSpace = surface_format.colorSpace;
  create_info.imageExtent = extent;
  create_info.imageArrayLayers = 1;
  create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  const std::array queue_family_indices = {indices_.graphic, indices_.present};

  if (indices_.graphic != indices_.present) {
    create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    create_info.queueFamilyIndexCount = queue_family_indices.size();
    create_info.pQueueFamilyIndices = queue_family_indices.data();
  } else {
    create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  }

  create_info.preTransform = device_support_details.capabilities.currentTransform;
  create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  create_info.presentMode = present_mode;
  create_info.clipped = VK_TRUE;

  VkSwapchainKHR swapchain = VK_NULL_HANDLE;
  if (const VkResult result = vkCreateSwapchainKHR(logical_device_, &create_info, allocator_, &swapchain); result != VK_SUCCESS) {
    throw Error("failed to create swap chain!").WithCode(result);
  }
  return {
    swapchain,
    logical_device_,
    physical_device_,
    allocator_,
    extent,
    format
  };
}

std::vector<VkCommandBuffer> Device::CreateCommandBuffers(VkCommandPool cmd_pool, uint32_t count) const {
  VkCommandBufferAllocateInfo alloc_info = {};
  alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  alloc_info.commandPool = cmd_pool;
  alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  alloc_info.commandBufferCount = count;

  std::vector<VkCommandBuffer> cmd_buffers(count);
  if (const VkResult result = vkAllocateCommandBuffers(logical_device_, &alloc_info, cmd_buffers.data()); result != VK_SUCCESS) {
    throw Error("failed to allocate command buffers").WithCode(result);
  }
  return cmd_buffers;
}

} // namespace vk