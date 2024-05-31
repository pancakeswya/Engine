#ifndef ERROR_H_
#define ERROR_H_

#include <vulkan/vulkan.h>
#include <stdlib.h>

typedef enum GlfwError {
  kGlfwSuccess = 0,
  kGlfwErrorInit,
  kGlfwErrorWindowCreate,
  kGlfwErrorVulkanExt,
} GlfwError;

typedef enum StdError {
  kStdSuccess = 0,
  kStdErrorOutOfMemory,
  kStdErrorCantReadFile
} StdError;

typedef enum AppError {
  kAppSuccess = 0,
  kAppErrorLayersNotSupported,
  kAppErrorDllGetExtFn,
  kAppErrorNoVulkanSupportedGpu
} AppError;

typedef enum ErrorType {
  kErrorTypeStd = 0,
  kErrorTypeApp,
  kErrorTypeVulkan,
  kErrorTypeGlfw
} ErrorType;

typedef struct Error {
  union {
    StdError std;
    AppError app;
    VkResult vulkan;
    GlfwError glfw;
    int val;
  } code;
  ErrorType type;
} Error;

static Error kSuccess = {.code.app = kAppSuccess, .type = kErrorTypeStd};

static inline int ErrorEqual(const Error err1, const Error err2) {
  return err1.type == err2.type && err1.code.val == err2.code.val;
}

#define StdErrorCreate(err) (Error){.code.std = err, .type = kErrorTypeStd}
#define AppErrorCreate(err) (Error){.code.app = err, .type = kErrorTypeApp}
#define VulkanErrorCreate(err) (Error){.code.vulkan = err, .type = kErrorTypeVulkan}
#define GlfwErrorCreate(err) (Error){.code.glfw = err, .type = kErrorTypeGlfw}
#define PrintError(err) fprintf(stderr, "err code = %d, type = %d\n", err.code.val, err.type)

#endif  // ERROR_H_
