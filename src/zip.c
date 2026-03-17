#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>

#include <archive.h>
#include <archive_entry.h>

#include "zip.h"

static int copy_data(struct archive *ar, struct archive *aw) {
    int r;
    const void *buff;
    size_t size;
    la_int64_t offset;

    for (;;) {
        r = archive_read_data_block(ar, &buff, &size, &offset);
        if (r == ARCHIVE_EOF)
            return (ARCHIVE_OK);
        if (r < ARCHIVE_OK)
            return (r);
        r = archive_write_data_block(aw, buff, size, offset);
        if (r < ARCHIVE_OK)
            return (r);
    }
}

static int create_parent_directories(const char *filepath) {
    char *path_copy = strdup(filepath);
    if (!path_copy) return -1;

    char *p = path_copy;
    while (*p) {
        if (*p == '/') {
            *p = '\0';
            if (strlen(path_copy) > 0) {
                if (mkdir(path_copy, 0755) != 0 && errno != EEXIST) {
                    free(path_copy);
                    return -1;
                }
            }
            *p = '/';
        }
        p++;
    }

    free(path_copy);
    return 0;
}

static int is_path_safe(const char *base_dir, const char *filepath) {
    // Check for path traversal attempts
    if (strstr(filepath, "..") != NULL) {
        return 0;
    }

    // Ensure filepath starts with base_dir
    size_t base_len = strlen(base_dir);
    if (strncmp(filepath, base_dir, base_len) != 0) {
        return 0;
    }

    return 1;
}

int extract_lupi_to_tmp(const char *lupi_path, char *out_dir, size_t out_dir_size) {
    struct archive *a;
    struct archive *ext;
    struct archive_entry *entry;
    int flags;
    int r;
    int ret = 0;
    int files_extracted = 0;

    // Create temp directory
    snprintf(out_dir, out_dir_size, "/tmp/lupi-XXXXXX");
    if (mkdtemp(out_dir) == NULL) {
        fprintf(stderr, "Failed to create temp directory: %s\n", strerror(errno));
        return 1;
    }

    // Select which attributes we want to restore
    flags = ARCHIVE_EXTRACT_TIME;
    flags |= ARCHIVE_EXTRACT_PERM;
    flags |= ARCHIVE_EXTRACT_FFLAGS;
    flags |= ARCHIVE_EXTRACT_SECURE_SYMLINKS;
    flags |= ARCHIVE_EXTRACT_SECURE_NODOTDOT;

    a = archive_read_new();
    ext = archive_write_disk_new();
    archive_write_disk_set_options(ext, flags);
    archive_write_disk_set_standard_lookup(ext);

    // Enable zip format and all compression filters
    archive_read_support_format_zip(a);
    archive_read_support_filter_all(a);

    // Open the .lupi file
    r = archive_read_open_filename(a, lupi_path, 10240);
    if (r != ARCHIVE_OK) {
        fprintf(stderr, "Failed to open %s: %s\n", lupi_path, archive_error_string(a));
        ret = 1;
        goto cleanup;
    }

    // Extract each entry
    while ((r = archive_read_next_header(a, &entry)) == ARCHIVE_OK) {
        const char *pathname = archive_entry_pathname(entry);
        char filepath[PATH_MAX];

        // Skip entries with path traversal or outside base dir
        snprintf(filepath, sizeof(filepath), "%s/%s", out_dir, pathname);
        if (!is_path_safe(out_dir, filepath)) {
            fprintf(stderr, "Skipping unsafe path: %s\n", pathname);
            archive_read_data_skip(a);
            continue;
        }

        // Set the final path for extraction
        archive_entry_set_pathname(entry, filepath);

        // Create parent directories if needed
        if (create_parent_directories(filepath) != 0) {
            fprintf(stderr, "Failed to create directories for: %s\n", filepath);
            archive_read_data_skip(a);
            continue;
        }

        // Extract the entry
        r = archive_write_header(ext, entry);
        if (r != ARCHIVE_OK) {
            fprintf(stderr, "Failed to write header for: %s\n", filepath);
            archive_read_data_skip(a);
            continue;
        }

        if (archive_entry_size(entry) > 0) {
            r = copy_data(a, ext);
            if (r != ARCHIVE_OK) {
                fprintf(stderr, "Failed to extract: %s\n", filepath);
                ret = 1;
                goto cleanup;
            }
        }

        r = archive_write_finish_entry(ext);
        if (r != ARCHIVE_OK) {
            fprintf(stderr, "Failed to finalize: %s\n", filepath);
            ret = 1;
            goto cleanup;
        }

        files_extracted++;
    }

    if (r != ARCHIVE_EOF) {
        fprintf(stderr, "Error reading archive: %s\n", archive_error_string(a));
        ret = 1;
    }

    if (files_extracted == 0) {
        fprintf(stderr, "No files extracted from %s\n", lupi_path);
        ret = 1;
    }

cleanup:
    archive_read_free(a);
    archive_write_free(ext);
    return ret;
}

void cleanup_lupi_tmp(const char *tmp_dir) {
    if (!tmp_dir || strlen(tmp_dir) == 0) return;

    char cmd[PATH_MAX + 32];
    snprintf(cmd, sizeof(cmd), "rm -rf '%s'", tmp_dir);
    system(cmd);
}

char *create_temp_dir(char *out_dir) {
    snprintf(out_dir, 512, "/tmp/lupi-XXXXXX");
    if (mkdtemp(out_dir) == NULL) {
        return NULL;
    }
    return out_dir;
}

struct memory_reader {
    const unsigned char *data;
    size_t size;
    size_t offset;
};

static la_ssize_t memory_reader_callback(struct archive *a, void *client_data,
                                       const void **buffer) {
    struct memory_reader *reader = (struct memory_reader *)client_data;
    (void)a;
    
    if (reader->offset >= reader->size) {
        *buffer = NULL;
        return 0;
    }
    
    *buffer = reader->data + reader->offset;
    size_t remaining = reader->size - reader->offset;
    reader->offset += remaining;
    return (la_ssize_t)remaining;
}

static la_int64_t memory_skip_callback(struct archive *a, void *client_data,
                                  la_int64_t request, int whence) {
    struct memory_reader *reader = (struct memory_reader *)client_data;
    (void)a;
    
    size_t new_offset;
    switch (whence) {
        case SEEK_SET:
            new_offset = (size_t)request;
            break;
        case SEEK_CUR:
            new_offset = reader->offset + (size_t)request;
            break;
        case SEEK_END:
            new_offset = reader->size - (size_t)(-request);
            break;
        default:
            return -1;
    }
    
    if (new_offset > reader->size) {
        return -1;
    }
    
    reader->offset = new_offset;
    return (la_int64_t)new_offset;
}

static int memory_close_callback(struct archive *a, void *client_data) {
    (void)a;
    (void)client_data;
    return ARCHIVE_OK;
}

int extract_lupi_from_memory(const void *data, size_t size, const char *out_dir) {
    struct archive *a;
    struct archive *ext;
    struct archive_entry *entry;
    int flags;
    int r;
    int ret = 0;
    int files_extracted = 0;
    
    struct memory_reader reader;
    reader.data = (const unsigned char *)data;
    reader.size = size;
    reader.offset = 0;
    
    flags = ARCHIVE_EXTRACT_TIME;
    flags |= ARCHIVE_EXTRACT_PERM;
    flags |= ARCHIVE_EXTRACT_SECURE_SYMLINKS;
    flags |= ARCHIVE_EXTRACT_SECURE_NODOTDOT;
    
    a = archive_read_new();
    ext = archive_write_disk_new();
    archive_write_disk_set_options(ext, flags);
    archive_write_disk_set_standard_lookup(ext);
    
    archive_read_support_format_zip(a);
    archive_read_support_filter_all(a);
    
    archive_read_set_seek_callback(a, memory_skip_callback);
    archive_read_set_read_callback(a, memory_reader_callback);
    archive_read_set_close_callback(a, memory_close_callback);
    archive_read_set_callback_data(a, &reader);
    
    r = archive_read_open1(a);
    if (r != ARCHIVE_OK) {
        fprintf(stderr, "Failed to open memory archive: %s\n", archive_error_string(a));
        ret = 1;
        goto cleanup;
    }
    
    while ((r = archive_read_next_header(a, &entry)) == ARCHIVE_OK) {
        const char *pathname = archive_entry_pathname(entry);
        char filepath[PATH_MAX];
        
        snprintf(filepath, sizeof(filepath), "%s/%s", out_dir, pathname);
        
        archive_entry_set_pathname(entry, filepath);
        
        if (create_parent_directories(filepath) != 0) {
            fprintf(stderr, "Failed to create directories for: %s\n", filepath);
            archive_read_data_skip(a);
            continue;
        }
        
        r = archive_write_header(ext, entry);
        if (r != ARCHIVE_OK) {
            fprintf(stderr, "Failed to write header for: %s\n", filepath);
            archive_read_data_skip(a);
            continue;
        }
        
        if (archive_entry_size(entry) > 0) {
            r = copy_data(a, ext);
            if (r != ARCHIVE_OK) {
                fprintf(stderr, "Failed to extract: %s\n", filepath);
                ret = 1;
                goto cleanup;
            }
        }
        
        r = archive_write_finish_entry(ext);
        if (r != ARCHIVE_OK) {
            fprintf(stderr, "Failed to finalize: %s\n", filepath);
            ret = 1;
            goto cleanup;
        }
        
        files_extracted++;
    }
    
    if (r != ARCHIVE_EOF) {
        fprintf(stderr, "Error reading archive: %s\n", archive_error_string(a));
        ret = 1;
    }
    
    if (files_extracted == 0) {
        fprintf(stderr, "No files extracted from memory\n");
        ret = 1;
    }
    
cleanup:
    archive_read_free(a);
    archive_write_free(ext);
    return ret;
}
