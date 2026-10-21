#pragma once
#include "gc_disc.h"
#include <memory>
#include <mutex>

namespace battleheart {
class SonicMediaAdapter;
class IsoMusicStream;
// Shared synchronization token, not shared ownership of the adapter or its data.
struct MediaAccess final {
    std::mutex mutex;
    SonicMediaAdapter* owner = nullptr;
    uint64_t generation = 0;
};
struct MediaIdentity final {
    char game_id[7]{};
    unsigned revision = 0;
};
// Owns the descriptor and FST metadata; never extracts or executes game code.
class SonicMediaAdapter final {
public:
    SonicMediaAdapter();
    ~SonicMediaAdapter();
    SonicMediaAdapter(const SonicMediaAdapter&) = delete;
    SonicMediaAdapter& operator=(const SonicMediaAdapter&) = delete;
    int open(const char* path);
    void close();
    MediaIdentity identity() const;
    int boot_dol(GcFile& file) const;
    int find(const char* iso_path, GcFile& file) const;
    int read(const GcFile& file, uint64_t offset, void* buffer, size_t length) const;
private:
    friend class IsoMusicStream;
    void close_locked();
    int find_locked(const char* iso_path, GcFile& file) const;
    int read_locked(const GcFile& file, uint64_t offset, void* buffer, size_t length) const;
    static int read_at(void* context, uint64_t offset, void* buffer, size_t length);
    std::shared_ptr<MediaAccess> access_;
    int fd_ = -1;
    bool opened_ = false;
    GcDisc disc_{};
};
}
