#pragma once
#include <map>
#include "augs/filesystem/directory.h"

namespace augs {
	bool natural_order(const std::string& a, const std::string& b);
}

struct editor_filesystem_node {
	int file_thumbnail_id = 0;

	std::string name;

	std::vector<editor_filesystem_node> files;
	std::vector<editor_filesystem_node> subfolders;

	bool operator<(const editor_filesystem_node& b) const {
		return augs::natural_order(name, b.name);
	}

	bool is_folder() const {
		return subfolders.size() > 0 || files.size() > 0;
	}

	void sort_all() {
		sort_range(files);
		sort_range(subfolders);

		for (auto& subfolder : subfolders) {
			subfolder.sort_all();
		}
	}

	void clear() {
		files.clear();
		subfolders.clear();
	}

	void build_from(const augs::path_type& folder_path) {
		clear();

		auto add_entry = [this](const auto& path, const bool folder) {
			editor_filesystem_node new_node;
			new_node.name = path.filename().string();

			if (folder) {
				subfolders.emplace_back(std::move(new_node));
			}
			else {
				files.emplace_back(std::move(new_node));
			}

			return callback_result::CONTINUE;
		};

		augs::for_each_in_directory(
			folder_path,
			[add_entry](const auto& path) { return add_entry(path, true); },
			[add_entry](const auto& path) { return add_entry(path, false); }
		);

		for (auto& subfolder : subfolders) {
			subfolder.build_from(folder_path / subfolder.name);
		}

		sort_all();
	}
};

struct editor_filesystem {
	editor_filesystem_node root;
};

