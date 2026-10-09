#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>
#include <stdint.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#if defined(_WIN32)
#include <direct.h>
static int lupi_mkdir(const char *path) { return _mkdir(path); }
#else
static int lupi_mkdir(const char *path) { return mkdir(path, 0755); }
#endif

#ifndef LIBRETRO
#include <archive.h>
#include <archive_entry.h>
#else
#include <zlib.h>
#endif

#include "zip.h"

#ifndef LIBRETRO
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
#endif

static int create_parent_directories(const char *filepath) {
    char *path_copy = strdup(filepath);
    if (!path_copy) return -1;

    char *p = path_copy;
    while (*p) {
        if (*p == '/') {
            *p = '\0';
            if (strlen(path_copy) > 0) {
                if (lupi_mkdir(path_copy) != 0 && errno != EEXIST) {
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

static const char *temp_root(void) {
    const char *p = getenv("TMPDIR");
    if (p && p[0]) return p;
    p = getenv("TEMP");
    if (p && p[0]) return p;
    p = getenv("TMP");
    if (p && p[0]) return p;
    return "/tmp";
}

static int make_temp_dir(char *out_dir, size_t out_dir_size) {
    const char *root = temp_root();
    size_t n = strlen(root);
    int i;

    while (n > 0 && (root[n - 1] == '/' || root[n - 1] == '\\')) n--;

    for (i = 0; i < 128; i++) {
        int written = snprintf(out_dir, out_dir_size, "%.*s/lupi-%ld-%d",
                               (int)n, root, (long)getpid(), i);
        if (written < 0 || (size_t)written >= out_dir_size) return -1;
        if (lupi_mkdir(out_dir) == 0) return 0;
        if (errno != EEXIST) return -1;
    }
    return -1;
}

#ifdef LIBRETRO
static uint16_t read_u16(const uint8_t *p) {
    return (uint16_t)(p[0] | (p[1] << 8));
}

static uint32_t read_u32(const uint8_t *p) {
    return (uint32_t)(p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24));
}

static int inflate_entry(FILE *in, FILE *out, uint32_t comp_size, uint32_t uncomp_size) {
    z_stream strm;
    uint8_t inbuf[4096];
    uint8_t outbuf[4096];
    int ret;
    uint32_t remaining = comp_size;
    uint32_t written = 0;

    memset(&strm, 0, sizeof(strm));
    if (inflateInit2(&strm, -MAX_WBITS) != Z_OK) return -1;

    do {
        size_t n = remaining > sizeof(inbuf) ? sizeof(inbuf) : remaining;
        if (n == 0) break;
        if (fread(inbuf, 1, n, in) != n) {
            inflateEnd(&strm);
            return -1;
        }
        remaining -= (uint32_t)n;
        strm.next_in = inbuf;
        strm.avail_in = (uInt)n;

        do {
            strm.next_out = outbuf;
            strm.avail_out = sizeof(outbuf);
            ret = inflate(&strm, Z_NO_FLUSH);
            if (ret == Z_STREAM_ERROR || ret == Z_DATA_ERROR || ret == Z_MEM_ERROR) {
                inflateEnd(&strm);
                return -1;
            }
            size_t have = sizeof(outbuf) - strm.avail_out;
            if (fwrite(outbuf, 1, have, out) != have) {
                inflateEnd(&strm);
                return -1;
            }
            written += (uint32_t)have;
        } while (strm.avail_out == 0);
    } while (ret != Z_STREAM_END && remaining > 0);

    inflateEnd(&strm);
    (void)uncomp_size;
    return (ret == Z_STREAM_END || written > 0) ? 0 : -1;
}

int extract_lupi_to_tmp(const char *lupi_path, char *out_dir, size_t out_dir_size) {
    FILE *zip = fopen(lupi_path, "rb");
    int files_extracted = 0;
    int ret = 0;

    if (make_temp_dir(out_dir, out_dir_size) != 0) {
        fprintf(stderr, "Failed to create temp directory: %s\n", strerror(errno));
        if (zip) fclose(zip);
        return 1;
    }

    if (!zip) {
        fprintf(stderr, "Failed to open %s: %s\n", lupi_path, strerror(errno));
        return 1;
    }

    for (;;) {
        uint8_t header[30];
        size_t n = fread(header, 1, 30, zip);
        if (n < 30) break;

        uint32_t sig = read_u32(header);
        if (sig == 0x02014b50u || sig == 0x06054b50u) break;
        if (sig != 0x04034b50u) {
            fprintf(stderr, "Invalid zip signature in %s\n", lupi_path);
            ret = 1;
            break;
        }

        uint16_t flags = read_u16(header + 6);
        uint16_t method = read_u16(header + 8);
        uint32_t comp_size = read_u32(header + 18);
        uint32_t uncomp_size = read_u32(header + 22);
        uint16_t namelen = read_u16(header + 26);
        uint16_t extra = read_u16(header + 28);

        char *name = (char *)malloc((size_t)namelen + 1);
        if (!name || fread(name, 1, namelen, zip) != namelen) {
            free(name);
            ret = 1;
            break;
        }
        name[namelen] = '\0';

        if (extra > 0) fseek(zip, extra, SEEK_CUR);

        if ((flags & 0x8) != 0) {
            fprintf(stderr, "Unsupported zip data descriptor in %s\n", name);
            free(name);
            ret = 1;
            break;
        }

        char filepath[PATH_MAX];
        snprintf(filepath, sizeof(filepath), "%s/%s", out_dir, name);
        if (!is_path_safe(out_dir, filepath)) {
            fprintf(stderr, "Skipping unsafe path: %s\n", name);
            fseek(zip, (long)comp_size, SEEK_CUR);
            free(name);
            continue;
        }

        int is_dir = (namelen > 0 && name[namelen - 1] == '/') ||
                     (uncomp_size == 0 && method == 0 && comp_size == 0);
        if (is_dir) {
            if (create_parent_directories(filepath) != 0) ret = 1;
            lupi_mkdir(filepath);
            free(name);
            files_extracted++;
            continue;
        }

        if (create_parent_directories(filepath) != 0) {
            fprintf(stderr, "Failed to create directories for: %s\n", filepath);
            fseek(zip, (long)comp_size, SEEK_CUR);
            free(name);
            continue;
        }

        FILE *out = fopen(filepath, "wb");
        if (!out) {
            fprintf(stderr, "Failed to write %s\n", filepath);
            fseek(zip, (long)comp_size, SEEK_CUR);
            free(name);
            ret = 1;
            continue;
        }

        if (method == 0) {
            uint8_t buf[4096];
            uint32_t left = comp_size;
            while (left > 0) {
                size_t chunk = left > sizeof(buf) ? sizeof(buf) : left;
                if (fread(buf, 1, chunk, zip) != chunk || fwrite(buf, 1, chunk, out) != chunk) {
                    ret = 1;
                    break;
                }
                left -= (uint32_t)chunk;
            }
        } else if (method == 8) {
            if (inflate_entry(zip, out, comp_size, uncomp_size) != 0) ret = 1;
        } else {
            fprintf(stderr, "Unsupported compression method %u in %s\n", method, name);
            fseek(zip, (long)comp_size, SEEK_CUR);
            ret = 1;
        }

        fclose(out);
        free(name);
        files_extracted++;
    }

    fclose(zip);
    if (files_extracted == 0) {
        fprintf(stderr, "No files extracted from %s\n", lupi_path);
        ret = 1;
    }
    return ret;
}
#else

int extract_lupi_to_tmp(const char *lupi_path, char *out_dir, size_t out_dir_size) {
    struct archive *a;
    struct archive *ext;
    struct archive_entry *entry;
    int flags;
    int r;
    int ret = 0;
    int files_extracted = 0;

    if (make_temp_dir(out_dir, out_dir_size) != 0) {
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
#endif

static int remove_tree(const char *path) {
    struct stat st;
    DIR *dir;
    struct dirent *ent;

    if (stat(path, &st) != 0) return -1;

    if ((st.st_mode & S_IFDIR) == 0) {
        return unlink(path);
    }

    dir = opendir(path);
    if (!dir) return -1;
    while ((ent = readdir(dir)) != NULL) {
        char child[PATH_MAX];
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) continue;
        snprintf(child, sizeof(child), "%s/%s", path, ent->d_name);
        remove_tree(child);
    }
    closedir(dir);
    return rmdir(path);
}

void cleanup_lupi_tmp(const char *tmp_dir) {
    if (!tmp_dir || tmp_dir[0] == '\0') return;
    remove_tree(tmp_dir);
}
