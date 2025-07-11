#pragma once
#include <cstddef>
#include <thread>
#include <vector>
#include <string>
#include <mutex>
#include <atomic>
#include <queue>
#include <optional>
#include <condition_variable>

#include "augs/string/parse_url.h"

#include "application/setups/client/bandwidth_monitor.h"
#include "augs/filesystem/path.h"

class https_file_uploader {
	augs::path_type project_folder;
	parsed_url parsed;
	std::string api_key;

	std::string arena_name;

    std::queue<std::string> uploadQueue;
    std::mutex queueMutex;
    std::condition_variable queueCondition;
    std::atomic_bool keepRunning;
    std::vector<std::pair<std::string, std::string>> uploadedFiles;
    std::mutex uploadedFilesMutex;

	std::mutex currently_uploaded_file_mutex;
	std::mutex error_mutex;
	std::string currently_uploaded_file;

	std::atomic_size_t totalBytes = 0;
	std::atomic_size_t uploadedBytes = 0;

	BandwidthMonitor bandwidth;

	std::thread uploadThread;
	std::optional<std::string> last_error;

	template <class... Args>
	void set_error(Args&&...);

	void worker_func();

public:

	void upload_file(const std::string& path);
	std::optional<std::pair<std::string, std::string>> get_uploaded_file();

	bool is_running();

	std::size_t get_total_bytes() const {
		return totalBytes;
	}

	std::size_t get_uploaded_bytes() const {
		return uploadedBytes;
	}

	double get_bandwidth() const {
		return bandwidth.getAverageSpeed();
	}

	std::string get_currently_downloaded_file();

	https_file_uploader(
		const augs::path_type& project_folder,
		const parsed_url& upload_url,
		const std::string& api_key
	);

	~https_file_uploader();

	std::optional<std::string> get_last_error();
};
