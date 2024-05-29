#include "vk/context.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "base/io.h"

typedef struct QueueFamilyIndices {
  uint32_t graphics, present;
} QueueFamilyIndices;

typedef struct SurfaceSupportDetails {
  VkSurfaceCapabilitiesKHR capabilities;
  VkSurfaceFormatKHR* formats;
  uint32_t formats_count;
  VkPresentModeKHR* present_modes;
  uint32_t present_modes_count;
} SurfaceSupportDetails;

static const char* kDeviceExtensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
static const uint32_t kDeviceExtensionsCount =
    sizeof(kDeviceExtensions) / sizeof(const char*);
static const VkDynamicState kPipelineDynamicStates[] = {
    VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
static const uint32_t kPipelineDynamicStatesCount =
    sizeof(kPipelineDynamicStates) / sizeof(VkDynamicState);
#ifdef DEBUG
static const char* kLayers[] = {"VK_LAYER_KHRONOS_validation"};
static const uint32_t kLayersCount = sizeof(kLayers) / sizeof(const char*);
static PFN_vkCreateDebugUtilsMessengerEXT createMessengerFn = VK_NULL_HANDLE;
static PFN_vkDestroyDebugUtilsMessengerEXT destroyMessengerFn = VK_NULL_HANDLE;

static VKAPI_ATTR VkBool32 VKAPI_CALL
messengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
                  VkDebugUtilsMessageTypeFlagsEXT message_type,
                  const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
                  void* user_data) {
  puts(callback_data->pMessage);
  return VK_FALSE;
}

static Error messengerCreate(const VkInstance instance,
                             VkDebugUtilsMessengerCreateInfoEXT* create_info,
                             VkDebugUtilsMessengerEXT* messenger) {
  if (messenger == VK_NULL_HANDLE) {
    *create_info = (VkDebugUtilsMessengerCreateInfoEXT){
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
        .pfnUserCallback = messengerCallback};
    return kErrorSuccess;
  }
  createMessengerFn = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
      instance, "vkCreateDebugUtilsMessengerEXT");
  if (createMessengerFn == VK_NULL_HANDLE) {
    return (Error){kAppErrorDllGetExtFn, kErrorTypeApp};
  }
  destroyMessengerFn =
      (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
          instance, "vkDestroyDebugUtilsMessengerEXT");
  if (destroyMessengerFn == VK_NULL_HANDLE) {
    return (Error){kAppErrorDllGetExtFn, kErrorTypeApp};
  }
  const VkResult res =
      createMessengerFn(instance, create_info, NULL, messenger);
  if (res != VK_SUCCESS) {
    return (Error){res, kErrorTypeVulkan};
  }
  return kErrorSuccess;
}

static Error layersSupport(void) {
  uint32_t vk_layer_count = 0;
  VkResult vk_res = vkEnumerateInstanceLayerProperties(&vk_layer_count, NULL);
  if (vk_res != VK_SUCCESS) {
    return (Error){vk_res, kErrorTypeVulkan};
  }
  VkLayerProperties* available_layers =
      (VkLayerProperties*)malloc(vk_layer_count * sizeof(VkLayerProperties));
  if (available_layers == NULL) {
    return (Error){kStdErrorOutOfMemory, kErrorTypeStd};
  }
  vk_res =
      vkEnumerateInstanceLayerProperties(&vk_layer_count, available_layers);
  if (vk_res != VK_SUCCESS) {
    free(available_layers);
    return (Error){vk_res, kErrorTypeVulkan};
  }
  for (uint32_t i = 0; i < kLayersCount; ++i) {
    bool support = false;
    for (uint32_t j = 0; j < vk_layer_count; ++j) {
      if (strcmp(kLayers[i], available_layers[j].layerName) == 0) {
        support = true;
        break;
      }
    }
    if (support == false) {
      free(available_layers);
      return (Error){kAppErrorLayersNotSupported, kErrorTypeApp};
    }
  }
  free(available_layers);
  return kErrorSuccess;
}

#endif

static inline uint32_t clamp(const uint32_t val, const uint32_t min,
                             const uint32_t max) {
  if (val < min) return min;
  if (val > max) return max;
  return val;
}

static void freeSurfaceSupportDetails(const SurfaceSupportDetails* details) {
  free(details->formats);
  free(details->present_modes);
}

static Error getInstanceExtensions(const char*** extensions, uint32_t* count) {
  uint32_t glfw_ext_count = 0;
  const char** glfw_extensions =
      glfwGetRequiredInstanceExtensions(&glfw_ext_count);
  *count = glfw_ext_count;
#ifdef DEBUG
  *count += 1;
#endif
  *extensions = (const char**)malloc(*count * sizeof(const char*));
  if (*extensions == NULL) {
    return (Error){kStdErrorOutOfMemory, kErrorTypeStd};
  }
  memcpy(*extensions, glfw_extensions, glfw_ext_count * sizeof(const char*));
#ifdef DEBUG
  static const char* kDebugUtilsExt = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
  (*extensions)[glfw_ext_count] = kDebugUtilsExt;
#endif
  return kErrorSuccess;
}

static Error deviceExtensionsSupport(const VkPhysicalDevice device,
                                     bool* support) {
  *support = true;
  uint32_t extension_count = 0;
  VkResult vk_res = vkEnumerateDeviceExtensionProperties(
      device, NULL, &extension_count, NULL);
  if (vk_res != VK_SUCCESS) {
    return (Error){vk_res, kErrorTypeVulkan};
  }
  VkExtensionProperties* available_extensions = (VkExtensionProperties*)malloc(
      extension_count * sizeof(VkExtensionProperties));
  vk_res = vkEnumerateDeviceExtensionProperties(device, NULL, &extension_count,
                                                available_extensions);
  if (vk_res != VK_SUCCESS) {
    free(available_extensions);
    return (Error){vk_res, kErrorTypeVulkan};
  }
  for (uint32_t i = 0; i < kDeviceExtensionsCount; ++i) {
    bool found = false;
    for (uint32_t j = 0; j < extension_count; ++j) {
      if (strcmp(kDeviceExtensions[i], available_extensions[j].extensionName) ==
          0) {
        found = true;
        break;
      }
    }
    if (!found) {
      *support = false;
      break;
    }
  }
  free(available_extensions);
  return kErrorSuccess;
}

static Error SurfaceSupport(const VkPhysicalDevice device,
                            const VkSurfaceKHR surface,
                            SurfaceSupportDetails* details) {
  VkSurfaceCapabilitiesKHR capabilities = {0};
  VkResult vk_res =
      vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &capabilities);
  if (vk_res != VK_SUCCESS) {
    return (Error){vk_res, kErrorTypeVulkan};
  }
  uint32_t formats_count = 0;
  vk_res = vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formats_count,
                                                NULL);
  if (vk_res != VK_SUCCESS) {
    return (Error){vk_res, kErrorTypeVulkan};
  }
  VkSurfaceFormatKHR* formats = NULL;
  if (formats_count != 0) {
    formats =
        (VkSurfaceFormatKHR*)malloc(formats_count * sizeof(VkSurfaceFormatKHR));
    if (formats == NULL) {
      return (Error){kStdErrorOutOfMemory, kErrorTypeStd};
    }
    vk_res = vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface,
                                                  &formats_count, formats);
    if (vk_res != VK_SUCCESS) {
      free(formats);
      return (Error){vk_res, kErrorTypeVulkan};
    }
  }
  uint32_t present_modes_count = 0;
  vk_res = vkGetPhysicalDeviceSurfacePresentModesKHR(
      device, surface, &present_modes_count, NULL);
  if (vk_res != VK_SUCCESS) {
    free(formats);
    return (Error){vk_res, kErrorTypeVulkan};
  }
  VkPresentModeKHR* present_modes = NULL;
  if (present_modes_count != 0) {
    present_modes = (VkPresentModeKHR*)malloc(present_modes_count *
                                              sizeof(VkPresentModeKHR));
    if (present_modes == NULL) {
      free(formats);
      return (Error){kStdErrorOutOfMemory, kErrorTypeStd};
    }
    vk_res = vkGetPhysicalDeviceSurfacePresentModesKHR(
        device, surface, &present_modes_count, present_modes);
    if (vk_res != VK_SUCCESS) {
      free(present_modes);
      free(formats);
      return (Error){vk_res, kErrorTypeVulkan};
    }
  }
  *details =
      (SurfaceSupportDetails){.capabilities = capabilities,
                              .formats = formats,
                              .formats_count = formats_count,
                              .present_modes = present_modes,
                              .present_modes_count = present_modes_count};
  return kErrorSuccess;
}

static Error findQueueFamilyIndices(const VkPhysicalDevice device,
                                    const VkSurfaceKHR surface,
                                    QueueFamilyIndices* indices,
                                    SurfaceSupportDetails* details,
                                    bool* found) {
  *found = false;
  uint32_t graphics = 0, present = 0, families_count = 0;
  bool graphics_found = false, present_found = false;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &families_count, NULL);

  VkQueueFamilyProperties* families = (VkQueueFamilyProperties*)malloc(
      families_count * sizeof(VkQueueFamilyProperties));
  if (families == NULL) {
    return (Error){kStdErrorOutOfMemory, kErrorTypeStd};
  }
  vkGetPhysicalDeviceQueueFamilyProperties(device, &families_count, families);

  for (uint32_t i = 0; i < families_count; ++i) {
    if (families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      graphics_found = true;
      graphics = i;
    }
    VkBool32 present_support = false;
    const VkResult vk_res = vkGetPhysicalDeviceSurfaceSupportKHR(
        device, i, surface, &present_support);
    if (vk_res != VK_SUCCESS) {
      free(families);
      return (Error){vk_res, kErrorTypeVulkan};
    }
    if (present_support) {
      present_found = true;
      present = i;
    }
    if (graphics_found && present_found) {
      bool ext_support;
      Error err = deviceExtensionsSupport(device, &ext_support);
      if (!ErrorEqual(err, kErrorSuccess)) {
        free(families);
        return err;
      }
      if (ext_support) {
        err = SurfaceSupport(device, surface, details);
        if (!ErrorEqual(err, kErrorSuccess)) {
          free(families);
          return err;
        }
        if (details->formats && details->present_modes) {
          *found = true;
          *indices =
              (QueueFamilyIndices){.present = present, .graphics = graphics};
          break;
        }
      }
    }
  }
  free(families);
  return kErrorSuccess;
}

static Error createPhysicalDevice(const VkInstance instance,
                                  const VkSurfaceKHR surface,
                                  VkPhysicalDevice* device,
                                  QueueFamilyIndices* indices,
                                  SurfaceSupportDetails* details) {
  uint32_t device_count = 0;
  VkResult vk_res = vkEnumeratePhysicalDevices(instance, &device_count, NULL);
  if (vk_res != VK_SUCCESS) {
    return (Error){vk_res, kErrorTypeVulkan};
  }
  if (device_count == 0) {
    return (Error){kAppErrorNoVulkanSupportedGpu, kErrorTypeApp};
  }
  VkPhysicalDevice* devices =
      (VkPhysicalDevice*)malloc(device_count * sizeof(VkPhysicalDevice));
  if (devices == NULL) {
    return (Error){kStdErrorOutOfMemory, kErrorTypeStd};
  }
  vk_res = vkEnumeratePhysicalDevices(instance, &device_count, devices);
  if (vk_res != VK_SUCCESS) {
    free(devices);
    return (Error){vk_res, kErrorTypeVulkan};
  }
  for (uint32_t i = 0; i < device_count; ++i) {
    bool found = false;
    const Error err =
        findQueueFamilyIndices(devices[i], surface, indices, details, &found);
    if (!ErrorEqual(err, kErrorSuccess)) {
      free(devices);
      return err;
    }
    if (found) {
      *device = devices[i];
    }
  }
  free(devices);
  return kErrorSuccess;
}

static Error createLogicalDevice(const VkPhysicalDevice physical_device,
                                 const QueueFamilyIndices* indices,
                                 VkDevice* logical_device) {
  const uint32_t family_ids[] = {indices->graphics, indices->present};
  const size_t unique_family_count =
      (indices->graphics == indices->present) ? 1 : 2;
  VkDeviceQueueCreateInfo* queue_create_infos =
      (VkDeviceQueueCreateInfo*)malloc(unique_family_count *
                                       sizeof(VkDeviceQueueCreateInfo));
  if (queue_create_infos == NULL) {
    return (Error){kStdErrorOutOfMemory, kErrorTypeStd};
  }
  const float queue_priority = 1.0f;
  for (uint32_t i = 0; i < unique_family_count; ++i) {
    const VkDeviceQueueCreateInfo queue_create_info = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = family_ids[i],
        .queueCount = 1,
        .pQueuePriorities = &queue_priority};
    queue_create_infos[i] = queue_create_info;
  }
  const VkPhysicalDeviceFeatures device_features = {0};

  const VkDeviceCreateInfo create_info = {
      .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
      .queueCreateInfoCount = unique_family_count,
      .pQueueCreateInfos = queue_create_infos,
      .pEnabledFeatures = &device_features,
      .enabledExtensionCount = kDeviceExtensionsCount,
      .ppEnabledExtensionNames = kDeviceExtensions,
#ifdef DEBUG
      .enabledLayerCount = kLayersCount,
      .ppEnabledLayerNames = kLayers,
#endif
  };
  const VkResult vk_res =
      vkCreateDevice(physical_device, &create_info, NULL, logical_device);
  free(queue_create_infos);
  if (vk_res != VK_SUCCESS) {
    return (Error){vk_res, kErrorTypeVulkan};
  }
  return kErrorSuccess;
}

static Error instanceCreate(
    VkInstance* instance,
    VkDebugUtilsMessengerCreateInfoEXT* messenger_create_info) {
  Error err;
#ifdef DEBUG
  err = layersSupport();
  if (!ErrorEqual(err, kErrorSuccess)) {
    return err;
  }
#else
  (void)messenger_create_info;
#endif
  const VkApplicationInfo app_info = {
      .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
      .pApplicationName = "Hello Triangle",
      .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
      .pEngineName = "No Engine",
      .engineVersion = VK_MAKE_VERSION(1, 0, 0),
      .apiVersion = VK_API_VERSION_1_0};
  uint32_t ext_count;
  const char** ext;
  err = getInstanceExtensions(&ext, &ext_count);
  if (!ErrorEqual(err, kErrorSuccess)) {
    return err;
  }
  const VkInstanceCreateInfo create_info = {
      .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
      .pApplicationInfo = &app_info,
      .enabledExtensionCount = ext_count,
      .ppEnabledExtensionNames = ext,
#ifdef DEBUG
      .enabledLayerCount = kLayersCount,
      .ppEnabledLayerNames = kLayers,
      .pNext = messenger_create_info
#endif
  };
  const VkResult res = vkCreateInstance(&create_info, NULL, instance);
  free(ext);
  if (res != VK_SUCCESS) {
    return (Error){res, kErrorTypeVulkan};
  }
  return kErrorSuccess;
}

static VkSurfaceFormatKHR chooseSwapSurfaceFormat(
    const VkSurfaceFormatKHR available_formats[], const uint32_t count) {
  for (uint32_t i = 0; i < count; ++i) {
    if (available_formats[i].format == VK_FORMAT_B8G8R8A8_SRGB &&
        available_formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      return available_formats[i];
    }
  }
  return available_formats[0];
}

static VkPresentModeKHR chooseSwapPresentMode(
    const VkPresentModeKHR available_present_modes[], const uint32_t count) {
  for (uint32_t i = 0; i < count; ++i) {
    if (available_present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
      return available_present_modes[i];
    }
  }
  return VK_PRESENT_MODE_FIFO_KHR;
}

static VkExtent2D chooseSwapExtent(
    GLFWwindow* window, const VkSurfaceCapabilitiesKHR* capabilities) {
  if (capabilities->currentExtent.width != UINT32_MAX) {
    return capabilities->currentExtent;
  }
  int width, height;
  glfwGetFramebufferSize(window, &width, &height);
  VkExtent2D actual_extent = {(uint32_t)width, (uint32_t)height};
  actual_extent.width =
      clamp(actual_extent.width, capabilities->minImageExtent.width,
            capabilities->maxImageExtent.width);
  actual_extent.height =
      clamp(actual_extent.height, capabilities->minImageExtent.height,
            capabilities->maxImageExtent.height);

  return actual_extent;
}

static Error createSwapchain(GLFWwindow* window, const VkDevice logical_device,
                             const VkSurfaceKHR surface,
                             const QueueFamilyIndices* indices,
                             const SurfaceSupportDetails* details,
                             VkExtent2D* extent_ptr, VkFormat* format_ptr,
                             VkSwapchainKHR* swapchain) {
  const VkSurfaceFormatKHR surface_format =
      chooseSwapSurfaceFormat(details->formats, details->formats_count);
  const VkPresentModeKHR present_mode = chooseSwapPresentMode(
      details->present_modes, details->present_modes_count);

  const VkExtent2D extent = chooseSwapExtent(window, &details->capabilities);
  const VkFormat format = surface_format.format;

  uint32_t image_count = details->capabilities.minImageCount + 1;
  if (details->capabilities.maxImageCount > 0 &&
      image_count > details->capabilities.maxImageCount) {
    image_count = details->capabilities.maxImageCount;
  }

  VkSwapchainCreateInfoKHR create_info = {
      .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
      .surface = surface,
      .minImageCount = image_count,
      .imageFormat = surface_format.format,
      .imageColorSpace = surface_format.colorSpace,
      .imageExtent = extent,
      .imageArrayLayers = 1,
      .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
  };
  const uint32_t queue_family_indices[] = {indices->graphics, indices->present};

  if (indices->graphics != indices->present) {
    create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    create_info.queueFamilyIndexCount = 2;
    create_info.pQueueFamilyIndices = queue_family_indices;
  } else {
    create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  }

  create_info.preTransform = details->capabilities.currentTransform;
  create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  create_info.presentMode = present_mode;
  create_info.clipped = VK_TRUE;

  create_info.oldSwapchain = VK_NULL_HANDLE;

  const VkResult vk_res =
      vkCreateSwapchainKHR(logical_device, &create_info, NULL, swapchain);
  if (vk_res != VK_SUCCESS) {
    return (Error){vk_res, kErrorTypeVulkan};
  }
  *extent_ptr = extent;
  *format_ptr = format;

  return kErrorSuccess;
}

static Error createImages(const VkDevice logical_device,
                          const VkSwapchainKHR swapchain, VkImage** images_ptr,
                          uint32_t* image_count_ptr) {
  uint32_t image_count = 0;
  VkResult vk_res =
      vkGetSwapchainImagesKHR(logical_device, swapchain, &image_count, NULL);
  if (vk_res != VK_SUCCESS) {
    return (Error){vk_res, kErrorTypeVulkan};
  }
  VkImage* images = (VkImage*)malloc(image_count * sizeof(VkImage));
  if (images == NULL) {
    return (Error){kStdErrorOutOfMemory, kErrorTypeStd};
  }
  vk_res =
      vkGetSwapchainImagesKHR(logical_device, swapchain, &image_count, images);
  if (vk_res != VK_SUCCESS) {
    free(images);
    return (Error){vk_res, kErrorTypeVulkan};
  }
  *images_ptr = images;
  *image_count_ptr = image_count;
  return kErrorSuccess;
}

static Error createRenderPass(const VkDevice logical_device,
                              const VkFormat format,
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
    return (Error){vk_res, kErrorTypeVulkan};
  }
  return kErrorSuccess;
}

static Error createPipelineLayout(const VkDevice logical_device,
                                  VkPipelineLayout* pipeline_layout) {
  const VkPipelineLayoutCreateInfo pipeline_layout_info = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      .setLayoutCount = 0,
      .pushConstantRangeCount = 0};
  const VkResult vk_res = vkCreatePipelineLayout(
      logical_device, &pipeline_layout_info, NULL, pipeline_layout);
  if (vk_res != VK_SUCCESS) {
    return (Error){vk_res, kErrorTypeVulkan};
  }
  return kErrorSuccess;
}

static Error createShaderModule(const VkDevice logical_device, const char* path,
                                VkShaderModule* module) {
  size_t read = 0;
  char* code = NULL;
  const Error err = ReadFile(path, &code, &read);
  if (!ErrorEqual(err, kErrorSuccess)) {
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
    return (Error){vk_res, kErrorTypeVulkan};
  }
  return kErrorSuccess;
}

static Error createPipeline(const VkDevice logical_device,
                            const VkPipelineLayout pipeline_layout,
                            const VkRenderPass render_pass,
                            VkPipeline* pipeline) {
  VkShaderModule vert_shader_module = VK_NULL_HANDLE;
  VkShaderModule frag_shader_module = VK_NULL_HANDLE;
  Error err = createShaderModule(logical_device, "shaders/vert.spv",
                                 &vert_shader_module);
  if (!ErrorEqual(err, kErrorSuccess)) {
    return err;
  }
  err = createShaderModule(logical_device, "shaders/frag.spv",
                           &frag_shader_module);
  if (!ErrorEqual(err, kErrorSuccess)) {
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
    return (Error){vk_res, kErrorTypeVulkan};
  }
  return kErrorSuccess;
}

static Error createCmdPool(const VkDevice logical_device,
                           const QueueFamilyIndices* indices,
                           VkCommandPool* cmd_pool) {
  const VkCommandPoolCreateInfo create_info = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
      .queueFamilyIndex = indices->graphics,
      .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT};
  const VkResult vk_res =
      vkCreateCommandPool(logical_device, &create_info, NULL, cmd_pool);
  if (vk_res != VK_SUCCESS) {
    return (Error){vk_res, kErrorTypeVulkan};
  }
  return kErrorSuccess;
}

static Error createCmdBuffers(const VkDevice logical_device,
                              const VkCommandPool cmd_pool,
                              const uint32_t count,
                              VkCommandBuffer** cmd_buffers_ptr) {
  const VkCommandBufferAllocateInfo alloc_info = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .commandPool = cmd_pool,
      .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandBufferCount = count};
  VkCommandBuffer* cmd_buffers =
      (VkCommandBuffer*)malloc(count * sizeof(VkCommandBuffer));
  const VkResult vk_res =
      vkAllocateCommandBuffers(logical_device, &alloc_info, cmd_buffers);
  if (vk_res != VK_SUCCESS) {
    free(cmd_buffers);
    return (Error){vk_res, kErrorTypeVulkan};
  }
  *cmd_buffers_ptr = cmd_buffers;
  return kErrorSuccess;
}

static Error createImageView(const VkDevice logical_device, const VkImage image,
                             const VkFormat format, VkImageView* view) {
  const VkImageViewCreateInfo create_info = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      .image = image,
      .viewType = VK_IMAGE_VIEW_TYPE_2D,
      .format = format,
      .components.r = VK_COMPONENT_SWIZZLE_IDENTITY,
      .components.g = VK_COMPONENT_SWIZZLE_IDENTITY,
      .components.b = VK_COMPONENT_SWIZZLE_IDENTITY,
      .components.a = VK_COMPONENT_SWIZZLE_IDENTITY,
      .subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
      .subresourceRange.baseMipLevel = 0,
      .subresourceRange.levelCount = 1,
      .subresourceRange.baseArrayLayer = 0,
      .subresourceRange.layerCount = 1};
  const VkResult vk_res =
      vkCreateImageView(logical_device, &create_info, NULL, view);
  if (vk_res != VK_SUCCESS) {
    return (Error){vk_res, kErrorTypeVulkan};
  }
  return kErrorSuccess;
}

static Error createImageViews(const VkDevice logical_device,
                              const VkImage* images, const uint32_t image_count,
                              const VkFormat format,
                              VkImageView** image_views_ptr,
                              uint32_t* image_view_count_ptr) {
  VkImageView* image_views =
      (VkImageView*)malloc(image_count * sizeof(VkImageView));
  if (image_views == NULL) {
    return (Error){kStdErrorOutOfMemory, kErrorTypeStd};
  }
  for (uint32_t i = 0; i < image_count; ++i) {
    const Error err =
        createImageView(logical_device, images[i], format, &image_views[i]);
    if (!ErrorEqual(err, kErrorSuccess)) {
      free(image_views);
      return err;
    }
  }
  *image_views_ptr = image_views;
  *image_view_count_ptr = image_count;
  return kErrorSuccess;
}

static Error createFramebuffer(const VkDevice logical_device,
                               const VkRenderPass render_pass,
                               const VkImageView view, const VkExtent2D extent,
                               VkFramebuffer* framebuffer) {
  const VkFramebufferCreateInfo create_info = {
      .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
      .renderPass = render_pass,
      .attachmentCount = 1,
      .pAttachments = &view,
      .width = extent.width,
      .height = extent.height,
      .layers = 1};
  const VkResult vk_res =
      vkCreateFramebuffer(logical_device, &create_info, NULL, framebuffer);
  if (vk_res != VK_SUCCESS) {
    return (Error){vk_res, kErrorTypeVulkan};
  }
  return kErrorSuccess;
}

static Error createFramebuffers(const VkDevice logical_device,
                                const VkRenderPass render_pass,
                                const VkImageView* image_views,
                                const uint32_t image_view_count,
                                const VkExtent2D extent,
                                VkFramebuffer** framebuffers_ptr,
                                uint32_t* framebuffers_count_ptr) {
  VkFramebuffer* framebuffers =
      (VkFramebuffer*)malloc(image_view_count * sizeof(VkFramebuffer));
  if (framebuffers == NULL) {
    return (Error){kStdErrorOutOfMemory, kErrorTypeStd};
  }
  for (uint32_t i = 0; i < image_view_count; ++i) {
    const Error err = createFramebuffer(
        logical_device, render_pass, image_views[i], extent, &framebuffers[i]);
    if (!ErrorEqual(err, kErrorSuccess)) {
      free(framebuffers);
      return err;
    }
  }
  *framebuffers_ptr = framebuffers;
  *framebuffers_count_ptr = image_view_count;
  return kErrorSuccess;
}

static Error createSemaphore(const VkDevice logical_device,
                             VkSemaphore* semaphore) {
  const VkSemaphoreCreateInfo create_info = {
      .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
  const VkResult vk_res =
      vkCreateSemaphore(logical_device, &create_info, NULL, semaphore);
  if (vk_res != VK_SUCCESS) {
    return (Error){vk_res, kErrorTypeVulkan};
  }
  return kErrorSuccess;
}

static Error createFence(const VkDevice logical_device, VkFence* fence) {
  const VkFenceCreateInfo create_info = {
      .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
      .flags = VK_FENCE_CREATE_SIGNALED_BIT};
  const VkResult vk_res =
      vkCreateFence(logical_device, &create_info, NULL, fence);
  if (vk_res != VK_SUCCESS) {
    return (Error){vk_res, kErrorTypeVulkan};
  }
  return kErrorSuccess;
}

Error VulkanContextCreate(VulkanContext* context, GLFWwindow* window) {
  Error err;
  VkDebugUtilsMessengerCreateInfoEXT messenger_create_info;
#ifdef DEBUG
  err = messengerCreate(NULL, &messenger_create_info, NULL);
  if (!ErrorEqual(err, kErrorSuccess)) {
    return err;
  }
#endif
  err = instanceCreate(&context->instance, &messenger_create_info);
  if (!ErrorEqual(err, kErrorSuccess)) {
    VulkanContextDestroy(context);
    return err;
  }
#ifdef DEBUG
  err = messengerCreate(context->instance, &messenger_create_info,
                        &context->messenger);
  if (!ErrorEqual(err, kErrorSuccess)) {
    VulkanContextDestroy(context);
    return err;
  }
#endif
  VkResult vk_res = glfwCreateWindowSurface(context->instance, window, NULL,
                                            &context->surface);
  if (vk_res != VK_SUCCESS) {
    VulkanContextDestroy(context);
    return (Error){vk_res, kErrorTypeVulkan};
  }
  QueueFamilyIndices indices = {0};
  SurfaceSupportDetails details = {0};
  err = createPhysicalDevice(context->instance, context->surface,
                             &context->physical_device, &indices, &details);
  if (!ErrorEqual(err, kErrorSuccess)) {
    VulkanContextDestroy(context);
    return err;
  }
  err = createLogicalDevice(context->physical_device, &indices,
                            &context->logical_device);
  if (!ErrorEqual(err, kErrorSuccess)) {
    freeSurfaceSupportDetails(&details);
    VulkanContextDestroy(context);
    return err;
  }
  vkGetDeviceQueue(context->logical_device, indices.graphics, 0,
                   &context->graphics_queue);
  vkGetDeviceQueue(context->logical_device, indices.present, 0,
                   &context->present_queue);
  err = createSwapchain(window, context->logical_device, context->surface,
                        &indices, &details, &context->extent, &context->format,
                        &context->swapchain);
  freeSurfaceSupportDetails(&details);
  if (!ErrorEqual(err, kErrorSuccess)) {
    VulkanContextDestroy(context);
    return err;
  }
  err = createImages(context->logical_device, context->swapchain,
                     &context->images, &context->image_count);
  if (!ErrorEqual(err, kErrorSuccess)) {
    VulkanContextDestroy(context);
    return err;
  }
  err = createImageViews(context->logical_device, context->images,
                         context->image_count, context->format,
                         &context->image_views, &context->image_view_count);
  if (!ErrorEqual(err, kErrorSuccess)) {
    VulkanContextDestroy(context);
    return err;
  }
  err = createRenderPass(context->logical_device, context->format,
                         &context->render_pass);
  if (!ErrorEqual(err, kErrorSuccess)) {
    VulkanContextDestroy(context);
    return err;
  }
  err =
      createPipelineLayout(context->logical_device, &context->pipeline_layout);
  if (!ErrorEqual(err, kErrorSuccess)) {
    VulkanContextDestroy(context);
    return err;
  }
  err = createPipeline(context->logical_device, context->pipeline_layout,
                       context->render_pass, &context->pipeline);
  if (!ErrorEqual(err, kErrorSuccess)) {
    VulkanContextDestroy(context);
    return err;
  }
  err = createFramebuffers(context->logical_device, context->render_pass,
                           context->image_views, context->image_view_count,
                           context->extent, &context->framebuffers,
                           &context->framebuffer_count);
  if (!ErrorEqual(err, kErrorSuccess)) {
    VulkanContextDestroy(context);
    return err;
  }
  err = createCmdPool(context->logical_device, &indices, &context->cmd_pool);
  if (!ErrorEqual(err, kErrorSuccess)) {
    VulkanContextDestroy(context);
    return err;
  }
  err = createCmdBuffers(context->logical_device, context->cmd_pool, 1,
                         &context->cmd_buffers);
  if (!ErrorEqual(err, kErrorSuccess)) {
    VulkanContextDestroy(context);
    return err;
  }
  err = createSemaphore(context->logical_device, &context->image_semaphore);
  if (!ErrorEqual(err, kErrorSuccess)) {
    VulkanContextDestroy(context);
    return err;
  }
  err = createSemaphore(context->logical_device, &context->render_semaphore);
  if (!ErrorEqual(err, kErrorSuccess)) {
    VulkanContextDestroy(context);
    return err;
  }
  err = createFence(context->logical_device, &context->fence);
  if (!ErrorEqual(err, kErrorSuccess)) {
    VulkanContextDestroy(context);
    return err;
  }
  return kErrorSuccess;
}

void VulkanContextDestroy(const VulkanContext* context) {
  if (context->fence != VK_NULL_HANDLE) {
    vkDestroyFence(context->logical_device, context->fence, NULL);
  }
  if (context->render_semaphore != VK_NULL_HANDLE) {
    vkDestroySemaphore(context->logical_device, context->render_semaphore,
                       NULL);
  }
  if (context->image_semaphore != VK_NULL_HANDLE) {
    vkDestroySemaphore(context->logical_device, context->image_semaphore, NULL);
  }
  if (context->cmd_buffers != NULL) {
    vkFreeCommandBuffers(context->logical_device, context->cmd_pool, 1,
                         context->cmd_buffers);
    free(context->cmd_buffers);
  }
  if (context->cmd_pool != VK_NULL_HANDLE) {
    vkDestroyCommandPool(context->logical_device, context->cmd_pool, NULL);
  }
  if (context->framebuffers != NULL) {
    for (size_t i = 0; i < context->framebuffer_count; ++i) {
      vkDestroyFramebuffer(context->logical_device, context->framebuffers[i],
                           NULL);
    }
    free(context->framebuffers);
  }
  if (context->pipeline != VK_NULL_HANDLE) {
    vkDestroyPipeline(context->logical_device, context->pipeline, NULL);
  }
  if (context->pipeline_layout != VK_NULL_HANDLE) {
    vkDestroyPipelineLayout(context->logical_device, context->pipeline_layout,
                            NULL);
  }
  if (context->render_pass != VK_NULL_HANDLE) {
    vkDestroyRenderPass(context->logical_device, context->render_pass, NULL);
  }
  if (context->image_views != NULL) {
    for (size_t i = 0; i < context->image_view_count; ++i) {
      vkDestroyImageView(context->logical_device, context->image_views[i],
                         NULL);
    }
    free(context->image_views);
  }
  if (context->images != NULL) {
    free(context->images);
  }
  if (context->swapchain != VK_NULL_HANDLE) {
    vkDestroySwapchainKHR(context->logical_device, context->swapchain, NULL);
  }
  if (context->logical_device != VK_NULL_HANDLE) {
    vkDestroyDevice(context->logical_device, NULL);
  }
#ifdef DEBUG
  if (context->messenger != VK_NULL_HANDLE) {
    destroyMessengerFn(context->instance, context->messenger, NULL);
  }
#endif
  if (context->surface != VK_NULL_HANDLE) {
    vkDestroySurfaceKHR(context->instance, context->surface, NULL);
  }
  if (context->instance == VK_NULL_HANDLE) {
    return;
  }
  vkDestroyInstance(context->instance, NULL);
}
