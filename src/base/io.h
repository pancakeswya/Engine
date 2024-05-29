#ifndef IO_H_
#define IO_H_

#include "error.h"

Error ReadFile(const char* path, char** content, size_t* count);

#endif  // IO_H_