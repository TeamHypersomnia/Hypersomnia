#include "application/setups/client/https_file_downloader.h"
#include "augs/misc/httplib_utils.h"
#include "application/detail_file_paths.h"
#include "3rdparty/include_httplib.h"
#include "augs/log.h"

std::string to_forward_slashes(std::string in_str);

double yojimbo_time();

https_file_downloader::https_file_downloader(
	const parsed_url& parent_folder_url
) : parsed(parent_folder_url), keepRunning(true) {

	downloadThread = std::thread([this]() { 
		worker_func(); 
	});
}

void https_file_downloader::worker_func() {
	const auto ca_path = CA_CERT_PATH;
	auto client = http_client_type(parsed.host);

#if BUILD_OPENSSL
	client.set_ca_cert_path(ca_path.c_str());
	client.enable_server_certificate_verification(true);
#endif
	client.set_follow_location(true);
	client.set_read_timeout(3);
	client.set_write_timeout(3);
	client.set_keep_alive(true);

	while (keepRunning) {
		std::string path;
		{
			std::unique_lock<std::mutex> lock(queueMutex);
			queueCondition.wait(lock, [this]() { 
				return !downloadQueue.empty() || !keepRunning; 
			});
			if (!keepRunning && downloadQueue.empty()) {
				return;
			}
			path = downloadQueue.front();
			downloadQueue.pop();
		}

		if (!path.empty()) {
			const auto final_location = to_forward_slashes(parsed.location + "/" + path);
			LOG("HTTP downloader: requesting file at location: %x", final_location);

			downloadedBytes = 0;

			auto res = client.Get(
				final_location.c_str(), 
				[this](std::size_t data_length, std::size_t total_length) {
					const auto dt = data_length - downloadedBytes;
					downloadedBytes = data_length;
					totalBytes = total_length;

					if (!keepRunning.load()) {
						LOG("HTTP downloader: interrupting download.");
						return false;
					}

					bandwidth.newDataReceived(dt);

					return true;
				}
			);

			if (res && httplib_utils::successful(res->status)) {
				std::lock_guard<std::mutex> lock(downloadedFilesMutex);
				downloadedFiles.emplace_back(path, std::move(res->body));
			}
			else {
				if (res) {
					LOG("HTTP downloader: error when downloading. Status: %x", res->status);
				}
				else {
					LOG("HTTP downloader: error when downloading. Response was null.");
				}

				keepRunning = false;
				break;
			}
		}
	}
}

void https_file_downloader::download_file(const std::string& path) {
	{
		std::lock_guard<std::mutex> lock(queueMutex);
		downloadQueue.push(path);

		totalBytes = 0;
		downloadedBytes = 0;
	}

	queueCondition.notify_one(); // notify the worker thread
}

std::optional<std::pair<std::string, std::string>> https_file_downloader::get_downloaded_file() {
	std::lock_guard<std::mutex> lock(downloadedFilesMutex);
	if (!downloadedFiles.empty()) {
		auto fileData = downloadedFiles.back();
		downloadedFiles.pop_back();
		return fileData;
	}
	return std::nullopt;
}

https_file_downloader::~https_file_downloader() {
	keepRunning = false;
	queueCondition.notify_all(); // notify the worker thread
	if (downloadThread.joinable()) {
		downloadThread.join();
	}
}

bool https_file_downloader::is_running() {
	return keepRunning.load();
}

BandwidthMonitor::BandwidthMonitor() : total_bytes_(0), average_speed_(0.0) {
    last_time_ = yojimbo_time();
}

void BandwidthMonitor::newDataReceived(std::size_t bytes) {
    double current_time = yojimbo_time();

    // Add the new data
    data_.push_back({bytes, current_time});
    total_bytes_ += bytes;

    // Remove old data (> 1 second old)
    while (current_time - data_.front().second > 1.0) {
        total_bytes_ -= data_.front().first;
        data_.pop_front();
    }

    // Update average speed
    average_speed_ = total_bytes_;

    last_time_ = current_time;
}

double BandwidthMonitor::getAverageSpeed() const {
    return average_speed_;
}
