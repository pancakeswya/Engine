#include "vk/context.h"
#include "base/error.h"

#include <GLFW/glfw3.h>
#include <stdio.h>
#include <vulkan/vulkan.h>

static Error recordCommandBuffer(VulkanContext* context, const uint32_t image_index) {
    const VkCommandBufferBeginInfo begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO
    };
    VkResult vk_res = vkBeginCommandBuffer(context->cmd_buffers[0], &begin_info);
    if (vk_res != VK_SUCCESS) {
        return VulkanErrorCreate(vk_res);
    }
    const VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};

    const VkRenderPassBeginInfo render_pass_begin_info = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = context->render_pass,
        .framebuffer = context->framebuffers[image_index],
        .renderArea.offset = {0, 0},
        .renderArea.extent = context->extent,
        .clearValueCount = 1,
        .pClearValues = &clearColor
    };
    vkCmdBeginRenderPass(context->cmd_buffers[0], &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(context->cmd_buffers[0], VK_PIPELINE_BIND_POINT_GRAPHICS, context->pipeline);

    const VkViewport viewport = {
        .x = 0.0f,
        .y = 0.0f,
        .width = (float) context->extent.width,
        .height = (float) context->extent.height,
        .minDepth = 0.0f,
        .maxDepth = 1.0f,

    };
    vkCmdSetViewport(context->cmd_buffers[0], 0, 1, &viewport);

    const VkRect2D scissor = {
        .offset = {0, 0},
        .extent = context->extent
    };
    vkCmdSetScissor(context->cmd_buffers[0], 0, 1, &scissor);

    vkCmdDraw(context->cmd_buffers[0], 3, 1, 0, 0);

    vkCmdEndRenderPass(context->cmd_buffers[0]);
    vk_res = vkEndCommandBuffer(context->cmd_buffers[0]);
    if (vk_res != VK_SUCCESS) {
        return VulkanErrorCreate(vk_res);
    }
    return kErrorSuccess;
}

static Error render(VulkanContext* context) {
    VkResult vk_res = vkWaitForFences(context->logical_device, 1, &context->fence, VK_TRUE, UINT64_MAX);
    if (vk_res != VK_SUCCESS) {
        return VulkanErrorCreate(vk_res);
    }
    vk_res = vkResetFences(context->logical_device, 1, &context->fence);
    if (vk_res != VK_SUCCESS) {
        return VulkanErrorCreate(vk_res);
    }
    uint32_t image_index = 0;
    vk_res = vkAcquireNextImageKHR(context->logical_device, context->swapchain, UINT64_MAX, context->image_semaphore, VK_NULL_HANDLE, &image_index);
    if (vk_res != VK_SUCCESS) {
        return VulkanErrorCreate(vk_res);
    }
    vk_res = vkResetCommandBuffer(context->cmd_buffers[0], 0);
    if (vk_res != VK_SUCCESS) {
        return VulkanErrorCreate(vk_res);
    }
    const Error err = recordCommandBuffer(context, image_index);
    if (!ErrorEqual(err, kErrorSuccess)) {
        return err;
    }
    const VkSemaphore wait_semaphores[] = {context->image_semaphore};
    const VkSemaphore signal_semaphores[] = {context->render_semaphore};
    const VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

    const VkSubmitInfo submit_info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = wait_semaphores,
        .pWaitDstStageMask = wait_stages,
        .commandBufferCount = 1,
        .pCommandBuffers = context->cmd_buffers,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = signal_semaphores
    };
    vk_res = vkQueueSubmit(context->graphics_queue, 1, &submit_info, context->fence);
    if (vk_res != VK_SUCCESS) {
        return VulkanErrorCreate(vk_res);
    }
    const VkSwapchainKHR swapchains[] = {context->swapchain};
    const VkPresentInfoKHR present_info = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = signal_semaphores,
        .swapchainCount = 1,
        .pSwapchains = swapchains,
        .pImageIndices = &image_index
    };
    vk_res = vkQueuePresentKHR(context->present_queue, &present_info);
    if (vk_res != VK_SUCCESS) {
        return VulkanErrorCreate(vk_res);
    }
    return kErrorSuccess;
}

int main(void) {
    if (glfwInit() == GLFW_FALSE) {
        return 1;
    }
    const int kWindowWidth = 800;
    const int kWindowHeight = 600;
    const char kTitle[] = "Vulkan";

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    GLFWwindow* window = glfwCreateWindow(kWindowWidth, kWindowHeight, kTitle, NULL, NULL);
    VulkanContext context = {0};
    Error err = VulkanContextCreate(&context, window);
    if (!ErrorEqual(err, kErrorSuccess)) {
        printf("code %d, type %d\n", err.code.val, err.type);
        glfwTerminate();
        return 1;
    }
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        err = render(&context);
        if (!ErrorEqual(err, kErrorSuccess)) {
            printf("code %d, type %d\n", err.code.val, err.type);
            glfwTerminate();
            return 1;
        }
    }
    vkDeviceWaitIdle(context.logical_device);
    VulkanContextDestroy(&context);
    glfwTerminate();
    return 0;
}
