#pragma once
#include <thread>
#include <vector>
#include <string>
#include <mutex>
#include <atomic>
#include <queue>
#include <optional>
#include <condition_variable>

#include "augs/string/parse_url.h"

class BandwidthMonitor {
public:
	BandwidthMonitor();
	void newDataReceived(std::size_t bytes);
	double getAverageSpeed() const;

private:
	std::deque<std::pair<std::size_t, double>> data_;
	std::size_t total_bytes_;
	double last_time_;
	std::atomic<double> average_speed_;
};

class https_file_downloader {
	parsed_url parsed;

    std::queue<std::string> downloadQueue;
    std::mutex queueMutex;
    std::condition_variable queueCondition;
    std::atomic_bool keepRunning;
    std::vector<std::pair<std::string, std::string>> downloadedFiles;
    std::mutex downloadedFilesMutex;

	std::atomic_size_t totalBytes = 0;
	std::atomic_size_t downloadedBytes = 0;

	BandwidthMonitor bandwidth;

	std::thread downloadThread;

	void worker_func();

public:

	void download_file(const std::string& path);
	std::optional<std::pair<std::string, std::string>> get_downloaded_file();

	bool is_running();

	std::size_t get_total_bytes() const {
		return totalBytes;
	}

	std::size_t get_downloaded_bytes() const {
		return downloadedBytes;
	}

	double get_bandwidth() const {
		return bandwidth.getAverageSpeed();
	}

	https_file_downloader(const parsed_url& parent_folder_url);
	~https_file_downloader();
};
