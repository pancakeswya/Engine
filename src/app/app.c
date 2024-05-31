#include "app/app.h"
#include "base/error.h"
#include "vk/context.h"

#include <GLFW/glfw3.h>
#include <stdbool.h>

typedef struct WindowObserver {
  bool framebuffer_resized;
} WindowObserver;

static const int kWindowWidth = 800;
static const int kWindowHeight = 600;
static const char kTitle[] = "Vulkan";
static const uint32_t kMaxFrames = 2;

static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
  WindowObserver* observer = (WindowObserver*)glfwGetWindowUserPointer(window);
  observer->framebuffer_resized = true;
}
static Error recordCommandBuffer(
  VkCommandBuffer cmd_buffer,
  VkRenderPass render_pass,
  VkPipeline pipeline,
  VkFramebuffer framebuffer,
  const VkExtent2D extent) {
    const VkCommandBufferBeginInfo begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO
    };
    VkResult vk_res = vkBeginCommandBuffer(cmd_buffer, &begin_info);
    if (vk_res != VK_SUCCESS) {
        return VulkanErrorCreate(vk_res);
    }
    const VkClearValue clearColors[] = {
      {
          .color = {
            .float32 = {0.0f, 0.0f, 0.0f, 1.0f}
          }
        }
    };

    const VkRenderPassBeginInfo render_pass_begin_info = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = render_pass,
        .framebuffer = framebuffer,
        .renderArea.offset = {0, 0},
        .renderArea.extent = extent,
        .clearValueCount = sizeof(clearColors) / sizeof(VkClearValue),
        .pClearValues = clearColors
    };
    vkCmdBeginRenderPass(cmd_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

    const VkViewport viewport = {
        .x = 0.0f,
        .y = 0.0f,
        .width = (float) extent.width,
        .height = (float) extent.height,
        .minDepth = 0.0f,
        .maxDepth = 1.0f,

    };
    vkCmdSetViewport(cmd_buffer, 0, 1, &viewport);

    const VkRect2D scissor = {
        .offset = {0, 0},
        .extent = extent
    };
    vkCmdSetScissor(cmd_buffer, 0, 1, &scissor);

    vkCmdDraw(cmd_buffer, 3, 1, 0, 0);

    vkCmdEndRenderPass(cmd_buffer);
    vk_res = vkEndCommandBuffer(cmd_buffer);
    if (vk_res != VK_SUCCESS) {
        return VulkanErrorCreate(vk_res);
    }
    return kSuccess;
}

static Error ResizeEvent(VulkanContext* context, GLFWwindow* window) {
  int width = 0, height = 0;
  glfwGetFramebufferSize(window, &width, &height);
  while (width == 0 || height == 0) {
    glfwGetFramebufferSize(window, &width, &height);
    glfwWaitEvents();
  }
  vkDeviceWaitIdle(context->device.logical);
  VulkanSwapchainDestroy(context->device.logical, &context->swapchain);
  VulkanSwapchainImagesDestroy(context->device.logical, &context->swap_images);
  Error err = VulkanSwapchainCreate(
    context->device.logical, context->surface,
    &context->device.info, width, height, &context->swapchain
  );
  if (!ErrorEqual(err, kSuccess)) {
    return err;
  }
  err = VulkanSwapchainImagesCreate(context->device.logical, context->render.pass, &context->swapchain, &context->swap_images);
  if (!ErrorEqual(err, kSuccess)) {
    return err;
  }
  return kSuccess;
}

static Error render(VulkanContext* context, GLFWwindow* window, WindowObserver* observer, const uint32_t frame) {
  VkResult vk_res = vkWaitForFences(context->device.logical, 1, context->sync.fences + frame, VK_TRUE, UINT64_MAX);
  if (vk_res != VK_SUCCESS) {
    return VulkanErrorCreate(vk_res);
  }
  uint32_t image_index = 0;
  vk_res = vkAcquireNextImageKHR(context->device.logical, context->swapchain.swapchain, UINT64_MAX, context->sync.image_semaphores[frame], VK_NULL_HANDLE, &image_index);
  if (vk_res == VK_ERROR_OUT_OF_DATE_KHR) {
    const Error err = ResizeEvent(context, window);
    if (!ErrorEqual(err, kSuccess)) {
      return err;
    }
    return kSuccess;
  }
  if (vk_res != VK_SUCCESS && vk_res != VK_SUBOPTIMAL_KHR) {
    return VulkanErrorCreate(vk_res);
  }
  vk_res = vkResetFences(context->device.logical, 1, context->sync.fences + frame);
  if (vk_res != VK_SUCCESS) {
    return VulkanErrorCreate(vk_res);
  }
  vk_res = vkResetCommandBuffer(context->cmd.buffers[frame], 0);
  if (vk_res != VK_SUCCESS) {
    return VulkanErrorCreate(vk_res);
  }
  Error err = recordCommandBuffer(
    context->cmd.buffers[frame],
    context->render.pass,
    context->render.pipeline,
    context->swap_images.framebuffers[image_index],
    context->swapchain.extent
  );
  if (!ErrorEqual(err, kSuccess)) {
    return err;
  }
  const VkSemaphore wait_semaphores[] = {context->sync.image_semaphores[frame]};
  const VkSemaphore signal_semaphores[] = {context->sync.render_semaphores[frame]};
  const VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

  const VkSubmitInfo submit_info = {
    .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
    .waitSemaphoreCount = sizeof(wait_semaphores) / sizeof(VkSemaphore),
    .pWaitSemaphores = wait_semaphores,
    .pWaitDstStageMask = wait_stages,
    .commandBufferCount = 1,
    .pCommandBuffers = context->cmd.buffers + frame,
    .signalSemaphoreCount = sizeof(signal_semaphores) / sizeof(VkSemaphore),
    .pSignalSemaphores = signal_semaphores
};
  vk_res = vkQueueSubmit(context->device.graphics_queue, 1, &submit_info, context->sync.fences[frame]);
  if (vk_res != VK_SUCCESS) {
    return VulkanErrorCreate(vk_res);
  }
  const VkSwapchainKHR swapchains[] = {context->swapchain.swapchain};
  const VkPresentInfoKHR present_info = {
    .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
    .waitSemaphoreCount = sizeof(signal_semaphores) / sizeof(VkSemaphore),
    .pWaitSemaphores = signal_semaphores,
    .swapchainCount = 1,
    .pSwapchains = swapchains,
    .pImageIndices = &image_index
};
  vk_res = vkQueuePresentKHR(context->device.present_queue, &present_info);
  if (vk_res == VK_ERROR_OUT_OF_DATE_KHR || vk_res == VK_SUBOPTIMAL_KHR || observer->framebuffer_resized) {
    observer->framebuffer_resized = false;
    err = ResizeEvent(context, window);
    if (!ErrorEqual(err, kSuccess)) {
      return err;
    }
  } else if (vk_res != VK_SUCCESS) {
    return VulkanErrorCreate(vk_res);
  }
  return kSuccess;
}


int AppRun(void) {
  if (glfwInit() == GLFW_FALSE) {
    return 1;
  }
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
  GLFWwindow* window = glfwCreateWindow(kWindowWidth, kWindowHeight, kTitle, NULL, NULL);
  if (window == NULL) {
    glfwTerminate();
    return 1;
  }
  WindowObserver* observer = &(WindowObserver){ .framebuffer_resized = false };
  VulkanContext* vk_ctx = &(VulkanContext){0};

  glfwSetWindowUserPointer(window, &observer);
  glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
  Error err = VulkanContextCreate(vk_ctx, window, kMaxFrames);
  if (!ErrorEqual(err, kSuccess)) {
    PrintError(err);
    glfwTerminate();
    return 1;
  }
  int app_res = 0;
  uint32_t frame = 0;
  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
    err = render(vk_ctx, window, observer, frame);
    if (!ErrorEqual(err, kSuccess)) {
      app_res = 1;
      PrintError(err);
      break;
    }
    frame = (frame + 1) % kMaxFrames;
  }
  vkDeviceWaitIdle(vk_ctx->device.logical);
  VulkanContextDestroy(vk_ctx);
  glfwTerminate();
  return app_res;
}