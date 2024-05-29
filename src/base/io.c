#include "io.h"

#include <stdio.h>
#include <stdlib.h>

static inline long int fileSize(FILE* file) {
  fseek(file, 0L, SEEK_END);
  const long int n_bytes = ftell(file);
  fseek(file, 0L, SEEK_SET);
  if (n_bytes > 0) {
    return n_bytes;
  }
  return 0;
}

Error ReadFile(const char* path, char** content, size_t* count) {
  FILE* file = fopen(path, "rb");
  if (file == NULL) {
    return (Error){kStdErrorCantReadFile, kErrorTypeStd};
  }
  const long int n = fileSize(file);
  char* buffer = (char*)malloc((n + 1) * sizeof(char));
  if (buffer == NULL) {
    return (Error){kStdErrorOutOfMemory, kErrorTypeStd};
  }
  const size_t read = fread(buffer, sizeof(char), n, file);
  if (read != (size_t)n) {
    free(buffer);
    return (Error){kStdErrorCantReadFile, kErrorTypeStd};
  }
  buffer[read] = '\0';

  *count = read;
  *content = buffer;

  fclose(file);
  return kErrorSuccess;
}