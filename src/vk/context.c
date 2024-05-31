#include "vk/context.h"\

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

static Error messengerCreate(VkInstance instance,
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
    return kSuccess;
  }
  createMessengerFn = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
      instance, "vkCreateDebugUtilsMessengerEXT");
  if (createMessengerFn == VK_NULL_HANDLE) {
    return AppErrorCreate(kAppErrorDllGetExtFn);
  }
  destroyMessengerFn =
      (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
          instance, "vkDestroyDebugUtilsMessengerEXT");
  if (destroyMessengerFn == VK_NULL_HANDLE) {
    return AppErrorCreate(kAppErrorDllGetExtFn);
  }
  const VkResult vk_res =
      createMessengerFn(instance, create_info, NULL, messenger);
  if (vk_res != VK_SUCCESS) {
    return VulkanErrorCreate(vk_res);
  }
  return kSuccess;
}

static Error layersSupport(void) {
  uint32_t vk_layer_count = 0;
  VkResult vk_res = vkEnumerateInstanceLayerProperties(&vk_layer_count, NULL);
  if (vk_res != VK_SUCCESS) {
    return VulkanErrorCreate(vk_res);
  }
  VkLayerProperties* available_layers =
      (VkLayerProperties*)malloc(vk_layer_count * sizeof(VkLayerProperties));
  if (available_layers == NULL) {
    return StdErrorCreate(kStdErrorOutOfMemory);
  }
  vk_res =
      vkEnumerateInstanceLayerProperties(&vk_layer_count, available_layers);
  if (vk_res != VK_SUCCESS) {
    free(available_layers);
    return VulkanErrorCreate(vk_res);
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
      return AppErrorCreate(kAppErrorLayersNotSupported);
    }
  }
  free(available_layers);
  return kSuccess;
}

#endif

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
    return StdErrorCreate(kStdErrorOutOfMemory);
  }
  memcpy(*extensions, glfw_extensions, glfw_ext_count * sizeof(const char*));
#ifdef DEBUG
  static const char* kDebugUtilsExt = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
  (*extensions)[glfw_ext_count] = kDebugUtilsExt;
#endif
  return kSuccess;
}

static Error instanceCreate(
    VkInstance* instance,
    VkDebugUtilsMessengerCreateInfoEXT* messenger_create_info) {
  Error err;
#ifdef DEBUG
  err = layersSupport();
  if (!ErrorEqual(err, kSuccess)) {
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
  if (!ErrorEqual(err, kSuccess)) {
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
  const VkResult vk_res = vkCreateInstance(&create_info, NULL, instance);
  free(ext);
  if (vk_res != VK_SUCCESS) {
    return VulkanErrorCreate(vk_res);
  }
  return kSuccess;
}

Error VulkanContextCreate(VulkanContext* context, GLFWwindow* window, const uint32_t frames) {
  Error err;
  VkDebugUtilsMessengerCreateInfoEXT messenger_create_info;
#ifdef DEBUG
  err = messengerCreate(NULL, &messenger_create_info, NULL);
  if (!ErrorEqual(err, kSuccess)) {
    return err;
  }
#endif
  err = instanceCreate(&context->instance, &messenger_create_info);
  if (!ErrorEqual(err, kSuccess)) {
    VulkanContextDestroy(context);
    return err;
  }
#ifdef DEBUG
  err = messengerCreate(context->instance, &messenger_create_info,
                        &context->messenger);
  if (!ErrorEqual(err, kSuccess)) {
    VulkanContextDestroy(context);
    return err;
  }
#endif
  const VkResult vk_res = glfwCreateWindowSurface(context->instance, window, NULL, &context->surface);
  if (vk_res != VK_SUCCESS) {
    VulkanContextDestroy(context);
    return VulkanErrorCreate(vk_res);
  }
  err = VulkanDeviceCreate(context->instance, context->surface, kLayers, kLayersCount, &context->device);
  if (!ErrorEqual(err, kSuccess)) {
    VulkanContextDestroy(context);
    return err;
  }
  int width = 0, height = 0;
  glfwGetFramebufferSize(window, &width, &height);
  err = VulkanSwapchainCreate(
    context->device.logical, context->surface,
    &context->device.info, width, height,
    &context->swapchain
  );
  if (!ErrorEqual(err, kSuccess)) {
    VulkanContextDestroy(context);
    return err;
  }
  err = VulkanRenderCreate(context->device.logical, context->swapchain.format, &context->render);
  if (!ErrorEqual(err, kSuccess)) {
    VulkanContextDestroy(context);
    return err;
  }
  err = VulkanSwapchainImagesCreate(context->device.logical, context->render.pass, &context->swapchain, &context->swap_images);
  if (!ErrorEqual(err, kSuccess)) {
    VulkanContextDestroy(context);
    return err;
  }
  err = VulkanCommandCreate(context->device.logical, &context->device.info.indices, frames, &context->cmd);
  if (!ErrorEqual(err, kSuccess)) {
    VulkanContextDestroy(context);
    return err;
  }
  err = VulkanSyncCreate(context->device.logical, frames, &context->sync);
  if (!ErrorEqual(err, kSuccess)) {
    VulkanContextDestroy(context);
    return err;
  }
  return kSuccess;
}

void VulkanContextDestroy(VulkanContext* context) {
  VulkanSyncDestroy(context->device.logical, &context->sync);
  VulkanCommandDestroy(context->device.logical, &context->cmd);
  VulkanRenderDestroy(context->device.logical, &context->render);
  VulkanSwapchainDestroy(context->device.logical, &context->swapchain);
  VulkanSwapchainImagesDestroy(context->device.logical, &context->swap_images);
  VulkanDeviceDestroy(&context->device);
  if (context->surface != VK_NULL_HANDLE) {
    vkDestroySurfaceKHR(context->instance, context->surface, NULL);
  }
#ifdef DEBUG
  if (context->messenger != VK_NULL_HANDLE) {
    destroyMessengerFn(context->instance, context->messenger, NULL);
  }
#endif
  if (context->instance != VK_NULL_HANDLE) {
    vkDestroyInstance(context->instance, NULL);
  }
}
