#include "sonic_media_adapter.h"
#include <cstdio>

int main(int argc, char** argv) {
    // Diagnostic entry point: local argv path for host, fixed private path on PS5.
    const char* path = argc == 2 ? argv[1] : "/data/battleheart/game.iso";
    battleheart::SonicMediaAdapter media;
    const int result = media.open(path);
    if (result != GC_OK) {
        std::fprintf(stderr, "BattleHeart P1.1 media validation failed: %d\n", result);
        return 1;
    }
    GcFile dol{};
    if (media.boot_dol(dol) != GC_OK) return 1;
    const auto identity = media.identity();
    std::printf("{\"game_id\":\"%s\",\"revision\":%u,\"dol_offset\":%llu,"
                "\"dol_size\":%llu,\"gameplay_verified\":false}\n",
                identity.game_id, identity.revision,
                static_cast<unsigned long long>(dol.offset),
                static_cast<unsigned long long>(dol.size));
    return 0;
}
