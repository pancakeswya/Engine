#ifndef BACKEND_RENDER_VK_OBJECT_H_
#define BACKEND_RENDER_VK_OBJECT_H_

#include "backend/render/vk_wrappers.h"

#include <array>
#include <vulkan/vulkan.h>

namespace vk {

template<size_t kUboCount>
struct Object3D {


  std::array<Buffer, kUboCount> ubo_buffers_;
  std::array<UniformBufferObject*, kUboCount> ubo_mapped_;
};

} // namespace

#endif // BACKEND_RENDER_VK_OBJECT_H_