#ifndef PS5_PORTS_GC_DISC_H
#define PS5_PORTS_GC_DISC_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*GcReadAt)(void *context, uint64_t offset, void *buffer, size_t length);

typedef struct {
    void *context;
    uint64_t size;
    GcReadAt read_at;
} GcReader;

typedef struct {
    uint64_t offset;
    uint64_t size;
} GcFile;

typedef struct {
    GcReader reader;
    unsigned char *fst;
    uint32_t fst_size;
    uint32_t entry_count;
    uint32_t dol_offset;
    char game_id[7];
    unsigned char disc_number;
    unsigned char revision;
} GcDisc;

enum {
    GC_OK = 0,
    GC_INVALID = -1,
    GC_IO_ERROR = -2,
    GC_NO_MEMORY = -3,
    GC_NOT_FOUND = -4,
    GC_OUT_OF_BOUNDS = -5,
    GC_WRONG_GAME = -6
};

/* A reader returns zero only after filling the entire requested buffer. */
int gc_disc_open(GcDisc *disc, const GcReader *reader);
void gc_disc_close(GcDisc *disc);
int gc_disc_find(const GcDisc *disc, const char *path, GcFile *file);
int gc_disc_read(const GcDisc *disc, const GcFile *file, uint64_t offset,
                 void *buffer, size_t length);
const char *gc_disc_region(const GcDisc *disc);
int gc_disc_boot_dol(const GcDisc *disc, GcFile *file);

#ifdef __cplusplus
}
#endif

#endif
