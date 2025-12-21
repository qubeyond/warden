#include "services/system/monitor_service.hpp"

#include <sys/inotify.h>
#include <unistd.h>

#include <iostream>

#include "services/core/detector_service.hpp"
#include "services/system/config_service.hpp"

namespace warden::services {

MonitorService::MonitorService(DetectorService& ds, ConfigService& cs, IReportObserver& observer)
    : detector_(ds), config_(cs), observer_(observer) {
    inotify_fd_ = inotify_init();
    if (inotify_fd_ < 0) throw std::runtime_error("Failed to initialize inotify");
}

MonitorService::~MonitorService() {
    stop();
    if (inotify_fd_ >= 0) close(inotify_fd_);
}

void MonitorService::start(const std::vector<std::string>& paths) {
    for (const auto& path : paths) {
        int wd =
            inotify_add_watch(inotify_fd_, path.c_str(), IN_CREATE | IN_CLOSE_WRITE | IN_MOVED_TO);
        if (wd != -1) watch_descriptors_[wd] = path;
    }
    is_running_ = true;
    process_events();
}

void MonitorService::process_events() {
    char buffer[4096] __attribute__((aligned(__alignof__(struct inotify_event))));
    while (is_running_) {
        ssize_t len = read(inotify_fd_, buffer, sizeof(buffer));
        if (len < 0) break;
        char* ptr = buffer;
        while (ptr < buffer + len) {
            auto* event = reinterpret_cast<struct inotify_event*>(ptr);
            if (!(event->mask & IN_ISDIR) && event->len > 0) {
                std::string full_path = watch_descriptors_[event->wd] + "/" + event->name;
                auto result = detector_.process_file(full_path, config_.get_threshold());
                observer_.notify_detection(full_path, result);
            }
            ptr += sizeof(struct inotify_event) + event->len;
        }
    }
}

void MonitorService::stop() {
    is_running_ = false;
}
}  // namespace warden::services