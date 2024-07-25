#pragma once
#include <thread>
#include <vector>
#include <string>
#include <atomic>
#include <queue>
#include <mutex>
#include <optional>
#include <condition_variable>
#include <memory>

#include "augs/string/parse_url.h"
#include "application/setups/client/bandwidth_monitor.h"

#if PLATFORM_WEB
#include "augs/misc/async_response.h"
#endif

class https_file_downloader {
    parsed_url parsed;

	std::atomic_size_t totalBytes = 0;
	std::atomic_size_t downloadedBytes = 0;

	std::atomic_bool keepRunning = true;

	BandwidthMonitor bandwidth;

    std::queue<std::string> downloadQueue;

#if PLATFORM_WEB
    std::shared_ptr<augs::async_response> current_download;
    void download_next_file_from_queue();
#else
    void worker_func();

    std::thread downloadThread;
    std::mutex queueMutex;
    std::mutex downloadedFilesMutex;
    std::condition_variable queueCondition;

	std::vector<std::pair<std::string, std::string>> downloadedFiles;
#endif

public:
    void download_file(const std::string& path);
    std::optional<std::pair<std::string, std::string>> get_downloaded_file();

    bool is_running();

    std::size_t get_current_total_bytes() const {
        return totalBytes;
    }

    std::size_t get_current_downloaded_bytes() const {
        return downloadedBytes;
    }

    double get_bandwidth() const {
        return bandwidth.getAverageSpeed();
    }

    https_file_downloader(const parsed_url& parent_folder_url);
    ~https_file_downloader();
};
