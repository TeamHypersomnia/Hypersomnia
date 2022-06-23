#pragma once
#include <map>
#include "augs/filesystem/directory.h"
#include "augs/templates/reversion_wrapper.h"
#include "view/viewables/ad_hoc_in_atlas_map.h"
#include "view/viewables/ad_hoc_atlas_subject.h"
#include "view/asset_funcs.h"
#include "augs/log.h"

namespace augs {
	bool natural_order(const std::string& a, const std::string& b);
}

struct editor_filesystem_node_ui_state {
	bool was_open = false;
};

using editor_filesystem_ui_state = 
	std::map<augs::path_type, editor_filesystem_node_ui_state>
;

struct editor_filesystem_node {
	ad_hoc_entry_id file_thumbnail_id = static_cast<ad_hoc_entry_id>(-1);
	bool is_open = false;
	bool is_resource = false;

	std::string name;

	bool as_folder = false;
	std::vector<editor_filesystem_node> files;
	std::vector<editor_filesystem_node> subfolders;

	int level = 0;
	editor_filesystem_node* parent = nullptr;

	template <class F>
	void for_each_file_recursive(F&& callback) {
		for (auto& subfolder : subfolders) {
			subfolder.for_each_file_recursive(std::forward<F>(callback));
		}

		for (auto& file : files) {
			callback(file);
		}
	}

	template <class F>
	void in_ui_order(F&& callback) {
		for (auto& subfolder : subfolders) {
			callback(subfolder);

			if (subfolder.is_open) {
				subfolder.in_ui_order(std::forward<F>(callback));
			}
		}

		for (auto& file : files) {
			callback(file);
		}
	}

	bool operator<(const editor_filesystem_node& b) const {
		return augs::natural_order(name, b.name);
	}

	bool is_folder() const {
		return as_folder;
	}

	void toggle_open() {
		is_open = !is_open;
	}

	void sort_all() {
		sort_range(files);
		sort_range(subfolders);

		for (auto& subfolder : subfolders) {
			subfolder.sort_all();
		}
	}

	void set_parents(int current_level) {
		for (auto& f : files) {
			f.parent = this;
			f.level = current_level;
		}

		for (auto& f : subfolders) {
			f.parent = this;
			f.level = current_level;
			f.set_parents(current_level + 1);
		}
	}

	void clear() {
		parent = nullptr;
		files.clear();
		subfolders.clear();
	}

	auto get_ui_state() const {
		return editor_filesystem_node_ui_state { is_open };
	}

	void apply_ui_state(const editor_filesystem_node_ui_state& state) {
		is_open = state.was_open;
	}

	auto get_path() const {
		std::vector<std::string> parents;
		parents.reserve(level);

		auto it = parent;

		while (it) {
			parents.push_back(it->name);
			it = it->parent;
		}

		auto total_path = augs::path_type();

		for (auto& parent : reverse(parents)) {
			if (total_path.empty()) {
				total_path = parent;
			}
			else {
				total_path /= parent;
			}
		}

		total_path /= name;

		return total_path;
	}

	void build_from(const augs::path_type& folder_path) {
		clear();

		auto add_entry = [this](const auto& path, const bool folder) {
			editor_filesystem_node new_node;
			new_node.name = path.filename().string();
			const auto extension = path.extension().string();

			if (folder) {
				new_node.as_folder = true;
				subfolders.emplace_back(std::move(new_node));
			}
			else {
				new_node.is_resource = assets::is_asset_extension(extension);
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
		set_parents(0);
	}
};

struct editor_filesystem {
	editor_filesystem_node root;

	auto make_ui_state() {
		auto node_to_state = editor_filesystem_ui_state();

		root.in_ui_order([&](const auto& node) {
			node_to_state[node.get_path()] = node.get_ui_state();
		});

		return node_to_state;
	}

	void apply_ui_state(const editor_filesystem_ui_state& node_to_state) {
		root.in_ui_order([&](auto& node) {
			if (auto state = mapped_or_nullptr(node_to_state, node.get_path())) {
				node.apply_ui_state(*state);
			}
		});
	}

	void rebuild_from(const augs::path_type& folder_path) {
		auto saved_state = make_ui_state();

		root.build_from(folder_path);

		apply_ui_state(saved_state);
	}

	void fill_thumbnail_entries(const augs::path_type& project_folder, ad_hoc_atlas_subjects& out_subjects) {
		ad_hoc_entry_id id_counter = 1;

		root.for_each_file_recursive([&](auto& node) {
			const auto path = project_folder / node.get_path();

			if (assets::is_supported_extension<assets::image_id>(path.extension().string())) {
				node.file_thumbnail_id = id_counter++;
				out_subjects.push_back({ node.file_thumbnail_id, path });
			}
		});
	}

};

