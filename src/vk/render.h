#ifndef VK_RENDER_H_
#define VK_RENDER_H_

#include "base/error.h"

#include <vulkan/vulkan.h>

typedef struct VulkanRender {
  VkRenderPass pass;
  VkPipeline pipeline;
  VkPipelineLayout pipeline_layout;
} VulkanRender;

extern Error VulkanRenderCreate(VkDevice logical_device, VkFormat format, VulkanRender* render);
extern void VulkanRenderDestroy(VkDevice logical_device, VulkanRender* render);

#endif // VK_RENDER_H_