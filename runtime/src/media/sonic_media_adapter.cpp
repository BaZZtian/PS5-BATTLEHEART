#define _FILE_OFFSET_BITS 64
#include "sonic_media_adapter.h"
#include "sonic_gc_media.h"
#include <cerrno>
#include <cstdint>
#include <cstring>
#include <fcntl.h>
#include <limits>
#include <sys/stat.h>
#include <unistd.h>

namespace battleheart {
SonicMediaAdapter::SonicMediaAdapter() : access_(std::make_shared<MediaAccess>()) {
    access_->owner = this;
}
SonicMediaAdapter::~SonicMediaAdapter() {
    std::lock_guard<std::mutex> lock(access_->mutex);
    close_locked();
    access_->owner = nullptr;
}
void SonicMediaAdapter::close() {
    std::lock_guard<std::mutex> lock(access_->mutex);
    close_locked();
}
void SonicMediaAdapter::close_locked() {
    ++access_->generation;
    if (opened_) gc_disc_close(&disc_);
    opened_ = false;
    disc_ = {};
    if (fd_ >= 0) ::close(fd_);
    fd_ = -1;
}
int SonicMediaAdapter::read_at(void* context, uint64_t offset, void* buffer, size_t length) {
    auto& self = *static_cast<SonicMediaAdapter*>(context);
    const auto maximum = static_cast<uint64_t>(std::numeric_limits<off_t>::max());
    if (offset > maximum || length > maximum - offset) return -1;
    auto* destination = static_cast<unsigned char*>(buffer);
    size_t done = 0;
    while (done < length) {
        const size_t chunk = (length - done > 1024U * 1024U) ? 1024U * 1024U : length - done;
        const ssize_t count = pread(self.fd_, destination + done, chunk,
                                    static_cast<off_t>(offset + done));
        if (count < 0 && errno == EINTR) continue;
        if (count <= 0) return -1;
        done += static_cast<size_t>(count);
    }
    return 0;
}
int SonicMediaAdapter::open(const char* path) {
    std::lock_guard<std::mutex> lock(access_->mutex);
    close_locked();
    if (!path || !*path) return GC_INVALID;
    // Do not follow a substituted final symlink or block on a FIFO before fstat.
    fd_ = ::open(path, O_RDONLY | O_NOFOLLOW | O_NONBLOCK);
    if (fd_ < 0) return GC_IO_ERROR;
    struct stat info{};
    if (fstat(fd_, &info) != 0 || !S_ISREG(info.st_mode) || info.st_size < 0) {
        close_locked(); return GC_IO_ERROR;
    }
    const GcReader reader{this, static_cast<uint64_t>(info.st_size), read_at};
    const int result = sonic_gc_open_media(&disc_, &reader);
    if (result != GC_OK) { close_locked(); return result; }
    opened_ = true;
    if (std::strcmp(disc_.game_id, "GSNE8P") != 0 &&
        std::strcmp(disc_.game_id, "GSNP8P") != 0) {
        close_locked(); return GC_WRONG_GAME;
    }
    return GC_OK;
}
int SonicMediaAdapter::boot_dol(GcFile& file) const {
    std::lock_guard<std::mutex> lock(access_->mutex);
    file = {};
    return opened_ ? gc_disc_boot_dol(&disc_, &file) : GC_INVALID;
}
int SonicMediaAdapter::find(const char* iso_path, GcFile& file) const {
    std::lock_guard<std::mutex> lock(access_->mutex);
    return find_locked(iso_path, file);
}
int SonicMediaAdapter::find_locked(const char* iso_path, GcFile& file) const {
    file = {};
    if (!opened_ || !iso_path || !*iso_path) return GC_INVALID;
    return gc_disc_find(&disc_, iso_path, &file);
}
int SonicMediaAdapter::read(const GcFile& file, uint64_t offset, void* buffer, size_t length) const {
    std::lock_guard<std::mutex> lock(access_->mutex);
    return read_locked(file, offset, buffer, length);
}
int SonicMediaAdapter::read_locked(const GcFile& file, uint64_t offset, void* buffer, size_t length) const {
    return opened_ ? gc_disc_read(&disc_, &file, offset, buffer, length) : GC_INVALID;
}
MediaIdentity SonicMediaAdapter::identity() const {
    std::lock_guard<std::mutex> lock(access_->mutex);
    MediaIdentity result{};
    if (opened_) {
        std::memcpy(result.game_id, disc_.game_id, sizeof(result.game_id));
        result.revision = disc_.revision;
    }
    return result;
}
}
