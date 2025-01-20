#include "backend/vk/renderer/device_dispatchable_factory.h"

#include <array>
#include <set>

#include "backend/vk/renderer/error.h"

namespace vk {

namespace {

int32_t FindMemoryType(const uint32_t type_filter, VkPhysicalDevice physical_device, VkMemoryPropertyFlags properties) {
  VkPhysicalDeviceMemoryProperties mem_properties;
  vkGetPhysicalDeviceMemoryProperties(physical_device, &mem_properties);

  for (int32_t i = 0; i < mem_properties.memoryTypeCount; i++) {
    if ((type_filter & (1 << i)) && (mem_properties.memoryTypes[i].propertyFlags & properties) == properties) {
      return i;
    }
  }
  throw Error("failed to find suitable memory type!");
}

} // namespace

template<typename Handle, typename HandleInfo>
DeviceDispatchable<Handle> DeviceDispatchableFactory::ExecuteCreate(const DeviceCreateFunc<Handle, HandleInfo> create_func, const DeviceDestroyFunc<Handle> destroy_func, const HandleInfo* handle_info) const {
  Handle handle = VK_NULL_HANDLE;
  VkDevice logical_device = device_.GetLogical();
  const VkAllocationCallbacks* allocator = device_.GetAllocationCallbacks();
  if (const VkResult result = create_func(logical_device, handle_info, allocator, &handle); result != VK_SUCCESS) {
    throw Error("failed to create render pass").WithCode(result);
  }
  return {
    handle,
    logical_device,
    destroy_func,
    allocator
  };
}

ShaderModule DeviceDispatchableFactory::CreateShaderModule(const ShaderInfo& shader_info) const {
  VkShaderModuleCreateInfo create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  create_info.codeSize = shader_info.spirv.size() * sizeof(uint32_t);
  create_info.pCode = shader_info.spirv.data();

  return {
    ExecuteCreate<VkShaderModule>(vkCreateShaderModule, vkDestroyShaderModule, &create_info),
    shader_info
  };
}

DeviceDispatchable<VkRenderPass> DeviceDispatchableFactory::CreateRenderPass(const VkFormat image_format, const VkFormat depth_format) const {
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

  return ExecuteCreate<VkRenderPass>(vkCreateRenderPass, vkDestroyRenderPass, &render_pass_info);
}

DeviceDispatchable<VkPipelineLayout> DeviceDispatchableFactory::CreatePipelineLayout(const std::vector<VkDescriptorSetLayout>& descriptor_set_layouts) const {
  VkPipelineLayoutCreateInfo pipeline_layout_info = {};
  pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipeline_layout_info.setLayoutCount = descriptor_set_layouts.size();
  pipeline_layout_info.pSetLayouts = descriptor_set_layouts.data();

  return ExecuteCreate<VkPipelineLayout>(vkCreatePipelineLayout, vkDestroyPipelineLayout, &pipeline_layout_info);
}

DeviceDispatchable<VkPipeline> DeviceDispatchableFactory::CreatePipeline(VkPipelineLayout pipeline_layout, VkRenderPass render_pass, const std::vector<VkVertexInputAttributeDescription>& attribute_descriptions, const std::vector<VkVertexInputBindingDescription>& binding_descriptions, const std::vector<ShaderModule>& shaders) const {
  std::vector<VkPipelineShaderStageCreateInfo> shader_stages_infos;
  shader_stages_infos.reserve(shaders.size());
  for(const ShaderModule& shader : shaders) {
    VkPipelineShaderStageCreateInfo shader_stage_info = {};
    shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shader_stage_info.stage = shader.Stage();
    shader_stage_info.module = shader.GetHandle();
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
  VkDevice logical_device = device_.GetLogical();
  const VkAllocationCallbacks* allocator = device_.GetAllocationCallbacks();
  if (const VkResult result = vkCreateGraphicsPipelines(logical_device, VK_NULL_HANDLE, 1, &pipeline_info, allocator, &pipeline); result != VK_SUCCESS) {
    throw Error("failed to create graphics pipeline").WithCode(result);
  }
  return {
    pipeline,
    logical_device,
    vkDestroyPipeline,
    allocator
  };
}

DeviceDispatchable<VkCommandPool> DeviceDispatchableFactory::CreateCommandPool() const {
  VkCommandPoolCreateInfo create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  create_info.queueFamilyIndex = device_.GetQueueFamilyIndices().graphic;

  create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

  return ExecuteCreate<VkCommandPool>(vkCreateCommandPool, vkDestroyCommandPool, &create_info);
}

DeviceDispatchable<VkSemaphore> DeviceDispatchableFactory::CreateSemaphore() const {
  VkSemaphoreCreateInfo create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  return ExecuteCreate<VkSemaphore>(vkCreateSemaphore, vkDestroySemaphore, &create_info);
}

DeviceDispatchable<VkFence> DeviceDispatchableFactory::CreateFence() const {
  VkFenceCreateInfo create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  return ExecuteCreate<VkFence>(vkCreateFence, vkDestroyFence, &create_info);
}

DeviceDispatchable<VkDeviceMemory> DeviceDispatchableFactory::CreateMemory(const VkMemoryPropertyFlags properties, const VkMemoryRequirements mem_requirements) const {
  VkMemoryAllocateInfo alloc_info = {};
  alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  alloc_info.allocationSize = mem_requirements.size;
  alloc_info.memoryTypeIndex = FindMemoryType(mem_requirements.memoryTypeBits, device_.GetPhysical(), properties);

  return ExecuteCreate<VkDeviceMemory>(vkAllocateMemory, vkFreeMemory, &alloc_info);
}

Buffer DeviceDispatchableFactory::CreateBuffer(const VkBufferUsageFlags usage, const VkMemoryPropertyFlags properties, const uint32_t data_size) const {
  VkBufferCreateInfo buffer_info = {};
  buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  buffer_info.size = data_size;
  buffer_info.usage = usage;
  buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  DeviceDispatchable<VkBuffer> buffer = ExecuteCreate<VkBuffer>(vkCreateBuffer, vkDestroyBuffer, &buffer_info);

  VkMemoryRequirements mem_requirements;
  vkGetBufferMemoryRequirements(device_.GetLogical(), buffer.GetHandle(), &mem_requirements);

  DeviceDispatchable<VkDeviceMemory> memory = CreateMemory(properties, mem_requirements);

  if (const VkResult result = vkBindBufferMemory(device_.GetLogical(), buffer.GetHandle(), memory.GetHandle(), 0); result != VK_SUCCESS) {
    throw Error("failed to bind memory").WithCode(result);
  }

  return {
    std::move(buffer),
    std::move(memory),
    data_size
  };
}

DeviceDispatchable<VkDescriptorSetLayout> DeviceDispatchableFactory::CreateUboDescriptorSetLayout() const {
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

  return ExecuteCreate<VkDescriptorSetLayout>(vkCreateDescriptorSetLayout, vkDestroyDescriptorSetLayout, &layout_info);
}

DeviceDispatchable<VkDescriptorSetLayout> DeviceDispatchableFactory::CreateSamplerDescriptorSetLayout() const {
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

  return ExecuteCreate<VkDescriptorSetLayout>(vkCreateDescriptorSetLayout, vkDestroyDescriptorSetLayout, &layout_info);
}

DeviceDispatchable<VkDescriptorPool> DeviceDispatchableFactory::CreateDescriptorPool(const size_t ubo_count, const size_t texture_count) const {
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

  return ExecuteCreate<VkDescriptorPool>(vkCreateDescriptorPool, vkDestroyDescriptorPool, &pool_info);
}

Image DeviceDispatchableFactory::CreateImage(const VkImageUsageFlags usage, const VkImageAspectFlags aspect_flags, const VkMemoryPropertyFlags properties, const VkExtent2D extent, const VkFormat format, const VkImageTiling tiling, const uint32_t mip_levels) const {
  VkImageCreateInfo image_info = {};
  image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  image_info.imageType = VK_IMAGE_TYPE_2D;
  image_info.extent = { extent.width, extent.height, 1 };
  image_info.mipLevels = mip_levels;
  image_info.arrayLayers = 1;
  image_info.format = format;
  image_info.tiling = tiling;
  image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  image_info.usage = usage;
  image_info.samples = VK_SAMPLE_COUNT_1_BIT;
  image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  DeviceDispatchable<VkImage> image = ExecuteCreate<VkImage>(vkCreateImage, vkDestroyImage, &image_info);

  VkMemoryRequirements mem_requirements;
  vkGetImageMemoryRequirements(device_.GetLogical(), image.GetHandle(), &mem_requirements);
  DeviceDispatchable<VkDeviceMemory> memory = CreateMemory(properties, mem_requirements);
  if (const VkResult result = vkBindImageMemory(device_.GetLogical(), image.GetHandle(), memory.GetHandle(), 0); result != VK_SUCCESS) {
    throw Error("failed to bind memory").WithCode(result);
  }
  return Image{
    std::move(image),
    CreateImageView(image.GetHandle(), aspect_flags, format, mip_levels),
    std::move(memory),
    extent,
    format,
    mip_levels
  };
}

DeviceDispatchable<VkImageView> DeviceDispatchableFactory::CreateImageView(VkImage image, const VkImageAspectFlags aspect_flags, const VkFormat format, const uint32_t mip_levels) const {
  VkImageViewCreateInfo view_info = {};
  view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  view_info.image = image;
  view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
  view_info.format = format;
  view_info.subresourceRange.aspectMask = aspect_flags;
  view_info.subresourceRange.baseMipLevel = 0;
  view_info.subresourceRange.levelCount = mip_levels;
  view_info.subresourceRange.baseArrayLayer = 0;
  view_info.subresourceRange.layerCount = 1;

  return ExecuteCreate<VkImageView>(vkCreateImageView, vkDestroyImageView, &view_info);
}

DeviceDispatchable<VkFramebuffer> DeviceDispatchableFactory::CreateFramebuffer(const std::vector<VkImageView>& views, VkRenderPass render_pass, const VkExtent2D extent) const {
  VkFramebufferCreateInfo create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
  create_info.renderPass = render_pass;
  create_info.attachmentCount = static_cast<uint32_t>(views.size());
  create_info.pAttachments = views.data();
  create_info.width = extent.width;
  create_info.height = extent.height;
  create_info.layers = 1;

  return ExecuteCreate<VkFramebuffer>(vkCreateFramebuffer, vkDestroyFramebuffer, &create_info);
}

DeviceDispatchable<VkSampler> DeviceDispatchableFactory::CreateSampler(VkSamplerMipmapMode mipmap_mode, uint32_t mip_levels) const {
  VkPhysicalDeviceProperties properties = {};
  vkGetPhysicalDeviceProperties(device_.GetPhysical(), &properties);

  VkSamplerCreateInfo sampler_info = {};
  sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  sampler_info.magFilter = VK_FILTER_LINEAR;
  sampler_info.minFilter = VK_FILTER_LINEAR;
  sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  sampler_info.anisotropyEnable = VK_TRUE;
  sampler_info.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
  sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
  sampler_info.unnormalizedCoordinates = VK_FALSE;
  sampler_info.compareEnable = VK_FALSE;
  sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;
  sampler_info.mipmapMode = mipmap_mode;
  sampler_info.minLod = 0.0f;
  sampler_info.maxLod = static_cast<float>(mip_levels);
  sampler_info.mipLodBias = 0.0f;

  return ExecuteCreate<VkSampler>(vkCreateSampler, vkDestroySampler, &sampler_info);
}

Swapchain DeviceDispatchableFactory::CreateSwapchain(const VkExtent2D size, VkSurfaceKHR surface) const {
  const SurfaceSupportDetails device_support_details = Device::GetSurfaceSupport(device_.GetPhysical(), surface);

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

  const QueueFamilyIndices queue_family_indices = device_.GetQueueFamilyIndices();
  const std::array queue_family_indices_arr = {queue_family_indices.graphic, queue_family_indices.present};

  if (queue_family_indices.graphic != queue_family_indices.present) {
    create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    create_info.queueFamilyIndexCount = queue_family_indices_arr.size();
    create_info.pQueueFamilyIndices = queue_family_indices_arr.data();
  } else {
    create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  }

  create_info.preTransform = device_support_details.capabilities.currentTransform;
  create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  create_info.presentMode = present_mode;
  create_info.clipped = VK_TRUE;
  return {
    ExecuteCreate<VkSwapchainKHR>(vkCreateSwapchainKHR, vkDestroySwapchainKHR, &create_info),
    device_.GetPhysical(),
    extent,
    format
  };
}

std::vector<VkCommandBuffer> DeviceDispatchableFactory::CreateCommandBuffers(VkCommandPool cmd_pool, uint32_t count) const {
  VkCommandBufferAllocateInfo alloc_info = {};
  alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  alloc_info.commandPool = cmd_pool;
  alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  alloc_info.commandBufferCount = count;

  std::vector<VkCommandBuffer> cmd_buffers(count);
  if (const VkResult result = vkAllocateCommandBuffers(device_.GetLogical(), &alloc_info, cmd_buffers.data()); result != VK_SUCCESS) {
    throw Error("failed to allocate command buffers").WithCode(result);
  }
  return cmd_buffers;
}


} // namespace vk