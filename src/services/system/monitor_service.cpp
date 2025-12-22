#include "services/system/monitor_service.hpp"

#include <fcntl.h>
#include <poll.h>
#include <sys/eventfd.h>
#include <sys/fanotify.h>
#include <unistd.h>

#include <climits>
#include <iostream>

#include "services/core/detector_service.hpp"
#include "services/system/config_service.hpp"

namespace warden::services {

MonitorService::MonitorService(DetectorService& ds, ConfigService& cs, IReportObserver& observer)
    : detector_(ds), config_(cs), observer_(observer) {
    fan_fd_ = fanotify_init(FAN_CLASS_NOTIF, O_RDONLY | O_LARGEFILE);
    if (fan_fd_ < 0) {
        if (errno == EPERM) throw std::runtime_error("Monitor requires sudo (CAP_SYS_ADMIN)");
        throw std::runtime_error("Failed to init fanotify");
    }

    stop_fd_ = eventfd(0, EFD_NONBLOCK);
}

MonitorService::~MonitorService() {
    stop();
    if (fan_fd_ >= 0) close(fan_fd_);
    if (stop_fd_ >= 0) close(stop_fd_);
}

void MonitorService::start(const std::vector<std::string>& paths) {
    if (is_running_.exchange(true)) return;

    for (const auto& path : paths) {
        uint64_t mask = FAN_MODIFY | FAN_CLOSE_WRITE | FAN_OPEN | FAN_EVENT_ON_CHILD;

        if (fanotify_mark(fan_fd_, FAN_MARK_ADD, mask, AT_FDCWD, path.c_str()) < 0) {
            perror("[Monitor] fanotify_mark failed");
        }
    }

    worker_thread_ = std::thread(&MonitorService::run, this);
}

void MonitorService::run() {
    struct pollfd fds[2];
    fds[0].fd = fan_fd_;
    fds[0].events = POLLIN;
    fds[1].fd = stop_fd_;
    fds[1].events = POLLIN;

    char buffer[4096];
    pid_t my_pid = getpid();

    while (is_running_) {
        int ret = poll(fds, 2, -1);
        if (ret <= 0) continue;

        if (fds[1].revents & POLLIN) break;

        ssize_t len = read(fan_fd_, buffer, sizeof(buffer));
        if (len <= 0) continue;

        const struct fanotify_event_metadata* metadata =
            reinterpret_cast<const struct fanotify_event_metadata*>(buffer);

        while (FAN_EVENT_OK(metadata, len)) {
            if (metadata->pid == my_pid) {
                if (metadata->fd >= 0) close(metadata->fd);
                metadata = FAN_EVENT_NEXT(metadata, len);
                continue;
            }

            if (metadata->fd >= 0) {
                char path[PATH_MAX];
                std::string proc_path = "/proc/self/fd/" + std::to_string(metadata->fd);
                ssize_t path_len = readlink(proc_path.c_str(), path, sizeof(path) - 1);

                if (path_len > 0) {
                    path[path_len] = '\0';

                    auto result = detector_.process_file(path, config_.get_threshold());
                    result.event_mask = metadata->mask;

                    observer_.notify_detection(path, result);
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
    if (write(stop_fd_, &wakeup, sizeof(wakeup)) == -1) {
    }

    if (worker_thread_.joinable()) {
        worker_thread_.join();
    }
}

}  // namespace warden::services