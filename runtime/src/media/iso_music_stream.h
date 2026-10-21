#pragma once
#include "sonic_media_adapter.h"

namespace battleheart {
// Compressed input only. Close, any reopen attempt and adapter destruction
// invalidate the binding. Reads never extend the adapter's lifetime.
class IsoMusicStream final {
public:
    explicit IsoMusicStream(const SonicMediaAdapter& media) : access_(media.access_) {}
    int open(const char* verified_iso_path) {
        std::lock_guard<std::mutex> stream_lock(mutex_);
        ready_ = false;
        file_ = {};
        const auto access = access_.lock();
        if (!access) return GC_INVALID;
        std::lock_guard<std::mutex> source_lock(access->mutex);
        if (!access->owner) return GC_INVALID;
        const int result = access->owner->find_locked(verified_iso_path, file_);
        ready_ = result == GC_OK && file_.size > 0;
        generation_ = access->generation;
        return ready_ ? GC_OK : (result == GC_OK ? GC_INVALID : result);
    }
    int read_at(uint64_t offset, void* buffer, size_t length) const {
        std::lock_guard<std::mutex> stream_lock(mutex_);
        const auto access = access_.lock();
        if (!access) return GC_INVALID;
        std::lock_guard<std::mutex> source_lock(access->mutex);
        if (!ready_ || !access->owner || access->generation != generation_) return GC_INVALID;
        return access->owner->read_locked(file_, offset, buffer, length);
    }
    uint64_t size() const {
        std::lock_guard<std::mutex> stream_lock(mutex_);
        const auto access = access_.lock();
        if (!access) return 0;
        std::lock_guard<std::mutex> source_lock(access->mutex);
        return ready_ && access->owner && access->generation == generation_ ? file_.size : 0;
    }
private:
    std::weak_ptr<MediaAccess> access_;
    mutable std::mutex mutex_;
    uint64_t generation_ = 0;
    GcFile file_{};
    bool ready_ = false;
};
}
