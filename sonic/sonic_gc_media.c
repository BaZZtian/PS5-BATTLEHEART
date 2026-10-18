#include "sonic_gc_media.h"
#include <string.h>

int sonic_gc_open_media(GcDisc *disc, const GcReader *reader) {
    int result = gc_disc_open(disc, reader);
    GcFile dol;
    if (result != GC_OK) return result;
    if (memcmp(disc->game_id, "GSN", 3u) != 0 ||
        memcmp(disc->game_id + 4, "8P", 2u) != 0 ||
        disc->disc_number != 0u ||
        (disc->game_id[3] != 'E' && disc->game_id[3] != 'P' && disc->game_id[3] != 'J')) {
        gc_disc_close(disc);
        return GC_WRONG_GAME;
    }
    result = gc_disc_boot_dol(disc, &dol);
    if (result != GC_OK) gc_disc_close(disc);
    return result;
}
