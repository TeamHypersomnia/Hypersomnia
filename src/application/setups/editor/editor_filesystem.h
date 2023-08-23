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
#include "application/setups/editor/editor_filesystem_node_type.h"
#include "augs/string/path_sanitization.h"
#include "augs/string/to_forward_slashes.h"
#include "view/necessary_image_id.h"

namespace augs {
	bool natural_order(const std::string& a, const std::string& b);
}

struct editor_filesystem_node_ui_state {
	bool was_open = false;
};

using editor_filesystem_ui_state = 
	std::map<augs::path_type, editor_filesystem_node_ui_state>
;

struct forbidden_path_result {
	augs::path_type forbidden_path;
	std::size_t len = 0;
	sanitization::forbidden_path_type reason;
	std::optional<std::string> suggested_filename;

	bool can_be_renamed() const {
		return suggested_filename.has_value();
	}

	auto get_suggested_path() const {
		auto good = forbidden_path;
		good.replace_filename(*suggested_filename);

		return good;
	}

	bool operator<(const forbidden_path_result& b) {
		return len < b.len;
	}

	forbidden_path_result(
		const augs::path_type& p,
		const sanitization::forbidden_path_type r
	) : forbidden_path(p), reason(r) {
		const auto bad_filename = augs::string_windows_friendly(forbidden_path.filename());
		const auto good_filename = sanitization::try_generate_sanitized_filename(bad_filename);

		len = augs::string_windows_friendly(p).length();

		if (good_filename != bad_filename) {
			suggested_filename = good_filename;
		}
	}
};

using editor_forbidden_paths_result = std::vector<forbidden_path_result>;

struct editor_filesystem_node {
	ad_hoc_entry_id file_thumbnail_id = static_cast<ad_hoc_entry_id>(-1);
	std::optional<assets::necessary_image_id> necessary_atlas_icon;

	editor_resource_id associated_resource;

	editor_filesystem_node_type type = editor_filesystem_node_type::OTHER_FILE;

	bool is_open = false;
	bool passed_filter = false;
	bool should_sort = true;
	bool hidden_in_explorer = false;

	std::string name;
	std::string after_text;

	std::vector<editor_filesystem_node> files;
	std::vector<editor_filesystem_node> subfolders;

	int level = 0;
	augs::file_time_type last_write_time; 

	/* Used for official resources */
	augs::path_type custom_thumbnail_path;

	editor_filesystem_node* parent = nullptr;

	bool just_one_file() const {
		return files.size() == 1 && subfolders.empty();
	}

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

	void open_all_parents() {
		for_each_parent([](auto& node){ node.is_open = true; });
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
	void in_ui_order(
		F&& callback,
		const bool with_closed_folders = false,
		const bool skip_empty_folders = false
	) {
		for (auto& subfolder : subfolders) {
			if (skip_empty_folders) {
				if (subfolder.subfolders.empty() && subfolder.files.empty()) {
					continue;
				}
			}

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
		if (should_sort) {
			sort_range(files);
			sort_range(subfolders);
		}

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

	void set_file_type_by(const std::string& extension) {
		type = ::get_filesystem_node_type_by_extension(extension);
	}

	template <class F>
	void build_from(
		const augs::path_type& folder_path,
		const augs::path_type& project_folder,
		F should_hide_in_explorer,
		editor_forbidden_paths_result& out_ignored_paths
	) {
		clear();

		auto add_entry = [this, should_hide_in_explorer, &project_folder, &out_ignored_paths](const auto& untrusted_path, const bool folder) {
			auto add_file_or_folder = [&](auto filename, const bool hidden_in_explorer) {
				if (folder) {
					filename.replace_extension("");
				}

				editor_filesystem_node new_node;
				new_node.name = filename.string();
				new_node.hidden_in_explorer = hidden_in_explorer;

				if (folder) {
					new_node.type = editor_filesystem_node_type::FOLDER;
					subfolders.emplace_back(std::move(new_node));
				}
				else {
					const auto extension = filename.extension().string();
					new_node.set_file_type_by(extension);

					try {
						new_node.last_write_time = augs::last_write_time(untrusted_path);
						files.emplace_back(std::move(new_node));
					}
					catch (...) {

					}
				}
			};

			const auto relative_in_project = std::filesystem::relative(untrusted_path, project_folder);

			if (should_hide_in_explorer(untrusted_path)) {
				add_file_or_folder(untrusted_path.filename(), true);
			}
			else {
				auto filename_to_sanitize = ::to_forward_slashes(augs::string_windows_friendly(relative_in_project));

				if (folder) {
					/* Add a dummy extension to silence the NO_EXTENSION error for folders */
					filename_to_sanitize += ".png";
				}

				std::visit(
					[&]<typename S>(const S& sanitization_result) {
						if constexpr(std::is_same_v<S, augs::path_type>) {
							add_file_or_folder(sanitization_result.filename(), false);
						}
						else {
							out_ignored_paths.emplace_back(relative_in_project, sanitization_result);
						}
					},

					sanitization::sanitize_downloaded_file_path(project_folder, filename_to_sanitize)
				);
			}

			return callback_result::CONTINUE;
		};

		augs::for_each_in_directory(
			folder_path,
			[add_entry](const auto& path) { return add_entry(path, true); },
			[add_entry](const auto& path) { return add_entry(path, false); }
		);

		for (auto& subfolder : subfolders) {
			subfolder.build_from(
				folder_path / subfolder.name,
				project_folder, 
				should_hide_in_explorer, 
				out_ignored_paths
			);
		}

		adding_children_finished();
	}

	void adding_children_finished() {
		sort_all();
		set_parents(level);
	}

	ad_hoc_entry_id fill_thumbnail_entries(
		const augs::path_type& project_folder,
		std::unordered_map<augs::path_type, ad_hoc_entry_id>& out_subjects,
		ad_hoc_entry_id id_counter = 1
	) {
		for_each_file_recursive([&id_counter, &out_subjects, &project_folder](auto& node) {
			auto path = project_folder / node.get_path_in_project();

			if (node.hidden_in_explorer) {
				return;
			}

			if (!node.custom_thumbnail_path.empty()) {
				path = node.custom_thumbnail_path;
			}

			if (assets::is_supported_extension<assets::image_id>(path.extension().string())) {
				if (const auto found_id = mapped_or_nullptr(out_subjects, path)) {
					node.file_thumbnail_id = *found_id;
				}
				else {
					node.file_thumbnail_id = id_counter++;
					out_subjects[path] = node.file_thumbnail_id;
				}
			}
		});

		return id_counter;
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

	template <class F>
	void rebuild_from(
		const augs::path_type& project_folder,
		F should_hide_in_explorer,
		editor_forbidden_paths_result& out_ignored_paths
	) {
		auto saved_state = make_ui_state();

		root.build_from(project_folder, project_folder, should_hide_in_explorer, out_ignored_paths);

		apply_ui_state(saved_state);
	}
};

