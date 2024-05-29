#ifndef ERROR_H_
#define ERROR_H_

#include <vulkan/vulkan.h>

typedef enum GlfwError {
    kGlfwSuccess = 0,
    kErrorInit,
    kErrorVulkanExt,
} GlfwError;

typedef enum StdError {
    kStdSuccess = 0,
    kStdErrorOutOfMemory,
    kStdErrorCantReadFile
} StdError;

typedef enum AppError {
    kAppSucces = 0,
    kAppErrorLayersNotSupported,
    kAppErrorDllGetExtFn,
    kAppErrorNoVulkanSupportedGpu
} AppError;

typedef enum ErrorType {
    kErrorTypeStd = 0,
    kErrorTypeApp,
    kErrorTypeGlfw,
    kErrorTypeVulkan
} ErrorType;

typedef struct Error {
    union {
        AppError app;
        StdError std;
        VkResult vulkan;
        GlfwError glfw;
        int val;
    } code;
    ErrorType type;
} Error;


static Error kErrorSuccess = { .code = kStdSuccess, .type = kErrorTypeStd };

static inline int ErrorEqual(const Error err1, const Error err2) {
    return err1.type == err2.type && err1.code.val == err2.code.val;
}

#endif // ERROR_H_
