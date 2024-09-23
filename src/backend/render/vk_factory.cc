#include "backend/render/vk_factory.h"
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
    if (graphic.has_value() &&
        present.has_value() &&
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

HandleWrapper<VkImageView> CreateImageView(VkDevice logical_device, VkImage image, VkFormat format) {
  const VkAllocationCallbacks* alloc_cb = config::GetAllocationCallbacks();
  VkImageViewCreateInfo create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  create_info.image = image;
  create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
  create_info.format = format;
  create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
  create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
  create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
  create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
  create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  create_info.subresourceRange.baseMipLevel = 0;
  create_info.subresourceRange.levelCount = 1;
  create_info.subresourceRange.baseArrayLayer = 0;
  create_info.subresourceRange.layerCount = 1;

  VkImageView image_view = VK_NULL_HANDLE;
  if (const VkResult result = vkCreateImageView(logical_device, &create_info, alloc_cb, &image_view); result != VK_SUCCESS) {
    throw Error("failed to create image views!").WithCode(result);
  }
  return {
    image_view,
    [logical_device, alloc_cb](VkImageView image_view) {
      vkDestroyImageView(logical_device, image_view, alloc_cb);
    }
  };
}

HandleWrapper<VkFramebuffer> CreateFramebuffer(VkDevice logical_device, VkRenderPass render_pass, VkImageView view, VkExtent2D extent) {
  const VkAllocationCallbacks* alloc_cb = config::GetAllocationCallbacks();
  VkFramebufferCreateInfo create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
  create_info.renderPass = render_pass;
  create_info.attachmentCount = 1;
  create_info.pAttachments = &view;
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

  constexpr VkPhysicalDeviceFeatures device_features = {};

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

HandleWrapper<VkRenderPass> CreateRenderPass(VkDevice logical_device, VkFormat format) {
  const VkAllocationCallbacks* alloc_cb = config::GetAllocationCallbacks();
  VkAttachmentDescription color_attachment = {};

  color_attachment.format = format;
  color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
  color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  VkAttachmentReference color_attachment_ref = {};
  color_attachment_ref .attachment = 0;
  color_attachment_ref .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpass = {};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &color_attachment_ref;

  VkRenderPassCreateInfo render_pass_info = {};
  render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  render_pass_info.attachmentCount = 1;
  render_pass_info.pAttachments = &color_attachment;
  render_pass_info.subpassCount = 1;
  render_pass_info.pSubpasses = &subpass;

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

  create_info.oldSwapchain = VK_NULL_HANDLE;

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
    HandleWrapper<VkImageView> image_view = CreateImageView(logical_device, image, format);
    image_views.emplace_back(std::move(image_view));
  }
  return image_views;
}

std::vector<HandleWrapper<VkFramebuffer>> CreateFramebuffers(VkDevice logical_device, const std::vector<HandleWrapper<VkImageView>>& image_views, VkRenderPass render_pass,VkExtent2D extent) {
  std::vector<HandleWrapper<VkFramebuffer>> framebuffers;
  framebuffers.reserve(image_views.size());
  for(const HandleWrapper<VkImageView>& image_view : image_views) {
    HandleWrapper<VkFramebuffer> framebuffer = CreateFramebuffer(logical_device, render_pass, image_view.get(), extent);
    framebuffers.emplace_back(std::move(framebuffer));
  }
  return framebuffers;
}

HandleWrapper<VkPipelineLayout> CreatePipelineLayout(VkDevice logical_device) {
  const VkAllocationCallbacks* alloc_cb = config::GetAllocationCallbacks();
  VkPipelineLayoutCreateInfo pipeline_layout_info = {};
  pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipeline_layout_info.setLayoutCount = 0;
  pipeline_layout_info.pushConstantRangeCount = 0;

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

HandleWrapper<VkPipeline> CreatePipeline(VkDevice logical_device, VkPipelineLayout pipeline_layout, VkRenderPass render_pass, const std::vector<VkPipelineShaderStageCreateInfo> &shader_stages) {
  const std::vector<VkDynamicState> dynamic_states = config::GetDynamicStates();
  const VkAllocationCallbacks* alloc_cb = config::GetAllocationCallbacks();

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
  dynamic_state.dynamicStateCount = static_cast<uint32_t>(dynamic_states.size());
  dynamic_state.pDynamicStates = dynamic_states.data();

  VkGraphicsPipelineCreateInfo pipeline_info = {};
  pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipeline_info.stageCount = 2;
  pipeline_info.pStages = shader_stages.data();
  pipeline_info.pVertexInputState = &vertex_input_info;
  pipeline_info.pInputAssemblyState = &input_assembly;
  pipeline_info.pViewportState = &viewport_state;
  pipeline_info.pRasterizationState = &rasterizer;
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

} // namespace vk::factory