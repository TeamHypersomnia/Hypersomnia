#pragma once
#include <future>
#include <mutex>

#include "augs/templates/thread_templates.h"
#include "augs/misc/timing/timer.h"
#include "augs/window_framework/pipe.h"

struct archive_extractor {
	std::string currently_processed;
	int read_percent;
	int percent_complete = 0;

	struct info {
		std::string processed = "";
		int percent = 0;
	};

	info current;

	std::future<void> completed_extraction;
	std::mutex mtx;

	auto lock_info() {
		return std::unique_lock<std::mutex>(mtx);
	}

	std::atomic<bool>& exit_requested;

	void worker(augs::pipe& pipe) {
		std::string line;
		bool read_till_backspace = false;

		while (const auto maybe_new_char = pipe.get_next_character()) {
			if (exit_requested.load()) {
				LOG("Extraction worker: requested cancellation.");
				return;
			}

			const auto new_char = *maybe_new_char;

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

	auto get_info() {
		info result;

		{
			auto lk = lock_info();
			result = current;
		}

		return result;
	}

	bool has_completed() {
		if (valid_and_is_ready(completed_extraction)) {
			completed_extraction.get();
			return true;
		}

		return false;
	}

	archive_extractor(
		const augs::path_type& archive_path,
		const augs::path_type& destination_path,
		std::atomic<bool>& exit_requested
	) : exit_requested(exit_requested) {
		const auto ext = archive_path.extension().string();

		const auto resolved_command = [&]() {
			const auto& source = archive_path;
			const auto& target = destination_path;

			return typesafe_sprintf("\"%x\" -o\"%x\"", source, target);
		}();

		completed_extraction = launch_async(
			[this, resolved_command]() {
				auto pipe = augs::pipe(resolved_command);
				worker(pipe);
			}
		);
	}
};
