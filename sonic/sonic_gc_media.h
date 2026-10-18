#ifndef PS5_PORTS_SONIC_GC_MEDIA_H
#define PS5_PORTS_SONIC_GC_MEDIA_H
#include "gc_disc.h"
#ifdef __cplusplus
extern "C" {
#endif
int sonic_gc_open_media(GcDisc *disc, const GcReader *reader);
#ifdef __cplusplus
}
#endif
#endif
