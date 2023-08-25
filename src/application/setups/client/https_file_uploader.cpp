#include "application/setups/client/https_file_uploader.h"
#include "augs/misc/httplib_utils.h"
#include "application/detail_file_paths.h"
#include "3rdparty/include_httplib.h"
#include "augs/readwrite/byte_file.h"
#include "augs/readwrite/json_readwrite.h"
#include "augs/log.h"

std::string to_forward_slashes(std::string in_str);

double yojimbo_time();

https_file_uploader::https_file_uploader(
	const augs::path_type& project_folder,
	const parsed_url& upload_url,
	const std::string& api_key
) : 
	project_folder(project_folder),
	parsed(upload_url),
	api_key(api_key),
	arena_name(project_folder.filename().string()),
	keepRunning(true) 
{
	uploadThread = std::thread([this]() { 
		worker_func(); 
	});
}

void https_file_uploader::worker_func() {
	const auto ca_path = CA_CERT_PATH;
	auto client = http_client_type(parsed.host);

	LOG("Uploading to host: %x, location: %x", parsed.host, parsed.location);

#if BUILD_OPENSSL
	client.set_ca_cert_path(ca_path.c_str());
	client.enable_server_certificate_verification(true);
#endif
	client.set_follow_location(true);
	client.set_read_timeout(3);
	client.set_write_timeout(3);
	client.set_keep_alive(true);

	std::vector<std::byte> file_buffer;

	while (keepRunning) {
		std::string path;
		{
			std::unique_lock<std::mutex> lock(queueMutex);
			queueCondition.wait(lock, [this]() { 
				return !uploadQueue.empty() || !keepRunning; 
			});
			if (!keepRunning && uploadQueue.empty()) {
				return;
			}
			path = uploadQueue.front();
			uploadQueue.pop();
		}

		if (!path.empty()) {
			const auto location_on_server = path;
			const auto location_on_disk = project_folder / path;

			LOG("HTTP uploader: uploading file at location: %x", location_on_disk);

			augs::file_to_bytes(location_on_disk, file_buffer);

			{
				std::lock_guard<std::mutex> lock(currently_uploaded_file_mutex);
				currently_uploaded_file = path;
				uploadedBytes = 0;
				totalBytes = file_buffer.size();
			}

			httplib::ContentProviderWithoutLength content_provider = [&](size_t offset, httplib::DataSink &sink) {
				std::size_t length = 1024;

				if (offset >= file_buffer.size()) {
					sink.done();
					return true;
				}

				if (offset + length > file_buffer.size()) {
					length = file_buffer.size() - offset;
				}

				sink.write(reinterpret_cast<const char*>(file_buffer.data()) + offset, length);
				uploadedBytes += length;
				bandwidth.newDataReceived(length);

				if (offset + length >= file_buffer.size()) {
					sink.done();
					return true;
				}

				if (!keepRunning.load()) {
					LOG("HTTP uploader: interrupting upload.");
					return false;
				}

				return true;
			};

			auto stringified_file = std::string(
				reinterpret_cast<const char*>(file_buffer.data()),
				reinterpret_cast<const char*>(file_buffer.data() + file_buffer.size())
			);

			httplib::MultipartFormDataItems items = {
				{"apikey", api_key, "", "text/plain"},
				{"arena", arena_name, "", "text/plain"},
				{"filename", location_on_server, "", "text/plain"}
			};

			httplib::MultipartFormDataProviderItems provider_items = {
				{"upload", content_provider, location_on_server, "application/octet-stream"}
			};

			(void)provider_items;
			auto res = client.Post(parsed.location, httplib::Headers(), items, provider_items);

			if (res && httplib_utils::successful(res->status)) {
				try {
					auto doc = augs::json_document_from(res->body);

					if (doc.HasMember("success") && doc["success"].IsString()) {
						/* All good */
						const auto success_message = doc["success"].GetString();

						std::lock_guard<std::mutex> lock(uploadedFilesMutex);
						uploadedFiles.emplace_back(path, std::move(success_message));

						continue;
					}
					else if (doc.HasMember("error") && doc["error"].IsString()) {
						set_error(doc["error"].GetString());
					}
					else {
						set_error("Couldn't parse: %x", res->body);
					}
				}
				catch (...) {
					set_error("Couldn't parse: %x", res->body);
				}

				keepRunning = false;
				break;
			}
			else {
				if (res) {
					set_error("HTTP uploader: error when uploading. Status: %x", res->status);
				}
				else {
					set_error("HTTP uploader: error when uploading. Response was null.");
				}

				keepRunning = false;
				break;
			}
		}
	}
}

template <class... Args>
void https_file_uploader::set_error(Args&&... args) {
	std::lock_guard<std::mutex> lock(error_mutex);
	last_error = typesafe_sprintf(std::forward<Args>(args)...);
	LOG(*last_error);
}

std::optional<std::string> https_file_uploader::get_last_error() {
	std::lock_guard<std::mutex> lock(error_mutex);
	return last_error;
}

void https_file_uploader::upload_file(const std::string& path) {
	{
		std::lock_guard<std::mutex> lock(queueMutex);
		uploadQueue.push(path);

		totalBytes = 0;
		uploadedBytes = 0;
	}

	queueCondition.notify_one(); // notify the worker thread
}

std::optional<std::pair<std::string, std::string>> https_file_uploader::get_uploaded_file() {
	std::lock_guard<std::mutex> lock(uploadedFilesMutex);
	if (!uploadedFiles.empty()) {
		uploadedBytes = 0;
		totalBytes = 0;

		auto fileData = uploadedFiles.back();
		uploadedFiles.pop_back();
		return fileData;
	}
	return std::nullopt;
}

std::string https_file_uploader::get_currently_downloaded_file() { 
	std::lock_guard<std::mutex> lock(uploadedFilesMutex);

	return currently_uploaded_file;
}

https_file_uploader::~https_file_uploader() {
	keepRunning = false;
	queueCondition.notify_all(); // notify the worker thread
	if (uploadThread.joinable()) {
		uploadThread.join();
	}
}

bool https_file_uploader::is_running() {
	return keepRunning.load();
}
