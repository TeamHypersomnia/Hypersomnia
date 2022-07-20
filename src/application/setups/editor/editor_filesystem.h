#pragma once
#include <map>
#include "augs/filesystem/directory.h"
#include "augs/templates/reversion_wrapper.h"
#include "view/viewables/ad_hoc_in_atlas_map.h"
#include "view/viewables/ad_hoc_atlas_subject.h"
#include "view/asset_funcs.h"
#include "augs/log.h"
#include "augs/filesystem/file.h"

#include "application/setups/editor/resources/editor_resource_id.h"

namespace augs {
	bool natural_order(const std::string& a, const std::string& b);
}

struct editor_filesystem_node_ui_state {
	bool was_open = false;
};

using editor_filesystem_ui_state = 
	std::map<augs::path_type, editor_filesystem_node_ui_state>
;

enum class editor_filesystem_node_type : uint8_t {
	OTHER_FILE,
	FOLDER,

	IMAGE,
	SOUND,

	OTHER_RESOURCE
};

struct editor_filesystem_node {
	ad_hoc_entry_id file_thumbnail_id = static_cast<ad_hoc_entry_id>(-1);
	std::optional<augs::atlas_entry> game_atlas_icon;

	editor_resource_id associated_resource;

	editor_filesystem_node_type type = editor_filesystem_node_type::OTHER_FILE;

	bool is_open = false;
	bool passed_filter = false;

	std::string name;

	std::vector<editor_filesystem_node> files;
	std::vector<editor_filesystem_node> subfolders;

	int level = 0;
	augs::file_time_type last_write_time; 

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
	void for_each_parent(F&& callback) {
		auto it = parent;

		while (it) {
			callback(*it);
			it = it->parent;
		}
	}

	template <class F>
	void for_each_parent(F&& callback) const {
		auto it = parent;

		while (it) {
			callback(*it);
			it = it->parent;
		}
	}

	template <class F>
	void for_each_entry_recursive(F&& callback) {
		in_ui_order(std::forward<F>(callback), true);
	}

	void reset_filter_flags() {
		for_each_entry_recursive([](auto& entry) { entry.passed_filter = false; });
	}

	void mark_passed_filter() {
		passed_filter = true;
		for_each_parent([](auto& parent){ parent.passed_filter = true; });
	}

	template <class F>
	void in_ui_order(F&& callback, bool with_closed_folders = false) {
		for (auto& subfolder : subfolders) {
			callback(subfolder);

			if (with_closed_folders || subfolder.is_open) {
				subfolder.in_ui_order(std::forward<F>(callback), with_closed_folders);
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
		return type == editor_filesystem_node_type::FOLDER;
	}

	bool is_image() const {
		return type == editor_filesystem_node_type::IMAGE;
	}

	bool is_sound() const {
		return type == editor_filesystem_node_type::SOUND;
	}

	bool is_resource() const {
		return 
			type == editor_filesystem_node_type::IMAGE
			|| type == editor_filesystem_node_type::SOUND
			|| type == editor_filesystem_node_type::OTHER_RESOURCE
		;
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

	bool is_root() const {
		return parent == nullptr;
	}

	bool is_child_of_root() const {
		return parent && parent->is_root();
	}

	auto get_ui_state() const {
		return editor_filesystem_node_ui_state { is_open };
	}

	void apply_ui_state(const editor_filesystem_node_ui_state& state) {
		is_open = state.was_open;
	}

	auto get_path_in_project() const {
		std::vector<std::string> parents;
		parents.reserve(level);

		for_each_parent([&parents](auto& parent) { parents.push_back(parent.name); });

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
				new_node.type = editor_filesystem_node_type::FOLDER;
				subfolders.emplace_back(std::move(new_node));
			}
			else {
				new_node.type = editor_filesystem_node_type::OTHER_FILE;

				if (assets::is_image_extension(extension)) {
					new_node.type = editor_filesystem_node_type::IMAGE;
				}

				if (assets::is_sound_extension(extension)) {
					new_node.type = editor_filesystem_node_type::SOUND;
				}

				try {
					new_node.last_write_time = augs::last_write_time(path);
					files.emplace_back(std::move(new_node));
				}
				catch (...) {

				}
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
			node_to_state[node.get_path_in_project()] = node.get_ui_state();
		});

		return node_to_state;
	}

	void apply_ui_state(const editor_filesystem_ui_state& node_to_state) {
		root.in_ui_order([&](auto& node) {
			if (auto state = mapped_or_nullptr(node_to_state, node.get_path_in_project())) {
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
			const auto path = project_folder / node.get_path_in_project();

			if (assets::is_supported_extension<assets::image_id>(path.extension().string())) {
				node.file_thumbnail_id = id_counter++;
				out_subjects.push_back({ node.file_thumbnail_id, path });
			}
		});
	}

};

