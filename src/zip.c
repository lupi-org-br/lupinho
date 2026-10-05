#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
#include <io.h>
#include <windows.h>
#define lupi_mkdir(path) _mkdir(path)
#define lupi_rmdir(path) _rmdir(path)
#define lupi_unlink(path) _unlink(path)
#ifndef PATH_MAX
#define PATH_MAX MAX_PATH
#endif
#else
#include <dirent.h>
#include <limits.h>
#include <unistd.h>
#define lupi_mkdir(path) mkdir((path), 0755)
#define lupi_rmdir(path) rmdir(path)
#define lupi_unlink(path) unlink(path)
#endif

#include <zlib.h>

#include "zip.h"

#define ZIP_LOCAL_SIG   0x04034b50u
#define ZIP_CENTRAL_SIG 0x02014b50u
#define ZIP_EOCD_SIG    0x06054b50u
#define ZIP_STORED      0
#define ZIP_DEFLATED    8

static uint16_t read_u16(const unsigned char *p) {
    return (uint16_t)(p[0] | (p[1] << 8));
}

static uint32_t read_u32(const unsigned char *p) {
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

static int join_path(char *out, size_t out_size, const char *base, const char *rel) {
    if (snprintf(out, out_size, "%s/%s", base, rel) < 0 || strlen(out) >= out_size - 1) {
        return -1;
    }
    return 0;
}

static int create_parent_directories(const char *filepath) {
    char *path_copy = strdup(filepath);
    if (!path_copy) return -1;

    for (char *p = path_copy; *p; p++) {
        if (*p == '/' || *p == '\\') {
            char saved = *p;
            *p = '\0';
            if (path_copy[0] != '\0'
#ifdef _WIN32
                && !(path_copy[1] == ':' && path_copy[2] == '\0')
#endif
                && lupi_mkdir(path_copy) != 0 && errno != EEXIST) {
                free(path_copy);
                return -1;
            }
            *p = saved;
        }
    }

    free(path_copy);
    return 0;
}

static int is_path_safe(const char *base_dir, const char *filepath) {
    if (strstr(filepath, "..") != NULL) {
        return 0;
    }

    size_t base_len = strlen(base_dir);
    if (strncmp(filepath, base_dir, base_len) != 0) {
        return 0;
    }

    return 1;
}

static int make_temp_dir(char *out_dir, size_t out_dir_size) {
#ifdef _WIN32
    char temp_path[MAX_PATH];
    DWORD n = GetTempPathA((DWORD)sizeof(temp_path), temp_path);
    if (n == 0 || n >= sizeof(temp_path)) {
        return -1;
    }
    for (int attempt = 0; attempt < 100; attempt++) {
        if (snprintf(out_dir, out_dir_size, "%slupi-%lu-%u", temp_path,
                     (unsigned long)GetCurrentProcessId(),
                     (unsigned)(GetTickCount() + (unsigned)attempt)) < 0) {
            return -1;
        }
        if (lupi_mkdir(out_dir) == 0) {
            return 0;
        }
    }
    return -1;
#else
    if (snprintf(out_dir, out_dir_size, "/tmp/lupi-XXXXXX") < 0) {
        return -1;
    }
    if (mkdtemp(out_dir) == NULL) {
        return -1;
    }
    return 0;
#endif
}

static int inflate_file(FILE *in, FILE *out, uint32_t comp_size, uint32_t uncomp_size) {
    z_stream strm;
    unsigned char inbuf[16384];
    unsigned char outbuf[16384];
    uint32_t remaining = comp_size;
    int ret;

    memset(&strm, 0, sizeof(strm));
    ret = inflateInit2(&strm, -MAX_WBITS);
    if (ret != Z_OK) {
        return -1;
    }

    do {
        size_t chunk = remaining > sizeof(inbuf) ? sizeof(inbuf) : remaining;
        if (chunk == 0) {
            break;
        }
        if (fread(inbuf, 1, chunk, in) != chunk) {
            inflateEnd(&strm);
            return -1;
        }
        remaining -= (uint32_t)chunk;
        strm.next_in = inbuf;
        strm.avail_in = (uInt)chunk;

        do {
            strm.next_out = outbuf;
            strm.avail_out = sizeof(outbuf);
            ret = inflate(&strm, remaining ? Z_NO_FLUSH : Z_FINISH);
            if (ret != Z_OK && ret != Z_STREAM_END) {
                inflateEnd(&strm);
                return -1;
            }
            size_t have = sizeof(outbuf) - strm.avail_out;
            if (have > 0 && fwrite(outbuf, 1, have, out) != have) {
                inflateEnd(&strm);
                return -1;
            }
        } while (strm.avail_out == 0);
    } while (ret != Z_STREAM_END);

    inflateEnd(&strm);
    if (strm.total_out != uncomp_size) {
        return -1;
    }
    return 0;
}

static int copy_stored(FILE *in, FILE *out, uint32_t size) {
    unsigned char buf[16384];
    while (size > 0) {
        size_t chunk = size > sizeof(buf) ? sizeof(buf) : size;
        if (fread(buf, 1, chunk, in) != chunk) return -1;
        if (fwrite(buf, 1, chunk, out) != chunk) return -1;
        size -= (uint32_t)chunk;
    }
    return 0;
}

static int find_eocd(FILE *fp, uint32_t *cd_offset, uint32_t *cd_size, uint16_t *entries) {
    if (fseek(fp, 0, SEEK_END) != 0) return -1;
    long file_size = ftell(fp);
    if (file_size < 22) return -1;

    long max_comment = 65535;
    long scan = file_size < max_comment + 22 ? file_size : max_comment + 22;
    unsigned char *buf = malloc((size_t)scan);
    if (!buf) return -1;

    if (fseek(fp, file_size - scan, SEEK_SET) != 0) {
        free(buf);
        return -1;
    }
    if (fread(buf, 1, (size_t)scan, fp) != (size_t)scan) {
        free(buf);
        return -1;
    }

    for (long i = scan - 22; i >= 0; i--) {
        if (read_u32(buf + i) == ZIP_EOCD_SIG) {
            *entries = read_u16(buf + i + 10);
            *cd_size = read_u32(buf + i + 12);
            *cd_offset = read_u32(buf + i + 16);
            free(buf);
            return 0;
        }
    }

    free(buf);
    return -1;
}

int extract_lupi_to_tmp(const char *lupi_path, char *out_dir, size_t out_dir_size) {
    FILE *fp = NULL;
    unsigned char *cd = NULL;
    int ret = 1;
    int files_extracted = 0;
    uint32_t cd_offset = 0;
    uint32_t cd_size = 0;
    uint16_t entries = 0;

    if (make_temp_dir(out_dir, out_dir_size) != 0) {
        fprintf(stderr, "Failed to create temp directory: %s\n", strerror(errno));
        return 1;
    }

    fp = fopen(lupi_path, "rb");
    if (!fp) {
        fprintf(stderr, "Failed to open %s: %s\n", lupi_path, strerror(errno));
        goto cleanup;
    }

    if (find_eocd(fp, &cd_offset, &cd_size, &entries) != 0 || cd_size == 0) {
        fprintf(stderr, "Invalid zip archive: %s\n", lupi_path);
        goto cleanup;
    }

    cd = malloc(cd_size);
    if (!cd) {
        fprintf(stderr, "Out of memory reading zip directory\n");
        goto cleanup;
    }

    if (fseek(fp, (long)cd_offset, SEEK_SET) != 0 || fread(cd, 1, cd_size, fp) != cd_size) {
        fprintf(stderr, "Failed to read zip central directory\n");
        goto cleanup;
    }

    size_t offset = 0;
    for (uint16_t i = 0; i < entries; i++) {
        if (offset + 46 > cd_size || read_u32(cd + offset) != ZIP_CENTRAL_SIG) {
            fprintf(stderr, "Corrupt zip central directory\n");
            goto cleanup;
        }

        uint16_t method = read_u16(cd + offset + 10);
        uint32_t comp_size = read_u32(cd + offset + 20);
        uint32_t uncomp_size = read_u32(cd + offset + 24);
        uint16_t name_len = read_u16(cd + offset + 28);
        uint16_t extra_len = read_u16(cd + offset + 30);
        uint16_t comment_len = read_u16(cd + offset + 32);
        uint32_t local_off = read_u32(cd + offset + 42);

        if (offset + 46 + name_len > cd_size) {
            fprintf(stderr, "Corrupt zip file name\n");
            goto cleanup;
        }

        char name[PATH_MAX];
        if (name_len >= sizeof(name)) {
            fprintf(stderr, "Skipping overly long path in archive\n");
            offset += 46 + name_len + extra_len + comment_len;
            continue;
        }
        memcpy(name, cd + offset + 46, name_len);
        name[name_len] = '\0';
        offset += 46 + name_len + extra_len + comment_len;

        if (name[0] == '\0') continue;
        int is_dir = name[name_len - 1] == '/' || name[name_len - 1] == '\\';

        char filepath[PATH_MAX];
        if (join_path(filepath, sizeof(filepath), out_dir, name) != 0 || !is_path_safe(out_dir, filepath)) {
            fprintf(stderr, "Skipping unsafe path: %s\n", name);
            continue;
        }

        if (create_parent_directories(filepath) != 0) {
            fprintf(stderr, "Failed to create directories for: %s\n", filepath);
            continue;
        }

        if (is_dir) {
            lupi_mkdir(filepath);
            files_extracted++;
            continue;
        }

        unsigned char local[30];
        if (fseek(fp, (long)local_off, SEEK_SET) != 0 || fread(local, 1, 30, fp) != 30 ||
            read_u32(local) != ZIP_LOCAL_SIG) {
            fprintf(stderr, "Failed to read local header for: %s\n", filepath);
            continue;
        }

        uint16_t local_name_len = read_u16(local + 26);
        uint16_t local_extra_len = read_u16(local + 28);
        if (fseek(fp, local_name_len + local_extra_len, SEEK_CUR) != 0) {
            fprintf(stderr, "Failed to skip local extra data for: %s\n", filepath);
            continue;
        }

        FILE *out = fopen(filepath, "wb");
        if (!out) {
            fprintf(stderr, "Failed to write: %s\n", filepath);
            continue;
        }

        int ok = 0;
        if (method == ZIP_STORED) {
            ok = copy_stored(fp, out, comp_size) == 0;
        } else if (method == ZIP_DEFLATED) {
            ok = inflate_file(fp, out, comp_size, uncomp_size) == 0;
        } else {
            fprintf(stderr, "Unsupported zip method %u for: %s\n", method, filepath);
        }

        fclose(out);
        if (!ok) {
            fprintf(stderr, "Failed to extract: %s\n", filepath);
            lupi_unlink(filepath);
            goto cleanup;
        }

        files_extracted++;
    }

    if (files_extracted == 0) {
        fprintf(stderr, "No files extracted from %s\n", lupi_path);
        goto cleanup;
    }

    ret = 0;

cleanup:
    if (fp) fclose(fp);
    free(cd);
    return ret;
}

#ifdef _WIN32
static void remove_tree(const char *path) {
    char pattern[MAX_PATH];
    WIN32_FIND_DATAA find_data;
    HANDLE handle;

    if (snprintf(pattern, sizeof(pattern), "%s\\*", path) < 0) return;
    handle = FindFirstFileA(pattern, &find_data);
    if (handle == INVALID_HANDLE_VALUE) {
        lupi_rmdir(path);
        lupi_unlink(path);
        return;
    }

    do {
        if (strcmp(find_data.cFileName, ".") == 0 || strcmp(find_data.cFileName, "..") == 0) {
            continue;
        }
        char child[MAX_PATH];
        if (snprintf(child, sizeof(child), "%s\\%s", path, find_data.cFileName) < 0) continue;
        if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            remove_tree(child);
        } else {
            lupi_unlink(child);
        }
    } while (FindNextFileA(handle, &find_data));

    FindClose(handle);
    lupi_rmdir(path);
}
#else
static void remove_tree(const char *path) {
    DIR *dir = opendir(path);
    if (!dir) {
        lupi_unlink(path);
        return;
    }

    struct dirent *ent;
    while ((ent = readdir(dir)) != NULL) {
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) continue;
        char child[PATH_MAX];
        if (snprintf(child, sizeof(child), "%s/%s", path, ent->d_name) < 0) continue;

        struct stat st;
        if (stat(child, &st) == 0 && S_ISDIR(st.st_mode)) {
            remove_tree(child);
        } else {
            lupi_unlink(child);
        }
    }

    closedir(dir);
    lupi_rmdir(path);
}
#endif

void cleanup_lupi_tmp(const char *tmp_dir) {
    if (!tmp_dir || strlen(tmp_dir) == 0) return;
    remove_tree(tmp_dir);
}
