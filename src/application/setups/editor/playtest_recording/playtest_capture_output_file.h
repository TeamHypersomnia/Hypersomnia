#pragma once
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <system_error>
#include "augs/filesystem/path.h"

#if PLATFORM_LINUX
#include <fcntl.h>
#include <unistd.h>
#endif

class playtest_capture_output_file {
	static constexpr int pipe_capacity_for_encoder_lookahead_v = 1 << 20;

	std::ofstream file;

#if PLATFORM_LINUX
	int pipe_fd = -1;

	void write_to_pipe(const std::byte* data, std::size_t num_bytes) {
		while (num_bytes > 0) {
			const auto written = ::write(pipe_fd, data, num_bytes);

			if (written <= 0) {
				return;
			}

			data += written;
			num_bytes -= static_cast<std::size_t>(written);
		}
	}
#endif

public:
	explicit playtest_capture_output_file(const augs::path_type& path) {
		auto ignored = std::error_code();

#if PLATFORM_LINUX
		if (std::filesystem::is_fifo(path, ignored)) {
			pipe_fd = ::open(path.c_str(), O_RDWR);
			::fcntl(pipe_fd, F_SETPIPE_SZ, pipe_capacity_for_encoder_lookahead_v);
			return;
		}
#else
		(void)ignored;
#endif

		file.open(path, std::ios::out | std::ios::binary);
	}

	~playtest_capture_output_file() {
#if PLATFORM_LINUX
		if (pipe_fd != -1) {
			::close(pipe_fd);
		}
#endif
	}

	playtest_capture_output_file(const playtest_capture_output_file&) = delete;
	playtest_capture_output_file& operator=(const playtest_capture_output_file&) = delete;

	void write(const void* const data, const std::size_t num_bytes) {
#if PLATFORM_LINUX
		if (pipe_fd != -1) {
			write_to_pipe(reinterpret_cast<const std::byte*>(data), num_bytes);
			return;
		}
#endif

		file.write(reinterpret_cast<const char*>(data), static_cast<std::streamsize>(num_bytes));
		file.flush();
	}
};
