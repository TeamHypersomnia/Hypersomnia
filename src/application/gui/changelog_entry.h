#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "augs/filesystem/path.h"
#include "augs/string/simple_markdown.h"

/*
	CHANGELOG.md is read at runtime from the game folder,
	so editing it and reopening the changelog from Settings shows the changes immediately.
	The repository root has a symlink to it.
*/

#define CHANGELOG_PATH "CHANGELOG.md"

/*
	Versions older than this are not shown in-game - their changelogs were written before it existed.
*/

#define CHANGELOG_SINCE_VERSION "3.0.0"

struct changelog_entry {
	std::string version;
	std::string date;
	int64_t timestamp = 0;
	std::vector<augs::markdown_line> lines;
};

using changelog_entries = std::vector<changelog_entry>;

/*
	One entry per "## [X.Y.Z] - YYYY-MM-DD", in file order (newest first).
	"## [Unreleased]" and anything above the first version is skipped.
*/

changelog_entries parse_changelog(const std::string& source);

/*
	Skips versions older than CHANGELOG_SINCE_VERSION.
	Empty if the file can't be read.
*/

changelog_entries load_changelog(const augs::path_type& path);
