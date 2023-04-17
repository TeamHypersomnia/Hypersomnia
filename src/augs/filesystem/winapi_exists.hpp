#pragma once

static std::wstring pathToWstring(const std::filesystem::path& path) {
    return path.wstring();
}

namespace augs {
	bool exists(const std::filesystem::path& filepath) {
		std::wstring wideFilepath = pathToWstring(filepath);

		WIN32_FIND_DATAW findData;
		HANDLE hFind = FindFirstFileW(wideFilepath.c_str(), &findData);

		if (hFind == INVALID_HANDLE_VALUE) {
			return false; // File not found
		}

		FindClose(hFind);

		// Compare the case-sensitive filename
		std::filesystem::path foundPath = std::filesystem::path(findData.cFileName);
		return filepath.filename().compare(foundPath) == 0;
	}
}
