#include "services/system/monitor_service.hpp"

#include <fcntl.h>
#include <poll.h>
#include <sys/eventfd.h>
#include <sys/fanotify.h>
#include <sys/stat.h>
#include <unistd.h>

#include <climits>
#include <stdexcept>
#include <string>

#include "common/config.hpp"
#include "services/core/detector_service.hpp"

namespace warden::services {

MonitorService::MonitorService(DetectorService& ds, const ConfigManager& cs,
                               IReportObserver& observer)
    : detector_(ds), config_(cs), observer_(observer) {
    fan_fd_ = fanotify_init(FAN_CLASS_NOTIF | FAN_CLOEXEC, O_RDONLY | O_LARGEFILE);
    if (fan_fd_ < 0) throw std::runtime_error("fanotify_init failed");
    stop_fd_ = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
}

MonitorService::~MonitorService() {
    stop();
    if (fan_fd_ >= 0) close(fan_fd_);
    if (stop_fd_ >= 0) close(stop_fd_);
}

void MonitorService::start(const std::vector<std::string>& paths) {
    if (is_running_.exchange(true)) return;
    for (const auto& path : paths) {
        fanotify_mark(fan_fd_, FAN_MARK_ADD | FAN_MARK_MOUNT, FAN_CLOSE_WRITE | FAN_EVENT_ON_CHILD,
                      AT_FDCWD, path.c_str());
    }
    worker_thread_ = std::thread(&MonitorService::run, this);
}

void MonitorService::run() {
    struct pollfd fds[2] = {{fan_fd_, POLLIN, 0}, {stop_fd_, POLLIN, 0}};
    alignas(struct fanotify_event_metadata) char buffer[8192];
    const pid_t my_pid = getpid();

    while (is_running_) {
        if (poll(fds, 2, -1) <= 0 || (fds[1].revents & POLLIN)) break;
        ssize_t len = read(fan_fd_, buffer, sizeof(buffer));
        if (len <= 0) continue;

        const auto* metadata = reinterpret_cast<const struct fanotify_event_metadata*>(buffer);
        while (FAN_EVENT_OK(metadata, len)) {
            if (metadata->fd >= 0) {
                if (metadata->pid != my_pid) {
                    struct stat st;
                    if (fstat(metadata->fd, &st) == 0 && S_ISREG(st.st_mode)) {
                        char path[PATH_MAX];
                        std::string proc_p = "/proc/self/fd/" + std::to_string(metadata->fd);
                        ssize_t p_len = readlink(proc_p.c_str(), path, sizeof(path) - 1);
                        if (p_len > 0) {
                            path[p_len] = '\0';
                            auto res = detector_.process_file(path, config_.model().threshold);
                            res.event_mask = metadata->mask;
                            observer_.notify_detection(path, res);
                        }
                    }
                }
                close(metadata->fd);
            }
            metadata = FAN_EVENT_NEXT(metadata, len);
        }
    }
}

void MonitorService::stop() {
    if (!is_running_.exchange(false)) return;
    uint64_t wakeup = 1;
    ssize_t n = write(stop_fd_, &wakeup, sizeof(wakeup));
    (void)n;
    if (worker_thread_.joinable()) worker_thread_.join();
}

}  // namespace warden::services