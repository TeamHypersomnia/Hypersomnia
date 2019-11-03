#pragma once
#include <future>
#include <mutex>

#include "augs/templates/thread_templates.h"
#include "augs/window_framework/exec.h"

#define BUF_SIZE 4096

struct archive_extractor {
	std::shared_ptr<FILE> pipe;
	std::string currently_processed;
	int read_percent;
	int percent_complete = 0;

	struct info {
		std::string processed = "";
		int percent = 0;
	};

	info current;

	std::array<char, BUF_SIZE> buffer;

	std::future<void> completed_extraction;
	std::mutex mtx;

	bool can_read_lines = false;

	auto lock_info() {
		return std::unique_lock<std::mutex>(mtx);
	}

	void worker() {
		std::string line;
		bool read_till_backspace = false;

		while (!std::feof(pipe.get())) {
			if (const auto result = std::fgetc(pipe.get()); result != EOF) {
				const auto new_char = static_cast<char>(result);

				if (new_char == '%') {
					if (const auto number = get_trailing_number(line)) {
						line = std::to_string(*number) + "% ";
						read_till_backspace = true;
					}
				}
				else {
					constexpr char BACKSPACE_v = 8;
					constexpr char CARRIAGE_RETURN_v = 13;

					if (new_char == BACKSPACE_v || new_char == CARRIAGE_RETURN_v) {
						if (read_till_backspace) {
							int num_files = -1;
							int percent = -1;
							std::string processed;

							LOG("Processing: %x", line);

							if (typesafe_sscanf(line, "%x%  %x - %x", percent, num_files, processed)) {
								if (processed.size() > 0) {
									auto lk = lock_info();
									current.processed = processed;
									current.percent = percent;
								}
								else {
									auto lk = lock_info();
									current.percent = percent;
								}
							}
							LOG_NVPS(percent, num_files, processed);

							read_till_backspace = false;
						}

						line.clear();
					}
					else {
						line += new_char;
					}
				}
			}
		}
	}

	auto get_info() {
		info result;

		{
			auto lk = lock_info();
			result = current;
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

		const auto resolved_command = [&]() {
			const auto& source = archive_path;
			const auto& target = destination_path;

			return typesafe_sprintf("%x -o%x", source, target);
		}();

		completed_extraction = std::async(
			std::launch::async,
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
