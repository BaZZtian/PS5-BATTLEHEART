#include "gc_disc.h"
#ifdef PS5_PORTS_WITH_SONIC
#include "sonic_gc_media.h"
#endif
#ifndef PS5_PORTS_TEST_DISC_ID
#define PS5_PORTS_TEST_DISC_ID "GSNE8P"
#endif
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdio.h>
#include <string.h>

static unsigned char image[0x2000];
static const unsigned char names[] = "\0assets\0fixture.dat\0empty.bin\0";

static void put32(size_t offset, uint32_t value) {
    image[offset] = (unsigned char)(value >> 24);
    image[offset + 1u] = (unsigned char)(value >> 16);
    image[offset + 2u] = (unsigned char)(value >> 8);
    image[offset + 3u] = (unsigned char)value;
}

static int read_memory(void *context, uint64_t offset, void *buffer, size_t length) {
    (void)context;
    if (offset > sizeof(image) || length > sizeof(image) - offset) {
        return -1;
    }
    memcpy(buffer, image + (size_t)offset, length);
    return 0;
}

static void fixture(char region) {
    memset(image, 0, sizeof(image));
    memcpy(image, PS5_PORTS_TEST_DISC_ID, 6u);
    image[3] = (unsigned char)region;
    put32(0x1c, 0xc2339f3du);
    put32(0x420, 0x500u);
    put32(0x424, 0x800u);
    put32(0x428, 48u + (uint32_t)sizeof(names));
    put32(0x500, 0x100u);
    put32(0x590, 8u);
    put32(0x800, 0x01000000u);
    put32(0x808, 4u);
    put32(0x80c, 0x01000001u);
    put32(0x810, 0u);
    put32(0x814, 3u);
    put32(0x818, 8u);
    put32(0x81c, 0x1000u);
    put32(0x820, 9u);
    put32(0x824, 20u);
    put32(0x828, 0x1010u);
    put32(0x82c, 0u);
    memcpy(image + 0x830, names, sizeof(names));
    memcpy(image + 0x1000, "SYNTHETIC", 9u);
}

int main(void) {
    GcReader reader = {NULL, sizeof(image), read_memory};
    GcDisc disc;
    GcFile file;
    unsigned char data[9];
    const char regions[] = {'E', 'P', 'J'};
    const char *labels[] = {"NTSC-U", "PAL", "NTSC-J"};
    for (size_t i = 0; i < sizeof(regions); ++i) {
        fixture(regions[i]);
        assert(gc_disc_open(&disc, &reader) == GC_OK);
        assert(strcmp(gc_disc_region(&disc), labels[i]) == 0);
        assert(gc_disc_boot_dol(&disc, &file) == GC_OK);
        assert(file.offset == 0x500u && file.size == 0x108u);
        assert(gc_disc_find(&disc, "assets/fixture.dat", &file) == GC_OK);
        assert(gc_disc_read(&disc, &file, 0u, data, sizeof(data)) == GC_OK);
        assert(memcmp(data, "SYNTHETIC", sizeof(data)) == 0);
        assert(gc_disc_read(&disc, &file, UINT64_MAX, data, 1u) == GC_OUT_OF_BOUNDS);
        assert(gc_disc_read(&disc, &file, 8u, data, 2u) == GC_OUT_OF_BOUNDS);
        assert(gc_disc_find(&disc, "/empty.bin", &file) == GC_OK);
        assert(gc_disc_read(&disc, &file, 0u, NULL, 0u) == GC_OK);
        assert(gc_disc_find(&disc, "assets/../empty.bin", &file) == GC_INVALID);
        assert(gc_disc_find(&disc, "missing.dat", &file) == GC_NOT_FOUND);
        assert(gc_disc_find(&disc, "fixture.dat", &file) == GC_NOT_FOUND);
        gc_disc_close(&disc);
#ifdef PS5_PORTS_WITH_SONIC
        assert(sonic_gc_open_media(&disc, &reader) == GC_OK);
        gc_disc_close(&disc);
#endif
    }
    fixture('P');
    reader.size = 20u;
    assert(gc_disc_open(&disc, &reader) == GC_INVALID);
    reader.size = sizeof(image);
    fixture('P'); put32(0x1c, 0u);
    assert(gc_disc_open(&disc, &reader) == GC_INVALID);
    fixture('P'); put32(0x428, UINT32_MAX);
    assert(gc_disc_open(&disc, &reader) == GC_INVALID);
    fixture('P'); put32(0x808, UINT32_MAX);
    assert(gc_disc_open(&disc, &reader) == GC_INVALID);
    fixture('P'); put32(0x814, 1u);
    assert(gc_disc_open(&disc, &reader) == GC_INVALID);
    fixture('P'); put32(0x810, 2u);
    assert(gc_disc_open(&disc, &reader) == GC_INVALID);
    fixture('P'); put32(0x818, 0x00ffffffu);
    assert(gc_disc_open(&disc, &reader) == GC_INVALID);
    fixture('P'); put32(0x81c, (uint32_t)sizeof(image) - 1u);
    assert(gc_disc_open(&disc, &reader) == GC_INVALID);
    fixture('P'); image[0x831] = '/';
    assert(gc_disc_open(&disc, &reader) == GC_INVALID);
    fixture('P'); put32(0x590, UINT32_MAX);
    assert(gc_disc_open(&disc, &reader) == GC_OK);
    assert(gc_disc_boot_dol(&disc, &file) == GC_INVALID);
    gc_disc_close(&disc);
    fixture('P'); put32(0x500, 0x80u);
    assert(gc_disc_open(&disc, &reader) == GC_OK);
    assert(gc_disc_boot_dol(&disc, &file) == GC_INVALID);
    gc_disc_close(&disc);
#ifdef PS5_PORTS_WITH_SONIC
    fixture('P'); memcpy(image, "GSAP01", 6u);
    assert(sonic_gc_open_media(&disc, &reader) == GC_WRONG_GAME);
    fixture('P'); put32(0x590, UINT32_MAX);
    assert(sonic_gc_open_media(&disc, &reader) == GC_INVALID);
#endif
    puts("PASS: synthetic PAL/NTSC-U/NTSC-J, nested reads, empty files, bounds and malformed FST rejection");
    return 0;
}
