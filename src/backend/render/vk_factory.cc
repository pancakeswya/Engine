#include "backend/render/vk_factory.h"
#include "backend/render/vk_config.h"
#include "base/io.h"

#include <algorithm>
#include <array>
#include <set>
#include <optional>
#include <limits>

namespace vk::factory {

namespace {

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

std::pair<bool, QueueFamilyIndices> FindPhysicalDeviceFamilyIndices(VkPhysicalDevice device, VkSurfaceKHR surface) {
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

HandleWrapper<VkFramebuffer> CreateFramebuffer(VkDevice logical_device, VkRenderPass render_pass, const std::vector<VkImageView>& views, VkExtent2D extent) {
  const VkAllocationCallbacks* alloc_cb = config::GetAllocationCallbacks();
  VkFramebufferCreateInfo create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
  create_info.renderPass = render_pass;
  create_info.attachmentCount = static_cast<uint32_t>(views.size());
  create_info.pAttachments = views.data();
  create_info.width = extent.width;
  create_info.height = extent.height;
  create_info.layers = 1;

  VkFramebuffer framebuffer = VK_NULL_HANDLE;
  if (const VkResult result = vkCreateFramebuffer(logical_device, &create_info, alloc_cb, &framebuffer); result != VK_SUCCESS) {
    throw Error("failed to create framebuffer").WithCode(result);
  }
  return {
    framebuffer,
    [logical_device, alloc_cb](VkFramebuffer framebuffer) {
      vkDestroyFramebuffer(logical_device, framebuffer, alloc_cb);
    }
  };
}

uint32_t FindMemoryType(const uint32_t type_filter, VkPhysicalDevice physical_device, VkMemoryPropertyFlags properties) {
  VkPhysicalDeviceMemoryProperties mem_properties;
  vkGetPhysicalDeviceMemoryProperties(physical_device, &mem_properties);

  for (uint32_t i = 0; i < mem_properties.memoryTypeCount; i++) {
    if ((type_filter & (1 << i)) && (mem_properties.memoryTypes[i].propertyFlags & properties) == properties) {
      return i;
    }
  }
  throw Error("failed to find suitable memory type!");
}

} // namespace

HandleWrapper<VkInstance> CreateInstance() {
#ifdef DEBUG
  if (!config::InstanceLayersIsSupported()) {
    throw Error("Instance layers are not supported");
  }
  const std::vector<const char*> layers = config::GetInstanceLayers();
#endif // DEBUG
  const VkApplicationInfo app_info = config::GetApplicationInfo();
  const VkDebugUtilsMessengerCreateInfoEXT messenger_info = config::GetMessengerCreateInfo();
  const VkAllocationCallbacks* alloc_cb = config::GetAllocationCallbacks();

  const std::vector<const char*> extensions = config::GetInstanceExtensions();

  VkInstanceCreateInfo create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  create_info.pApplicationInfo = &app_info;
  create_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
  create_info.ppEnabledExtensionNames = extensions.data();
#ifdef __APPLE__
  create_info.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif
#ifdef DEBUG
  create_info.enabledLayerCount = static_cast<uint32_t>(layers.size());
  create_info.ppEnabledLayerNames = layers.data();
  create_info.pNext = &messenger_info;
#endif // DEBUG

  VkInstance instance = VK_NULL_HANDLE;
  if (const VkResult result = vkCreateInstance(&create_info, alloc_cb, &instance); result != VK_SUCCESS) {
    throw Error("failed to create instance").WithCode(result);
  }
  return {
    instance,
    [alloc_cb](VkInstance instance) {
      vkDestroyInstance(instance, alloc_cb);
    }
  };
}

#ifdef DEBUG
HandleWrapper<VkDebugUtilsMessengerEXT> CreateMessenger(VkInstance instance) {
  const auto create_messenger = vkGetInstanceProcAddrByType(instance, vkCreateDebugUtilsMessengerEXT);
  if (create_messenger == nullptr) {
    throw Error("Couldn't find vkCreateDebugUtilsMessengerEXT by procc addr");
  }
  const VkDebugUtilsMessengerCreateInfoEXT messenger_info = config::GetMessengerCreateInfo();
  const VkAllocationCallbacks* alloc_cb = config::GetAllocationCallbacks();

  VkDebugUtilsMessengerEXT messenger = VK_NULL_HANDLE;
  if (const VkResult result = create_messenger(instance, &messenger_info, alloc_cb, &messenger); result != VK_SUCCESS) {
    throw Error("failed to set up debug messenger").WithCode(result);
  }
  return {
    messenger,
    [instance, alloc_cb](VkDebugUtilsMessengerEXT messenger) {
      const auto destroy_messenger = vkGetInstanceProcAddrByType(instance, vkDestroyDebugUtilsMessengerEXT);
      if (destroy_messenger != nullptr) {
        destroy_messenger(instance, messenger, alloc_cb);
      }
    }
  };
}
#endif // DEBUG

HandleWrapper<VkSurfaceKHR> CreateSurface(VkInstance instance, GLFWwindow* window) {
  VkSurfaceKHR surface = VK_NULL_HANDLE;
  const VkAllocationCallbacks* alloc_cb = config::GetAllocationCallbacks();

  if (const VkResult result = glfwCreateWindowSurface(instance, window, alloc_cb, &surface); result != VK_SUCCESS) {
    throw Error("failed to create window surface!").WithCode(result);
  }
  return {
    surface,
    [instance, alloc_cb](VkSurfaceKHR surface) {
      vkDestroySurfaceKHR(instance, surface, alloc_cb);
    }
  };
}


std::pair<VkPhysicalDevice, QueueFamilyIndices> CreatePhysicalDevice(VkInstance instance, VkSurfaceKHR surface) {
  uint32_t device_count = 0;
  if (const VkResult result = vkEnumeratePhysicalDevices(instance, &device_count, nullptr); result != VK_SUCCESS) {
    throw Error("failed to get physical devices count").WithCode(result);
  }
  if (device_count == 0) {
    throw Error("failed to find GPUs with Vulkan support");
  }
  std::vector<VkPhysicalDevice> devices(device_count);
  if (const VkResult result = vkEnumeratePhysicalDevices(instance, &device_count, devices.data()); result != VK_SUCCESS) {
    throw Error("failed to get physical devices").WithCode(result);
  }
  for (const VkPhysicalDevice& device : devices) {
    if (auto [found, indices] = FindPhysicalDeviceFamilyIndices(device, surface); found) {
      return {device, indices};
    }
  }
  throw Error("failed to find a suitable GPU");
}

HandleWrapper<VkDevice> CreateLogicalDevice(VkPhysicalDevice physical_device, const QueueFamilyIndices& indices) {
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
  const VkAllocationCallbacks* alloc_cb = config::GetAllocationCallbacks();

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

  VkDevice logical_device = VK_NULL_HANDLE;
  if (const VkResult result = vkCreateDevice(physical_device, &create_info, alloc_cb, &logical_device); result != VK_SUCCESS) {
    throw Error("failed to create logical device").WithCode(result);
  }
  return {
    logical_device,
    [alloc_cb](VkDevice logical_device) {
      vkDestroyDevice(logical_device, alloc_cb);
    }
  };
}

HandleWrapper<VkShaderModule> CreateShaderModule(VkDevice logical_device, const std::string& path) {
  const std::vector<char> shader_bytes = io::ReadFile(path);
  const VkAllocationCallbacks* alloc_cb = config::GetAllocationCallbacks();

  VkShaderModuleCreateInfo create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  create_info.codeSize = shader_bytes.size();
  create_info.pCode = reinterpret_cast<const uint32_t*>(shader_bytes.data());

  VkShaderModule module = VK_NULL_HANDLE;
  if (const VkResult result = vkCreateShaderModule(logical_device, &create_info, alloc_cb, &module); result != VK_SUCCESS) {
    throw Error("failed to create shader module!").WithCode(result);
  }
  return {
    module,
    [logical_device, alloc_cb](VkShaderModule module) {
      vkDestroyShaderModule(logical_device, module, alloc_cb);
    }
  };
}

HandleWrapper<VkRenderPass> CreateRenderPass(VkDevice logical_device, VkFormat image_format, VkFormat depth_format) {
  const VkAllocationCallbacks* alloc_cb = config::GetAllocationCallbacks();
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
  color_attachment_ref .attachment = 0;
  color_attachment_ref .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

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
  if (const VkResult result = vkCreateRenderPass(logical_device, &render_pass_info, alloc_cb, &render_pass); result != VK_SUCCESS) {
    throw Error("failed to create render pass").WithCode(result);
  }
  return {
    render_pass,
    [logical_device, alloc_cb](VkRenderPass render_pass) {
      vkDestroyRenderPass(logical_device, render_pass, alloc_cb);
    }
  };
}

std::pair<HandleWrapper<VkSwapchainKHR>, SwapchainDetails> CreateSwapchain(GLFWwindow* window, VkSurfaceKHR surface, VkPhysicalDevice physical_device, QueueFamilyIndices indices, VkDevice logical_device) {
  const PhysicalDeviceSurfaceDetails device_support_details = GetPhysicalDeviceSurfaceDetails(physical_device, surface);
  const VkAllocationCallbacks* alloc_cb = config::GetAllocationCallbacks();

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

  const std::array queue_family_indices = {indices.graphic, indices.present};

  if (indices.graphic != indices.present) {
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
  if (const VkResult result = vkCreateSwapchainKHR(logical_device, &create_info, alloc_cb, &swapchain); result != VK_SUCCESS) {
    throw Error("failed to create swap chain!").WithCode(result);
  }
  return {
    HandleWrapper<VkSwapchainKHR>{
      swapchain,
      [logical_device, alloc_cb](VkSwapchainKHR swapchain) {
        vkDestroySwapchainKHR(logical_device, swapchain, alloc_cb);
      }
    },
    SwapchainDetails{
      extent,
      format
    }
  };
}

std::vector<VkImage> CreateSwapchainImages(VkSwapchainKHR swapchain, VkDevice logical_device) {
  uint32_t image_count;
  if (const VkResult result = vkGetSwapchainImagesKHR(logical_device, swapchain, &image_count, nullptr); result != VK_SUCCESS) {
    throw Error("failed to get swapchain image count").WithCode(result);
  }
  std::vector<VkImage> images(image_count);
  if (const VkResult result = vkGetSwapchainImagesKHR(logical_device, swapchain, &image_count, images.data()); result != VK_SUCCESS) {
    throw Error("failed to get swapchain images").WithCode(result);
  }
  return images;
}

std::vector<HandleWrapper<VkImageView>> CreateImageViews(const std::vector<VkImage>& images, VkDevice logical_device, VkFormat format) {
  std::vector<HandleWrapper<VkImageView>> image_views;
  image_views.reserve(images.size());
  for(VkImage image : images) {
    HandleWrapper<VkImageView> image_view = CreateImageView(logical_device, image, format, VK_IMAGE_ASPECT_COLOR_BIT);
    image_views.emplace_back(std::move(image_view));
  }
  return image_views;
}

std::vector<HandleWrapper<VkFramebuffer>> CreateFramebuffers(VkDevice logical_device, const std::vector<HandleWrapper<VkImageView>>& image_views, VkImageView depth_view, VkRenderPass render_pass,VkExtent2D extent) {
  std::vector<HandleWrapper<VkFramebuffer>> framebuffers;
  framebuffers.reserve(image_views.size());
  for(const HandleWrapper<VkImageView>& image_view : image_views) {
    HandleWrapper<VkFramebuffer> framebuffer = CreateFramebuffer(logical_device, render_pass, {image_view.get(), depth_view}, extent);
    framebuffers.emplace_back(std::move(framebuffer));
  }
  return framebuffers;
}

HandleWrapper<VkPipelineLayout> CreatePipelineLayout(VkDevice logical_device, VkDescriptorSetLayout descriptor_set_layout) {
  const VkAllocationCallbacks* alloc_cb = config::GetAllocationCallbacks();
  VkPushConstantRange pushConstantRange = {};
  pushConstantRange.offset = 0;
  pushConstantRange.size = sizeof(unsigned int);
  pushConstantRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

  VkPipelineLayoutCreateInfo pipeline_layout_info = {};
  pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipeline_layout_info.setLayoutCount = 1;
  pipeline_layout_info.pSetLayouts = &descriptor_set_layout;
  pipeline_layout_info.pPushConstantRanges = &pushConstantRange;
  pipeline_layout_info.pushConstantRangeCount = 1;

  VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;
  if (const VkResult result = vkCreatePipelineLayout(logical_device, &pipeline_layout_info, alloc_cb, &pipeline_layout); result != VK_SUCCESS) {
    throw Error("failed to create pipeline layout").WithCode(result);
  }
  return {
    pipeline_layout,
    [logical_device, alloc_cb](VkPipelineLayout pipeline_layout) {
      vkDestroyPipelineLayout(logical_device, pipeline_layout, alloc_cb);
    }
  };
}

HandleWrapper<VkPipeline> CreatePipeline(VkDevice logical_device, VkPipelineLayout pipeline_layout, VkRenderPass render_pass, const std::initializer_list<ShaderStage>& shader_stages) {
  std::vector<VkPipelineShaderStageCreateInfo> shader_stages_infos;
  shader_stages_infos.reserve(shader_stages.size());
  for(const ShaderStage& shader_stage : shader_stages) {
    VkPipelineShaderStageCreateInfo shader_stage_info = {};
    shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shader_stage_info.stage = shader_stage.bits;
    shader_stage_info.module = shader_stage.module.get();
    shader_stage_info.pName = shader_stage.name.data();
    shader_stages_infos.push_back(shader_stage_info);
  }
  const std::vector<VkDynamicState> dynamic_states = config::GetDynamicStates();
  const std::vector<VkVertexInputAttributeDescription> attribute_descriptions = Vertex::GetAttributeDescriptions();
  const std::vector<VkVertexInputBindingDescription> binding_descriptions = Vertex::GetBindingDescriptions();
  const VkAllocationCallbacks* alloc_cb = config::GetAllocationCallbacks();

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
  if (const VkResult result = vkCreateGraphicsPipelines(logical_device, VK_NULL_HANDLE, 1, &pipeline_info, alloc_cb, &pipeline); result != VK_SUCCESS) {
    throw Error("failed to create graphics pipeline").WithCode(result);
  }
  return {
    pipeline,
    [logical_device, alloc_cb](VkPipeline pipeline) {
      vkDestroyPipeline(logical_device, pipeline, alloc_cb);
    }
  };
}

HandleWrapper<VkCommandPool> CreateCommandPool(VkDevice logical_device, QueueFamilyIndices indices) {
  const VkAllocationCallbacks* alloc_cb = config::GetAllocationCallbacks();
  VkCommandPoolCreateInfo create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  create_info.queueFamilyIndex = indices.graphic;
  create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

  VkCommandPool cmd_pool = VK_NULL_HANDLE;
  if (const VkResult result = vkCreateCommandPool(logical_device, &create_info, alloc_cb, &cmd_pool); result != VK_SUCCESS) {
    throw Error("failed to create command pool");
  }
  return {
    cmd_pool,
    [logical_device, alloc_cb](VkCommandPool cmd_pool) {
      vkDestroyCommandPool(logical_device, cmd_pool, alloc_cb);
    }
  };
}

std::vector<VkCommandBuffer> CreateCommandBuffers(VkDevice logical_device, VkCommandPool cmd_pool, uint32_t count) {
  VkCommandBufferAllocateInfo alloc_info = {};
  alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  alloc_info.commandPool = cmd_pool;
  alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  alloc_info.commandBufferCount = count;

  std::vector<VkCommandBuffer> cmd_buffers(count);
  if (const VkResult result = vkAllocateCommandBuffers(logical_device, &alloc_info, cmd_buffers.data()); result != VK_SUCCESS) {
    throw Error("failed to allocate command buffers").WithCode(result);
  }
  return cmd_buffers;
}

HandleWrapper<VkSemaphore> CreateSemaphore(VkDevice logical_device) {
  const VkAllocationCallbacks* alloc_cb = config::GetAllocationCallbacks();
  VkSemaphoreCreateInfo create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  VkSemaphore semaphore = VK_NULL_HANDLE;
  if (const VkResult result = vkCreateSemaphore(logical_device, &create_info, alloc_cb, &semaphore); result != VK_SUCCESS) {
    throw Error("failed to create semaphore").WithCode(result);
  }
  return {
    semaphore,
    [logical_device, alloc_cb](VkSemaphore semaphore) {
      vkDestroySemaphore(logical_device, semaphore, alloc_cb);
    }
  };
}

HandleWrapper<VkFence> CreateFence(VkDevice logical_device) {
  const VkAllocationCallbacks* alloc_cb = config::GetAllocationCallbacks();
  VkFenceCreateInfo create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  VkFence fence = VK_NULL_HANDLE;
  if (const VkResult result = vkCreateFence(logical_device, &create_info, alloc_cb, &fence); result != VK_SUCCESS) {
    throw Error("failed to create semaphore").WithCode(result);
  }
  return {
    fence,
    [logical_device, alloc_cb](VkFence fence) {
      vkDestroyFence(logical_device, fence, alloc_cb);
    }
  };
}

HandleWrapper<VkBuffer> CreateBuffer(VkDevice logical_device, VkBufferUsageFlags usage, uint32_t data_size) {
  const VkAllocationCallbacks* alloc_cb = config::GetAllocationCallbacks();
  VkBufferCreateInfo buffer_info = {};
  buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  buffer_info.size = data_size;
  buffer_info.usage = usage;
  buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  VkBuffer buffer = VK_NULL_HANDLE;
  if (const VkResult result = vkCreateBuffer(logical_device, &buffer_info, alloc_cb, &buffer); result != VK_SUCCESS) {
    throw Error("failed to create vertex buffer").WithCode(result);
  }
  return {
    buffer,
    [logical_device, alloc_cb](VkBuffer buffer) {
      vkDestroyBuffer(logical_device, buffer, alloc_cb);
    }
  };
}

HandleWrapper<VkDeviceMemory> CreateMemory(VkDevice logical_device, VkPhysicalDevice physical_device, VkMemoryPropertyFlags properties, VkMemoryRequirements mem_requirements) {
  const VkAllocationCallbacks* alloc_cb = config::GetAllocationCallbacks();

  VkMemoryAllocateInfo alloc_info = {};
  alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  alloc_info.allocationSize = mem_requirements.size;
  alloc_info.memoryTypeIndex = FindMemoryType(mem_requirements.memoryTypeBits, physical_device, properties);

  VkDeviceMemory buffer_memory = VK_NULL_HANDLE;
  if (const VkResult result = vkAllocateMemory(logical_device, &alloc_info, alloc_cb, &buffer_memory); result != VK_SUCCESS) {
    throw Error("failed to allocate vertex buffer memory").WithCode(result);
  }
  return {
    buffer_memory,
    [logical_device, alloc_cb](VkDeviceMemory buffer_memory) {
      vkFreeMemory(logical_device, buffer_memory, alloc_cb);
    }
  };
}

HandleWrapper<VkDeviceMemory> CreateBufferMemory(VkDevice logical_device, VkPhysicalDevice physical_device, VkMemoryPropertyFlags properties, VkBuffer buffer) {
  VkMemoryRequirements mem_requirements;
  vkGetBufferMemoryRequirements(logical_device, buffer, &mem_requirements);

  return CreateMemory(logical_device, physical_device, properties, mem_requirements);
}

HandleWrapper<VkDescriptorSetLayout> CreateDescriptorSetLayout(VkDevice logical_device) {
  const VkAllocationCallbacks* alloc_cb = config::GetAllocationCallbacks();

  VkDescriptorSetLayoutBinding ubo_layout_binding = {};
  ubo_layout_binding.binding = 0;
  ubo_layout_binding.descriptorCount = 1;
  ubo_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  ubo_layout_binding.pImmutableSamplers = nullptr;
  ubo_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

  VkDescriptorSetLayoutBinding sampler_layout_binding = {};
  sampler_layout_binding.binding = 1;
  sampler_layout_binding.descriptorCount = config::kMaxTextureCount;
  sampler_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  sampler_layout_binding.pImmutableSamplers = nullptr;
  sampler_layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

  std::array bindings = {ubo_layout_binding, sampler_layout_binding};
  VkDescriptorSetLayoutCreateInfo layout_info = {};
  layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  layout_info.bindingCount = static_cast<uint32_t>(bindings.size());
  layout_info.pBindings = bindings.data();

  VkDescriptorSetLayout descriptor_set_layout = VK_NULL_HANDLE;
  if (const VkResult result = vkCreateDescriptorSetLayout(logical_device, &layout_info, alloc_cb, &descriptor_set_layout); result != VK_SUCCESS) {
    throw Error("failed to create descriptor set layout").WithCode(result);
  }
  return {
    descriptor_set_layout,
    [logical_device, alloc_cb](VkDescriptorSetLayout descriptor_set_layout) {
      vkDestroyDescriptorSetLayout(logical_device, descriptor_set_layout, alloc_cb);
    }
  };
}

HandleWrapper<VkDescriptorPool> CreateDescriptorPool(VkDevice logical_device, const size_t count) {
  const VkAllocationCallbacks* alloc_cb = config::GetAllocationCallbacks();

  std::array<VkDescriptorPoolSize, 2> pool_sizes = {};
  pool_sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  pool_sizes[0].descriptorCount = static_cast<uint32_t>(count);
  pool_sizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  pool_sizes[1].descriptorCount = static_cast<uint32_t>(count * config::kMaxTextureCount);

  VkDescriptorPoolCreateInfo pool_info = {};
  pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  pool_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
  pool_info.pPoolSizes = pool_sizes.data();
  pool_info.maxSets = static_cast<uint32_t>(count);

  VkDescriptorPool descriptor_pool = VK_NULL_HANDLE;
  if (const VkResult result = vkCreateDescriptorPool(logical_device, &pool_info, alloc_cb, &descriptor_pool); result != VK_SUCCESS) {
    throw Error("failed to create descriptor pool").WithCode(result);
  }
  return {
    descriptor_pool,
    [logical_device, alloc_cb](VkDescriptorPool descriptor_pool) {
      vkDestroyDescriptorPool(logical_device, descriptor_pool, alloc_cb);
    }
  };
}

std::vector<VkDescriptorSet> CreateDescriptorSets(VkDevice logical_device, VkDescriptorSetLayout descriptor_set_layout, VkDescriptorPool descriptor_pool, const size_t count) {
  std::vector layouts(count, descriptor_set_layout);
  VkDescriptorSetAllocateInfo alloc_info = {};
  alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  alloc_info.descriptorPool = descriptor_pool;
  alloc_info.descriptorSetCount = static_cast<uint32_t>(count);
  alloc_info.pSetLayouts = layouts.data();

  std::vector<VkDescriptorSet> descriptor_sets(count);
  if (const VkResult result = vkAllocateDescriptorSets(logical_device, &alloc_info, descriptor_sets.data()); result != VK_SUCCESS) {
    throw Error("failed to allocate descriptor sets").WithCode(result);
  }
  return descriptor_sets;
}

HandleWrapper<VkDeviceMemory> CreateImageMemory(VkDevice logical_device, VkPhysicalDevice physical_device, VkMemoryPropertyFlags properties, VkImage image) {
  VkMemoryRequirements mem_requirements;
  vkGetImageMemoryRequirements(logical_device, image, &mem_requirements);

  return CreateMemory(logical_device, physical_device, properties, mem_requirements);
}

HandleWrapper<VkImage> CreateImage(VkDevice logical_device, VkExtent2D extent, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, uint32_t mip_levels) {
  const VkAllocationCallbacks* alloc_cb = config::GetAllocationCallbacks();

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
  if (const VkResult result = vkCreateImage(logical_device, &image_info, alloc_cb, &image); result != VK_SUCCESS) {
    throw Error("failed to create image!").WithCode(result);
  }
  return {
    image,
    [logical_device, alloc_cb](VkImage image) {
      vkDestroyImage(logical_device, image, alloc_cb);
    }
  };
}

HandleWrapper<VkImageView> CreateImageView(VkDevice logical_device, VkImage image, VkFormat format, VkImageAspectFlags aspect_flags, uint32_t mip_levels) {
  const VkAllocationCallbacks* alloc_cb = config::GetAllocationCallbacks();

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
  if (const VkResult result = vkCreateImageView(logical_device, &view_info, alloc_cb, &image_view); result != VK_SUCCESS) {
    throw Error("failed to create texture image view").WithCode(result);
  }

  return {
    image_view,
    [logical_device, alloc_cb](VkImageView image_view) {
      vkDestroyImageView(logical_device, image_view, alloc_cb);
    }
  };
}

HandleWrapper<VkSampler> CreateTextureSampler(VkDevice logical_device, VkPhysicalDevice physical_device, VkSamplerMipmapMode mipmap_mode, const uint32_t mip_levels) {
  const VkAllocationCallbacks* alloc_cb = config::GetAllocationCallbacks();

  VkPhysicalDeviceProperties properties = {};
  vkGetPhysicalDeviceProperties(physical_device, &properties);

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

  VkSampler sampler = VK_NULL_HANDLE;
  if (const VkResult result = vkCreateSampler(logical_device, &sampler_info, alloc_cb, &sampler); result != VK_SUCCESS) {
    throw Error("failed to create texture sampler").WithCode(result);
  }
  return {
    sampler,
    [logical_device, alloc_cb](VkSampler sampler) {
      vkDestroySampler(logical_device, sampler, alloc_cb);
    }
  };
}

} // namespace vk::factory