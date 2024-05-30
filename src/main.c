#include "vk/context.h"
#include "base/error.h"

#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdbool.h>
#include <vulkan/vulkan.h>

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

static Error render(VulkanContext* context, GLFWwindow* window, const uint32_t curr_frame, bool* framebuffer_resized) {
    VkResult vk_res = vkWaitForFences(context->logical_device, 1, context->fences + curr_frame, VK_TRUE, UINT64_MAX);
    if (vk_res != VK_SUCCESS) {
        return VulkanErrorCreate(vk_res);
    }
    uint32_t image_index = 0;
    vk_res = vkAcquireNextImageKHR(context->logical_device, context->swapchain, UINT64_MAX, context->image_semaphores[curr_frame], VK_NULL_HANDLE, &image_index);
    if (vk_res == VK_ERROR_OUT_OF_DATE_KHR) {
      const Error err = VulkanContextRecreateSwapchain(context, window);
      if (!ErrorEqual(err, kSuccess)) {
        return err;
      }
      return kSuccess;
    }
    if (vk_res != VK_SUCCESS && vk_res != VK_SUBOPTIMAL_KHR) {
        return VulkanErrorCreate(vk_res);
    }
    vk_res = vkResetFences(context->logical_device, 1, context->fences + curr_frame);
    if (vk_res != VK_SUCCESS) {
      return VulkanErrorCreate(vk_res);
    }
    vk_res = vkResetCommandBuffer(context->cmd_buffers[curr_frame], 0);
    if (vk_res != VK_SUCCESS) {
        return VulkanErrorCreate(vk_res);
    }
    Error err = recordCommandBuffer(
      context->cmd_buffers[curr_frame],
      context->render_pass,
      context->pipeline,
      context->framebuffers[image_index],
      context->extent
    );
    if (!ErrorEqual(err, kSuccess)) {
        return err;
    }
    const VkSemaphore wait_semaphores[] = {context->image_semaphores[curr_frame]};
    const VkSemaphore signal_semaphores[] = {context->render_semaphores[curr_frame]};
    const VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

    const VkSubmitInfo submit_info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = sizeof(wait_semaphores) / sizeof(VkSemaphore),
        .pWaitSemaphores = wait_semaphores,
        .pWaitDstStageMask = wait_stages,
        .commandBufferCount = 1,
        .pCommandBuffers = context->cmd_buffers + curr_frame,
        .signalSemaphoreCount = sizeof(signal_semaphores) / sizeof(VkSemaphore),
        .pSignalSemaphores = signal_semaphores
    };
    vk_res = vkQueueSubmit(context->graphics_queue, 1, &submit_info, context->fences[curr_frame]);
    if (vk_res != VK_SUCCESS) {
        return VulkanErrorCreate(vk_res);
    }
    const VkSwapchainKHR swapchains[] = {context->swapchain};
    const VkPresentInfoKHR present_info = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = sizeof(signal_semaphores) / sizeof(VkSemaphore),
        .pWaitSemaphores = signal_semaphores,
        .swapchainCount = 1,
        .pSwapchains = swapchains,
        .pImageIndices = &image_index
    };
    vk_res = vkQueuePresentKHR(context->present_queue, &present_info);
    if (vk_res == VK_ERROR_OUT_OF_DATE_KHR || vk_res == VK_SUBOPTIMAL_KHR || *framebuffer_resized) {
      *framebuffer_resized = false;
      err = VulkanContextRecreateSwapchain(context, window);
      if (!ErrorEqual(err, kSuccess)) {
        return err;
      }
    } else if (vk_res != VK_SUCCESS) {
      return VulkanErrorCreate(vk_res);
    }
    return kSuccess;
}

typedef struct WindowUser {
  bool framebuffer_resized;
} WindowUser;

static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
  WindowUser* user = (WindowUser*)(glfwGetWindowUserPointer(window));
  user->framebuffer_resized = true;
}

int main(void) {
    if (glfwInit() == GLFW_FALSE) {
        return 1;
    }
    const int kWindowWidth = 800;
    const int kWindowHeight = 600;
    const char kTitle[] = "Vulkan";
    WindowUser user = {0};

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    GLFWwindow* window = glfwCreateWindow(kWindowWidth, kWindowHeight, kTitle, NULL, NULL);
    glfwSetWindowUserPointer(window, &user);
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
    VulkanContext context = {0};
    const uint32_t kMaxFrames = 2;
    Error err = VulkanContextCreate(&context, window, kMaxFrames);
    if (!ErrorEqual(err, kSuccess)) {
        printf("code %d, type %d\n", err.code.val, err.type);
        glfwTerminate();
        return 1;
    }
    for (uint32_t curr_frame = 0; !glfwWindowShouldClose(window); curr_frame = (curr_frame + 1) % kMaxFrames) {
        glfwPollEvents();
        err = render(&context, window, curr_frame, &user.framebuffer_resized);
        if (!ErrorEqual(err, kSuccess)) {
            printf("code %d, type %d\n", err.code.val, err.type);
            break;
        }
    }
    vkDeviceWaitIdle(context.logical_device);
    VulkanContextDestroy(&context);
    glfwTerminate();
    return 0;
}
