#ifndef BASE_IO_H_
#define BASE_IO_H_

#include "base/error.h"

extern Error ReadFile(const char* path, char** content, size_t* count);

#endif  // BASE_IO_H_