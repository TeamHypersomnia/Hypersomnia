#include <cstddef>
#include "application/setups/client/https_file_downloader.h"
#include "augs/misc/httplib_utils.h"
#include "application/detail_file_paths.h"
#include "3rdparty/include_httplib.h"
#include "augs/log.h"

#if PLATFORM_WEB
#include "augs/misc/async_get.h"
#endif

std::string to_forward_slashes(std::string in_str);

namespace augs {
    double steady_secs();
}

https_file_downloader::https_file_downloader(const parsed_url& parent_folder_url)
    : parsed(parent_folder_url) {
#if PLATFORM_WEB
#else
    downloadThread = std::thread([this]() { 
        worker_func(); 
    });
#endif
}

#if PLATFORM_WEB
void https_file_downloader::download_next_file_from_queue() {
    if (!downloadQueue.empty() && !current_download) {
        std::string next_path = downloadQueue.front();
        downloadQueue.pop();

        const auto final_location = to_forward_slashes(parsed.location + "/" + next_path);
        LOG("HTTP downloader: requesting file at location: %x", final_location);

		downloadedBytes = 0;
		totalBytes = 0;

        current_download = augs::async_get(
            parsed.get_base_url(), 
            final_location, 
            {}, 
            [this](std::size_t loaded, std::size_t total) {
                downloadedBytes = loaded;
                totalBytes = total;

                bandwidth.newDataReceived(loaded - downloadedBytes);
                return keepRunning.load();
            }
        );

		current_download->user_data = next_path;
    }
}
#else
void https_file_downloader::worker_func() {
    auto client = httplib_utils::make_client(parsed, 3);
    client->set_keep_alive(true);

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

            auto res = client->Get(
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
                auto lock = std::scoped_lock(downloadedFilesMutex);
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
#endif

void https_file_downloader::download_file(const std::string& path) {
#if PLATFORM_WEB
	downloadQueue.push(path);
    download_next_file_from_queue();
#else
	{
		auto lock = std::scoped_lock(queueMutex);
		downloadQueue.push(path);

		totalBytes = 0;
		downloadedBytes = 0;
	}

    queueCondition.notify_one(); // notify the worker thread
#endif
}

std::optional<std::pair<std::string, std::string>> https_file_downloader::get_downloaded_file() {
#if PLATFORM_WEB
    {
        if (augs::is_ready(current_download)) {
			auto path = current_download->user_data;
			auto response = augs::get_once(current_download);
			const auto status = response.status;

			if (httplib_utils::successful(status)) {
				auto next_result = std::pair<std::string, std::string>(
					std::move(path),
					std::move(response.body)
				);

				download_next_file_from_queue();

				return next_result;
			}
			else {
				LOG("HTTP downloader: error when downloading. Status: %x", status);

				keepRunning = false;
			}
        }
    }
#else
    auto lock = std::scoped_lock(downloadedFilesMutex);

    if (!downloadedFiles.empty()) {
        auto fileData = downloadedFiles.back();
        downloadedFiles.pop_back();
        return fileData;
    }
#endif
    return std::nullopt;
}

https_file_downloader::~https_file_downloader() {
	keepRunning = false;

#if PLATFORM_WEB
#else
    queueCondition.notify_all(); // notify the worker thread
    if (downloadThread.joinable()) {
        downloadThread.join();
    }
#endif
}

bool https_file_downloader::is_running() {
    return keepRunning.load();
}

BandwidthMonitor::BandwidthMonitor() : total_bytes_(0), average_speed_(0.0) {
    last_time_ = augs::steady_secs();
}

void BandwidthMonitor::newDataReceived(std::size_t bytes) {
    double current_time = augs::steady_secs();

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
