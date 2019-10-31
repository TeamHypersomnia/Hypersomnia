#pragma once
#include <future>
#include <mutex>

#include "augs/templates/thread_templates.h"
#include "augs/window_framework/exec.h"

#define BUF_SIZE 4096

struct archive_extractor {
	std::shared_ptr<FILE> pipe;
	std::string currently_processed;
	std::array<char, BUF_SIZE> buffer;

	std::future<void> completed_extraction;
	std::mutex mtx;

	bool can_read_lines = false;

	auto lock_data() {
		return std::unique_lock<std::mutex>(mtx);
	}

	void worker() {
		while (!feof(pipe.get())) {
			if (fgets(buffer.data(), BUF_SIZE, pipe.get()) != NULL) {
				auto lk = lock_data();
				currently_processed = buffer.data();
			}
		}
	}

	auto get_currently_processed() {
		std::string result;

		{
			auto lk = lock_data();
			result = currently_processed;
		}

		return result;
	}

	bool update() {
		if (valid_and_is_ready(completed_extraction)) {
			completed_extraction.get();
			return true;
		}

		return false;
	}

	archive_extractor(
		const augs::path_type& archive_path,
		const augs::path_type& destination_path
	) {
		const auto ext = archive_path.extension().string();

		const auto command_template = [&]() {
			if (ext == ".gz") {
				auto ap = archive_path;
				ap.replace_extension("");

				if (ap.extension() == ".tar") {
					return "tar -xvzf %x -C %x";
				}
			}
			else if (ext == ".zip") {

			}

			throw std::runtime_error(typesafe_sprintf("Unknown archive format: %x", ext));
		}();

		const auto resolved_command = typesafe_sprintf(command_template, archive_path, destination_path);

		completed_extraction = std::async(
			[this, resolved_command]() {
				pipe = std::shared_ptr<FILE>(popen(resolved_command.c_str(), "r"), pclose);

				if (!pipe) {
					throw std::runtime_error("popen() failed!");
				}

				worker();
			}
		);
	}
};
