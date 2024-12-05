#include "backend/render/vk/device.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <optional>
#include <limits>
#include <set>

#include "base/io.h"
#include "backend/render/vk/error.h"
#include "backend/render/vk/config.h"

namespace vk {

namespace {

struct PhysicalDeviceSurfaceDetails {
  VkSurfaceCapabilitiesKHR capabilities;
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR> present_modes;
};

PhysicalDeviceSurfaceDetails GetPhysicalDeviceSurfaceDetails(VkPhysicalDevice device, VkSurfaceKHR surface) {
  PhysicalDeviceSurfaceDetails details;
  if (const VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities); result != VK_SUCCESS) {
    throw Error("failed to get physical device surface capabilities").WithCode(result);
  }
  uint32_t format_count;
  if (const VkResult result = vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, nullptr); result != VK_SUCCESS) {
    throw Error("failed to get physical device surface formats count").WithCode(result);
  }

  if (format_count != 0) {
    details.formats.resize(format_count);
    if (const VkResult result = vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, details.formats.data()); result != VK_SUCCESS) {
      throw Error("failed to get physical device surface formats").WithCode(result);
    }
  }
  uint32_t present_mode_count;
  if (const VkResult result = vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_count, nullptr); result != VK_SUCCESS) {
    throw Error("failed to get physical device surface present modes count").WithCode(result);
  }

  if (present_mode_count != 0) {
    details.present_modes.resize(present_mode_count);
    if (const VkResult result = vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_count, details.present_modes.data()); result != VK_SUCCESS) {
      throw Error("failed to get physical device surface present modes").WithCode(result);
    }
  }
  return details;
}

bool PhysicalDeviceExtensionSupport(VkPhysicalDevice device) {
  uint32_t extension_count;
  if (const VkResult result = vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, nullptr); result != VK_SUCCESS) {
    throw Error("failed to get device extension properties count").WithCode(result);
  }
  std::vector<VkExtensionProperties> available_extensions(extension_count);
  if (const VkResult result = vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, available_extensions.data()); result != VK_SUCCESS) {
    throw Error("failed to get device extension properties").WithCode(result);
  }
  const std::vector<const char*> extensions = config::GetDeviceExtensions();
  std::set<std::string> required_extensions(extensions.begin(), extensions.end());

  for (const VkExtensionProperties& extension : available_extensions) {
    required_extensions.erase(extension.extensionName);
  }

  return required_extensions.empty();
}

inline VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& available_formats) {
  for (const VkSurfaceFormatKHR& available_format : available_formats) {
    if (available_format.format == VK_FORMAT_B8G8R8A8_SRGB &&
        available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      return available_format;
        }
  }
  return available_formats[0];
}

inline VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& available_present_modes) {
  for (const auto& available_present_mode : available_present_modes) {
    if (available_present_mode == VK_PRESENT_MODE_MAILBOX_KHR) {
      return available_present_mode;
    }
  }
  return VK_PRESENT_MODE_FIFO_KHR;
}

inline VkExtent2D ChooseSwapExtent(GLFWwindow* window, const VkSurfaceCapabilitiesKHR& capabilities) {
  if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
    return capabilities.currentExtent;
  }
  int width, height;
  glfwGetFramebufferSize(window, &width, &height);

  return {
    std::clamp(static_cast<uint32_t>(width), capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
    std::clamp(static_cast<uint32_t>(height), capabilities.minImageExtent.height, capabilities.maxImageExtent.height)
  };
}

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

VkFormat FindSupportedFormat(const std::vector<VkFormat>& formats, VkPhysicalDevice physical_device, VkImageTiling tiling, VkFormatFeatureFlags features) {
  for (const VkFormat format : formats) {
    VkFormatProperties props;
    vkGetPhysicalDeviceFormatProperties(physical_device, format, &props);
    if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
      return format;
    }
    if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
      return format;
    }
  }
  throw Error("failed to find supported format");
}

inline VkFormat FindDepthFormat(VkPhysicalDevice physical_device) {
  return FindSupportedFormat(
      {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
      physical_device,
      VK_IMAGE_TILING_OPTIMAL,
      VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
  );
}

inline uint32_t CalculateMipMaps(const VkExtent2D extent) {
  return static_cast<uint32_t>(std::floor(std::log2(std::max(extent.width, extent.height)))) + 1;
}

Device::Dispatchable<VkDeviceMemory> CreateMemory(VkDevice logical_device, VkPhysicalDevice physical_device, const VkAllocationCallbacks* allocator, const VkMemoryPropertyFlags properties, const VkMemoryRequirements& mem_requirements) {
  VkMemoryAllocateInfo alloc_info = {};
  alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  alloc_info.allocationSize = mem_requirements.size;
  alloc_info.memoryTypeIndex = FindMemoryType(mem_requirements.memoryTypeBits, physical_device, properties);

  VkDeviceMemory buffer_memory = VK_NULL_HANDLE;
  if (const VkResult result = vkAllocateMemory(logical_device, &alloc_info, allocator, &buffer_memory); result != VK_SUCCESS) {
    throw Error("failed to allocate vertex buffer memory").WithCode(result);
  }
  return {
    buffer_memory,
    logical_device,
    vkFreeMemory,
    allocator
  };
}

Device::Dispatchable<VkImage> CreateImageInternal(VkDevice logical_device, VkPhysicalDevice physical_device, const VkAllocationCallbacks* allocator, const VkImageUsageFlags usage, const VkExtent2D extent, const VkFormat format, const VkImageTiling tiling) {
  const uint32_t mip_levels = CalculateMipMaps(extent);

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

  VkImage image = VK_NULL_HANDLE;
  if (const VkResult result = vkCreateImage(logical_device, &image_info, allocator, &image); result != VK_SUCCESS) {
    throw Error("failed to create image!").WithCode(result);
  }
  return {
    image,
    logical_device,
    physical_device,
    allocator,
    extent,
    format,
    mip_levels
  };
}

Device::Dispatchable<VkImageView> CreateImageViewInternal(VkImage image, VkDevice logical_device, const VkAllocationCallbacks* allocator, const VkImageAspectFlags aspect_flags, const VkFormat format, const uint32_t mip_levels = 1) {
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

  VkImageView image_view = VK_NULL_HANDLE;
  if (const VkResult result = vkCreateImageView(logical_device, &view_info, allocator, &image_view); result != VK_SUCCESS) {
    throw Error("failed to create texture image view").WithCode(result);
  }
  return {
    image_view,
    logical_device,
    vkDestroyImageView,
    allocator
  };
}

} // namespace

std::pair<bool, Device::QueueFamilyIndices> Device::Finder::PhysicalDeviceIsSuitable(VkPhysicalDevice device, VkSurfaceKHR surface) {
  std::optional<uint32_t> graphic, present;
  uint32_t families_count = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &families_count, nullptr);

  std::vector<VkQueueFamilyProperties> families(families_count);
  vkGetPhysicalDeviceQueueFamilyProperties(device, &families_count, families.data());

  for (size_t i = 0; i < families_count; ++i) {
    if (families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      graphic = static_cast<uint32_t>(i);
    }
    VkBool32 present_support = false;
    if (const VkResult result = vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &present_support); result != VK_SUCCESS) {
      throw Error("failed to get physical device surface support").WithCode(result);
    }
    if (present_support) {
      present = static_cast<uint32_t>(i);
    }
    VkPhysicalDeviceFeatures supported_features;
    vkGetPhysicalDeviceFeatures(device, &supported_features);
    if (graphic.has_value() &&
        present.has_value() &&
        supported_features.samplerAnisotropy &&
        PhysicalDeviceExtensionSupport(device)) {
      const PhysicalDeviceSurfaceDetails details = GetPhysicalDeviceSurfaceDetails(device, surface);
      if (!details.formats.empty() && !details.present_modes.empty()) {
        return {true, {graphic.value(), present.value()}};
      }
        }
  }
  return {};
}

bool Device::Finder::FindSuitableDeviceForSurface(VkSurfaceKHR surface) {
  for(VkPhysicalDevice device : devices_) {
    if (auto[suitable, indices] = PhysicalDeviceIsSuitable(device, surface); suitable) {
      result_.device = device;
      result_.indices = indices;
      return true;
    }
  }
  return false;
}

Device::Device() noexcept
  : logical_device_(VK_NULL_HANDLE), physical_device_(VK_NULL_HANDLE), allocator_(nullptr), indices_() {}

Device::Device(VkPhysicalDevice physical_device, const QueueFamilyIndices& indices, const VkAllocationCallbacks* allocator)
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
  const std::vector<const char*> layers = config::GetInstanceLayers();
#endif // DEBUG
  const std::vector<const char*> extensions = config::GetDeviceExtensions();

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

Device::Device(Device&& other) noexcept
  : logical_device_(other.logical_device_),
    physical_device_(other.physical_device_),
    allocator_(other.allocator_),
    indices_(other.indices_) {
  other.logical_device_ = VK_NULL_HANDLE;
  other.physical_device_ = VK_NULL_HANDLE;
  other.allocator_ = nullptr;
  other.indices_ = {};
}

Device::~Device() {
  if (logical_device_ != VK_NULL_HANDLE) {
    vkDestroyDevice(logical_device_, allocator_);
  }
  logical_device_ = VK_NULL_HANDLE;
}

Device& Device::operator=(Device&& other) noexcept {
  if (this != &other) {
    logical_device_ = std::exchange(other.logical_device_, VK_NULL_HANDLE);
    physical_device_ = other.physical_device_;
    allocator_ = other.allocator_;
    indices_ = other.indices_;
  }
  return *this;
}

Device::Dispatchable<VkShaderModule> Device::CreateShaderModule(const Shader& shader) const {
  VkShaderModuleCreateInfo create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  create_info.codeSize = shader.spirv.size() * sizeof(uint32_t);
  create_info.pCode = shader.spirv.data();

  VkShaderModule module = VK_NULL_HANDLE;
  if (const VkResult result = vkCreateShaderModule(logical_device_, &create_info, allocator_, &module); result != VK_SUCCESS) {
    throw Error("failed to create shader module!").WithCode(result);
  }
  return {
    module,
    logical_device_,
    allocator_,
    shader.stage,
    shader.entry_point,
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

  std::array attachments = {color_attachment, depth_attachment};

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

Device::Dispatchable<VkPipeline> Device::CreatePipeline(VkPipelineLayout pipeline_layout, VkRenderPass render_pass, const std::vector<VkVertexInputAttributeDescription>& attribute_descriptions, const std::vector<VkVertexInputBindingDescription>& binding_descriptions, const std::vector<Dispatchable<VkShaderModule>>& shaders) const {
  std::vector<VkPipelineShaderStageCreateInfo> shader_stages_infos;
  shader_stages_infos.reserve(shaders.size());
  for(const Dispatchable<VkShaderModule>& shader : shaders) {
    VkPipelineShaderStageCreateInfo shader_stage_info = {};
    shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shader_stage_info.stage = shader.Stage();
    shader_stage_info.module = shader.Handle();
    shader_stage_info.pName = shader.EntryPoint().data();
    shader_stages_infos.push_back(shader_stage_info);
  }
  const std::vector<VkDynamicState> dynamic_states = config::GetDynamicStates();

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
  rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
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

Device::Dispatchable<VkBuffer> Device::CreateBuffer(const VkBufferUsageFlags usage, const uint32_t data_size) const {
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

Device::Dispatchable<VkImage> Device::CreateImage(const VkImageUsageFlags usage, const VkExtent2D extent, const VkFormat format, const VkImageTiling tiling) const {
  return CreateImageInternal(logical_device_, physical_device_, allocator_, usage, extent, format, tiling);
}

Device::Dispatchable<VkSwapchainKHR> Device::CreateSwapchain(GLFWwindow* window, VkSurfaceKHR surface) const {
  const PhysicalDeviceSurfaceDetails device_support_details = GetPhysicalDeviceSurfaceDetails(physical_device_, surface);

  VkSurfaceFormatKHR surface_format = ChooseSwapSurfaceFormat(device_support_details.formats);
  VkPresentModeKHR present_mode = ChooseSwapPresentMode(device_support_details.present_modes);

  VkExtent2D extent = ChooseSwapExtent(window, device_support_details.capabilities);
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

Device::Dispatchable<VkBuffer>::Dispatchable() noexcept : physical_device_(VK_NULL_HANDLE), size_(0) {}

Device::Dispatchable<VkBuffer>::Dispatchable(Dispatchable&& other) noexcept
  : Base(std::move(other)),
    memory_(std::move(other.memory_)),
    physical_device_(other.physical_device_),
    size_(other.size_) {
  other.physical_device_ = VK_NULL_HANDLE;
  other.size_ = 0;
}

Device::Dispatchable<VkBuffer>::Dispatchable(VkBuffer buffer,
                                             VkDevice logical_device,
                                             VkPhysicalDevice physical_device,
                                             const VkAllocationCallbacks* allocator,
                                             const uint32_t size) noexcept
   : Base(buffer, logical_device, vkDestroyBuffer, allocator),
     physical_device_(physical_device),
     size_(size) {}


Device::Dispatchable<VkBuffer>& Device::Dispatchable<VkBuffer>::operator=(Dispatchable&& other) noexcept {
  if (this != &other) {
    static_cast<Base&>(*this) = static_cast<Base>(std::move(other));
    memory_ = std::move(other.memory_);
    physical_device_ = std::exchange(other.physical_device_, VK_NULL_HANDLE);
    size_ = std::exchange(other.size_, 0);
  }
  return *this;
}

void Device::Dispatchable<VkBuffer>::Bind() const {
  if (const VkResult result = vkBindBufferMemory(parent_, handle_, memory_.Handle(), 0); result != VK_SUCCESS) {
    throw Error("failed to bind memory").WithCode(result);
  }
}

void Device::Dispatchable<VkBuffer>::Allocate(const VkMemoryPropertyFlags properties) {
  VkMemoryRequirements mem_requirements;
  vkGetBufferMemoryRequirements(parent_, handle_, &mem_requirements);

  memory_ = CreateMemory(parent_, physical_device_, allocator_, properties, mem_requirements);
}

void* Device::Dispatchable<VkBuffer>::Map() const {
  void* data;
  if (const VkResult result = vkMapMemory(memory_.Parent(), memory_.Handle(), 0, size_, 0, &data); result != VK_SUCCESS) {
    throw Error("failed to map buffer memory").WithCode(result);
  }
  return data;
}

void Device::Dispatchable<VkBuffer>::Unmap() const noexcept {
  vkUnmapMemory(memory_.Parent(), memory_.Handle());
}

uint32_t Device::Dispatchable<VkBuffer>::Size() const noexcept {
  return size_;
}

Device::Dispatchable<VkImage>::Dispatchable() noexcept
  : mip_levels_(0), physical_device_(VK_NULL_HANDLE), extent_(), format_() {}

Device::Dispatchable<VkImage>::Dispatchable(Dispatchable&& other) noexcept
  : Base(std::move(other)),
    mip_levels_(other.mip_levels_),
    physical_device_(other.physical_device_),
    extent_(other.extent_),
    format_(other.format_),
    view_(std::move(other.view_)),
    sampler_(std::move(other.sampler_)),
    memory_(std::move(other.memory_)) {
  other.mip_levels_ = 0;
  other.physical_device_ = VK_NULL_HANDLE;
  other.extent_ = {};
  other.format_ = {};
}

Device::Dispatchable<VkImage>& Device::Dispatchable<VkImage>::operator=(Dispatchable&& other) noexcept {
  if (this != &other) {
    static_cast<Base&>(*this) = static_cast<Base>(std::move(other));
    mip_levels_ = std::exchange(other.mip_levels_, 0);
    physical_device_ = std::exchange(other.physical_device_, VK_NULL_HANDLE);
    extent_ = std::exchange(other.extent_, {});
    format_ = std::exchange(other.format_, {});
    view_ = std::move(other.view_);
    sampler_ = std::move(other.sampler_);
    memory_ = std::move(other.memory_);
  }
  return *this;
}

Device::Dispatchable<VkImage>::Dispatchable(VkImage image,
                                            VkDevice logical_device,
                                            VkPhysicalDevice physical_device,
                                            const VkAllocationCallbacks* allocator,
                                            const VkExtent2D extent,
                                            const VkFormat format,
                                            const uint32_t mip_levels) noexcept
  : Base(image, logical_device, vkDestroyImage, allocator),
    mip_levels_(mip_levels),
    physical_device_(physical_device),
    extent_(extent),
    format_(format) {}

void Device::Dispatchable<VkImage>::Allocate(const VkMemoryPropertyFlags properties) {
  VkMemoryRequirements mem_requirements;
  vkGetImageMemoryRequirements(parent_, handle_, &mem_requirements);

  memory_ = CreateMemory(parent_, physical_device_, allocator_, properties, mem_requirements);
}

void Device::Dispatchable<VkImage>::Bind() const {
  if (const VkResult result = vkBindImageMemory(parent_, handle_, memory_.Handle(), 0); result != VK_SUCCESS) {
    throw Error("failed to bind memory").WithCode(result);
  }
}

void Device::Dispatchable<VkImage>::CreateView(const VkImageAspectFlags aspect_flags) {
  view_ = CreateImageViewInternal(handle_, parent_, allocator_, aspect_flags, format_, mip_levels_);
}

void Device::Dispatchable<VkImage>::CreateSampler(const VkSamplerMipmapMode mipmap_mode) {
  VkPhysicalDeviceProperties properties = {};
  vkGetPhysicalDeviceProperties(physical_device_, &properties);

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
  sampler_info.maxLod = static_cast<float>(mip_levels_);
  sampler_info.mipLodBias = 0.0f;

  VkSampler sampler = VK_NULL_HANDLE;
  if (const VkResult result = vkCreateSampler(parent_, &sampler_info, allocator_, &sampler); result != VK_SUCCESS) {
    throw Error("failed to create texture sampler").WithCode(result);
  }
  sampler_ = Dispatchable<VkSampler>(sampler, parent_, vkDestroySampler, allocator_);
}

bool Device::Dispatchable<VkImage_T*>::FormatFeatureSupported(const VkFormatFeatureFlagBits feature) const {
  VkFormatProperties format_properties = {};
  vkGetPhysicalDeviceFormatProperties(physical_device_, format_, &format_properties);
  return (format_properties.optimalTilingFeatures & feature) != 0;
}

Device::Dispatchable<VkShaderModule>::Dispatchable() noexcept : stage_(VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM) {}

Device::Dispatchable<VkShaderModule>::Dispatchable(Dispatchable&& other) noexcept
  : Base(std::move(other)), stage_(other.stage_), entry_point_(other.entry_point_) {
  other.stage_ = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
  other.entry_point_ = {};
}

Device::Dispatchable<VkShaderModule>::Dispatchable(VkShaderModule module,
                                                   VkDevice logical_device,
                                                   const VkAllocationCallbacks* allocator,
                                                   VkShaderStageFlagBits type,
                                                   std::string_view entry_point) noexcept
  : Base(module, logical_device, vkDestroyShaderModule, allocator), stage_(type), entry_point_(entry_point) {}


Device::Dispatchable<VkShaderModule>& Device::Dispatchable<VkShaderModule>::operator=(Dispatchable&& other) noexcept {
  if (this != &other) {
    static_cast<Base&>(*this) = static_cast<Base>(std::move(other));
    stage_ = std::exchange(other.stage_, VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM);
    entry_point_ = std::exchange(other.entry_point_, {});
  }
  return *this;
}

Device::Dispatchable<VkSwapchainKHR>::Dispatchable() noexcept : extent_(), format_(VK_FORMAT_UNDEFINED), physical_device_(VK_NULL_HANDLE)  {}

Device::Dispatchable<VkSwapchainKHR>::Dispatchable(Dispatchable&& other) noexcept
  : Base(std::move(other)),
    extent_(other.extent_),
    format_(other.format_),
    physical_device_(other.physical_device_),
    depth_image_(std::move(other.depth_image_)),
    image_views_(std::move(other.image_views_)) {
  other.physical_device_ = VK_NULL_HANDLE;
  other.extent_ = {};
  other.format_ = {};
}

Device::Dispatchable<VkSwapchainKHR>& Device::Dispatchable<VkSwapchainKHR>::operator=(Dispatchable&& other) noexcept {
  if (this != &other) {
    static_cast<Base&>(*this) = static_cast<Base>(std::move(other));
    physical_device_ = std::exchange(other.physical_device_, VK_NULL_HANDLE);
    extent_ = std::exchange(other.extent_, {});
    format_ = std::exchange(other.format_, {});
    depth_image_ = std::move(other.depth_image_);
    image_views_ = std::move(other.image_views_);
  }
  return *this;
}

Device::Dispatchable<VkSwapchainKHR>::Dispatchable(VkSwapchainKHR swapchain,
                                                   VkDevice logical_device,
                                                   VkPhysicalDevice physical_device,
                                                   const VkAllocationCallbacks* allocator,
                                                   const VkExtent2D extent,
                                                   const VkFormat format) noexcept :
      Base(swapchain, logical_device, vkDestroySwapchainKHR, allocator),
      extent_(extent),
      format_(format),
      physical_device_(physical_device),
      depth_image_(CreateDepthImage()),
      image_views_(CreateImageViews()) {}

std::vector<VkImage> Device::Dispatchable<VkSwapchainKHR>::GetImages() const {
  uint32_t image_count;
  if (const VkResult result = vkGetSwapchainImagesKHR(parent_, handle_, &image_count, nullptr); result != VK_SUCCESS) {
    throw Error("failed to get swapchain image count").WithCode(result);
  }
  std::vector<VkImage> images(image_count);
  if (const VkResult result = vkGetSwapchainImagesKHR(parent_, handle_, &image_count, images.data()); result != VK_SUCCESS) {
    throw Error("failed to get swapchain images").WithCode(result);
  }
  return images;
}

std::vector<Device::Dispatchable<VkImageView>> Device::Dispatchable<VkSwapchainKHR>::CreateImageViews() const {
  std::vector<VkImage> images = GetImages();
  std::vector<Dispatchable<VkImageView>> image_views;
  image_views.reserve(images.size());
  for(VkImage image : images) {
    Dispatchable<VkImageView> image_view = CreateImageViewInternal(image, parent_, allocator_,
                                                                   VK_IMAGE_ASPECT_COLOR_BIT, format_);
    image_views.emplace_back(std::move(image_view));
  }
  return image_views;
}

Device::Dispatchable<VkImage> Device::Dispatchable<VkSwapchainKHR>::CreateDepthImage() const {
  const VkFormat depth_format = FindDepthFormat(physical_device_);

  Dispatchable<VkImage> depth_image = CreateImageInternal(parent_, physical_device_, allocator_, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, extent_, depth_format, VK_IMAGE_TILING_OPTIMAL);
  depth_image.Allocate(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
  depth_image.Bind();
  depth_image.CreateView(VK_IMAGE_ASPECT_DEPTH_BIT);
  return depth_image;
}

Device::Dispatchable<VkFramebuffer> Device::Dispatchable<VkSwapchainKHR>::CreateFramebuffer(const std::vector<VkImageView>& views, VkRenderPass render_pass) const {
  VkFramebufferCreateInfo create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
  create_info.renderPass = render_pass;
  create_info.attachmentCount = static_cast<uint32_t>(views.size());
  create_info.pAttachments = views.data();
  create_info.width = extent_.width;
  create_info.height = extent_.height;
  create_info.layers = 1;

  VkFramebuffer framebuffer = VK_NULL_HANDLE;
  if (const VkResult result = vkCreateFramebuffer(parent_, &create_info, allocator_, &framebuffer); result != VK_SUCCESS) {
    throw Error("failed to create framebuffer").WithCode(result);
  }
  return {
    framebuffer,
    parent_,
    vkDestroyFramebuffer,
    allocator_,
  };
}

std::vector<Device::Dispatchable<VkFramebuffer>> Device::Dispatchable<VkSwapchainKHR>::CreateFramebuffers(VkRenderPass render_pass) const {
  std::vector<Dispatchable<VkFramebuffer>> framebuffers;
  framebuffers.reserve(image_views_.size());
  for(const Dispatchable<VkImageView>& image_view : image_views_) {
    Dispatchable<VkFramebuffer> framebuffer = CreateFramebuffer({image_view.Handle(), depth_image_.View().Handle()}, render_pass);
    framebuffers.emplace_back(std::move(framebuffer));
  }
  return framebuffers;
}

} // namespace vk