#define _FILE_OFFSET_BITS 64
#define _POSIX_C_SOURCE 200809L
#include "gc_disc.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#if !defined(_WIN32)
#include <sys/types.h>
#endif

static int read_file(void *context, uint64_t offset, void *buffer, size_t length) {
    FILE *file = context;
    if (offset > INT64_MAX) return -1;
#if defined(_WIN32)
    if (_fseeki64(file, (int64_t)offset, SEEK_SET) != 0) return -1;
#else
    if (fseeko(file, (off_t)offset, SEEK_SET) != 0) return -1;
#endif
    return fread(buffer, 1u, length, file) == length ? 0 : -1;
}

int main(int argc, char **argv) {
    FILE *file;
    int64_t size;
    GcReader reader;
    GcDisc disc;
    GcFile dol;
    int result;
    if (argc != 2) {
        fputs("Usage: gc_inspect <own-uncompressed-gamecube.iso>\n", stderr);
        return 2;
    }
    file = fopen(argv[1], "rb");
    if (!file) return 2;
#if defined(_WIN32)
    if (_fseeki64(file, 0, SEEK_END) != 0) { fclose(file); return 2; }
    size = _ftelli64(file);
#else
    if (fseeko(file, 0, SEEK_END) != 0) { fclose(file); return 2; }
    size = (int64_t)ftello(file);
#endif
    if (size < 0) { fclose(file); return 2; }
    reader = (GcReader){file, (uint64_t)size, read_file};
    result = gc_disc_open(&disc, &reader);
    if (result == GC_OK) {
        result = gc_disc_boot_dol(&disc, &dol);
        if (result == GC_OK) {
            printf("{\"game_id\":\"%s\",\"region\":\"%s\",\"revision\":%u,"
                   "\"disc\":%u,\"fst_entries\":%u,\"dol_offset\":%u,"
                   "\"dol_size\":%llu,\"gameplay_verified\":false}\n",
                   disc.game_id, gc_disc_region(&disc),
                   (unsigned int)disc.revision, (unsigned int)disc.disc_number,
                   disc.entry_count, disc.dol_offset, (unsigned long long)dol.size);
        }
        gc_disc_close(&disc);
    }
    if (result != GC_OK) {
        fprintf(stderr, "Disc validation failed: %d\n", result);
    }
    fclose(file);
    return result == GC_OK ? 0 : 1;
}
