#include "gc_disc.h"

#include <stdlib.h>
#include <string.h>

#define GC_HEADER_SIZE 0x440u
#define GC_FST_LIMIT (16u * 1024u * 1024u)

static uint32_t be32(const unsigned char *p) {
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] << 8) | (uint32_t)p[3];
}

static int in_range(uint64_t total, uint64_t offset, uint64_t length) {
    return offset <= total && length <= total - offset;
}

static const unsigned char *entry(const GcDisc *disc, uint32_t index) {
    return disc->fst + (size_t)index * 12u;
}

static const char *entry_name(const GcDisc *disc, uint32_t index) {
    uint32_t offset = be32(entry(disc, index)) & 0x00ffffffu;
    return (const char *)disc->fst + (size_t)disc->entry_count * 12u + offset;
}

static int validate_fst(const GcDisc *disc) {
    size_t string_start = (size_t)disc->entry_count * 12u;
    size_t string_size = (size_t)disc->fst_size - string_start;
    uint32_t *parents = malloc((size_t)disc->entry_count * sizeof(*parents));
    uint32_t *ends = malloc((size_t)disc->entry_count * sizeof(*ends));
    uint32_t depth = 1u;
    int result = GC_INVALID;
    if (!parents || !ends) {
        result = GC_NO_MEMORY;
        goto done;
    }
    parents[0] = 0u;
    ends[0] = disc->entry_count;
    for (uint32_t i = 1u; i < disc->entry_count; ++i) {
        const unsigned char *item = entry(disc, i);
        uint32_t name_offset = be32(item) & 0x00ffffffu;
        const char *name;
        while (depth > 0u && i >= ends[depth - 1u]) {
            --depth;
        }
        if (depth == 0u || item[0] > 1u || name_offset >= string_size) {
            goto done;
        }
        name = (const char *)disc->fst + string_start + name_offset;
        if (!memchr(name, 0, string_size - name_offset)) {
            goto done;
        }
        if (!name[0] || strchr(name, '/') || strchr(name, '\\') ||
            strcmp(name, ".") == 0 || strcmp(name, "..") == 0) {
            goto done;
        }
        if (item[0] == 1u) {
            uint32_t end = be32(item + 8);
            if (be32(item + 4) != parents[depth - 1u] ||
                end <= i || end > ends[depth - 1u]) {
                goto done;
            }
            parents[depth] = i;
            ends[depth] = end;
            ++depth;
        } else if (!in_range(disc->reader.size, be32(item + 4), be32(item + 8))) {
            goto done;
        }
    }
    result = GC_OK;
done:
    free(parents);
    free(ends);
    return result;
}

int gc_disc_open(GcDisc *disc, const GcReader *reader) {
    unsigned char header[GC_HEADER_SIZE];
    uint32_t fst_offset;
    int result;
    if (!disc) {
        return GC_INVALID;
    }
    memset(disc, 0, sizeof(*disc));
    if (!reader || !reader->read_at || reader->size < sizeof(header)) {
        return GC_INVALID;
    }
    if (reader->read_at(reader->context, 0u, header, sizeof(header)) != 0) {
        return GC_IO_ERROR;
    }
    if (be32(header + 0x1c) != 0xc2339f3du) {
        return GC_INVALID;
    }
    for (size_t i = 0u; i < 6u; ++i) {
        if (!((header[i] >= 'A' && header[i] <= 'Z') ||
              (header[i] >= '0' && header[i] <= '9'))) {
            return GC_INVALID;
        }
    }
    fst_offset = be32(header + 0x424);
    disc->fst_size = be32(header + 0x428);
    disc->dol_offset = be32(header + 0x420);
    if (fst_offset < GC_HEADER_SIZE || disc->fst_size < 12u ||
        disc->fst_size > GC_FST_LIMIT ||
        !in_range(reader->size, fst_offset, disc->fst_size) ||
        disc->dol_offset < GC_HEADER_SIZE ||
        !in_range(reader->size, disc->dol_offset, 0x100u)) {
        memset(disc, 0, sizeof(*disc));
        return GC_INVALID;
    }
    disc->fst = malloc(disc->fst_size);
    if (!disc->fst) {
        memset(disc, 0, sizeof(*disc));
        return GC_NO_MEMORY;
    }
    disc->reader = *reader;
    if (reader->read_at(reader->context, fst_offset, disc->fst, disc->fst_size) != 0) {
        gc_disc_close(disc);
        return GC_IO_ERROR;
    }
    disc->entry_count = be32(disc->fst + 8);
    if (be32(disc->fst) != 0x01000000u || be32(disc->fst + 4) != 0u ||
        disc->entry_count == 0u ||
        disc->entry_count > disc->fst_size / 12u) {
        gc_disc_close(disc);
        return GC_INVALID;
    }
    result = validate_fst(disc);
    if (result != GC_OK) {
        gc_disc_close(disc);
        return result;
    }
    memcpy(disc->game_id, header, 6u);
    disc->game_id[6] = 0;
    disc->disc_number = header[6];
    disc->revision = header[7];
    return GC_OK;
}

void gc_disc_close(GcDisc *disc) {
    if (disc) {
        free(disc->fst);
        memset(disc, 0, sizeof(*disc));
    }
}

int gc_disc_find(const GcDisc *disc, const char *path, GcFile *file) {
    uint32_t begin = 1u;
    uint32_t end;
    if (!disc || !disc->fst || !path || !file) {
        return GC_INVALID;
    }
    memset(file, 0, sizeof(*file));
    end = disc->entry_count;
    if (*path == '/') {
        ++path;
    }
    for (;;) {
        const char *slash = strchr(path, '/');
        size_t length = slash ? (size_t)(slash - path) : strlen(path);
        int found = 0;
        if (length == 0u || (length == 1u && path[0] == '.') ||
            (length == 2u && path[0] == '.' && path[1] == '.')) {
            return GC_INVALID;
        }
        for (uint32_t i = begin; i < end;) {
            const unsigned char *item = entry(disc, i);
            const char *name = entry_name(disc, i);
            if (strlen(name) == length && memcmp(name, path, length) == 0) {
                if (!slash) {
                    if (item[0] != 0u) {
                        return GC_NOT_FOUND;
                    }
                    file->offset = be32(item + 4);
                    file->size = be32(item + 8);
                    return GC_OK;
                }
                if (item[0] != 1u) {
                    return GC_NOT_FOUND;
                }
                begin = i + 1u;
                end = be32(item + 8);
                path = slash + 1;
                found = 1;
                break;
            }
            i = item[0] == 1u ? be32(item + 8) : i + 1u;
        }
        if (!found) {
            return GC_NOT_FOUND;
        }
    }
}

int gc_disc_read(const GcDisc *disc, const GcFile *file, uint64_t offset,
                 void *buffer, size_t length) {
    if (!disc || !disc->fst || !file || (!buffer && length != 0u)) {
        return GC_INVALID;
    }
    if (!in_range(file->size, offset, length) ||
        !in_range(disc->reader.size, file->offset, file->size)) {
        return GC_OUT_OF_BOUNDS;
    }
    if (length == 0u) {
        return GC_OK;
    }
    return disc->reader.read_at(disc->reader.context, file->offset + offset,
                                buffer, length) == 0 ? GC_OK : GC_IO_ERROR;
}

const char *gc_disc_region(const GcDisc *disc) {
    if (!disc || !disc->fst) {
        return "unknown";
    }
    switch (disc->game_id[3]) {
        case 'E': return "NTSC-U";
        case 'P': return "PAL";
        case 'J': return "NTSC-J";
        default: return "unknown";
    }
}

int gc_disc_boot_dol(const GcDisc *disc, GcFile *file) {
    unsigned char header[0x100];
    uint64_t span = sizeof(header);
    unsigned int sections = 0u;
    if (!disc || !disc->fst || !file) return GC_INVALID;
    memset(file, 0, sizeof(*file));
    if (disc->reader.read_at(disc->reader.context, disc->dol_offset,
                             header, sizeof(header)) != 0) return GC_IO_ERROR;
    for (size_t i = 0u; i < 18u; ++i) {
        uint32_t offset = be32(header + i * 4u);
        uint32_t size = be32(header + 0x90u + i * 4u);
        uint64_t end;
        if (size == 0u) continue;
        if (offset < sizeof(header) ||
            !in_range(disc->reader.size - disc->dol_offset, offset, size)) {
            return GC_INVALID;
        }
        end = (uint64_t)offset + size;
        if (end > span) span = end;
        ++sections;
    }
    if (sections == 0u) return GC_INVALID;
    file->offset = disc->dol_offset;
    file->size = span;
    return GC_OK;
}
