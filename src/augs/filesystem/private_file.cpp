#include <cstdio>
#include <string>
#if PLATFORM_UNIX
#include <fcntl.h>
#include <unistd.h>
#elif defined(_WIN32)
#include <io.h>
#endif
#include "augs/filesystem/path.h"
#include "augs/filesystem/private_file.h"

namespace augs {
	bool write_new_private_file(const path_type& path, const std::string& content) {
#if PLATFORM_UNIX
		const auto fd = ::open(path.string().c_str(), O_WRONLY | O_CREAT | O_EXCL, 0600);

		if (fd < 0) {
			return false;
		}

		const auto written = ::write(fd, content.data(), content.size());
		const bool synced = ::fsync(fd) == 0;
		::close(fd);

		return synced && written == static_cast<decltype(written)>(content.size());
#elif defined(_WIN32)
		std::FILE* const file = ::_wfopen(path.c_str(), L"wbx");

		if (file == nullptr) {
			return false;
		}

		const auto written = std::fwrite(content.data(), 1, content.size(), file);
		const bool synced = std::fflush(file) == 0 && ::_commit(::_fileno(file)) == 0;
		std::fclose(file);

		return synced && written == content.size();
#else
		std::FILE* const file = std::fopen(path.string().c_str(), "wbx");

		if (file == nullptr) {
			return false;
		}

		const auto written = std::fwrite(content.data(), 1, content.size(), file);
		const bool flushed = std::fflush(file) == 0;
		std::fclose(file);

		return flushed && written == content.size();
#endif
	}
}
