#ifndef ZIP_H
#define ZIP_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// Extract a .lupi (zip) file to a temporary directory
// Returns: 0 on success, non-zero on failure
// out_dir: buffer to receive the temp directory path (should be at least 512 bytes)
// out_dir_size: size of out_dir buffer
int extract_lupi_to_tmp(const char *lupi_path, char *out_dir, size_t out_dir_size);

int extract_lupi_from_memory(const void *data, size_t size, const char *out_dir);

void cleanup_lupi_tmp(const char *tmp_dir);

char *create_temp_dir(char *out_dir);

#ifdef __cplusplus
}
#endif

#endif // ZIP_H
