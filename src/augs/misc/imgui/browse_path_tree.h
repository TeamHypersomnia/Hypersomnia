#pragma once
#include "augs/filesystem/path.h"
#include "augs/string/typesafe_sprintf.h"
#include "augs/misc/imgui/imgui_control_wrappers.h"
#include "augs/misc/imgui/path_tree_settings.h"

struct path_tree_detail {
	bool acquire_keyboard = false;
	ImVec2 files_view_size = ImVec2(0, 0);
};

struct browsed_path_entry_base {
protected:
	augs::path_type p;

public:
	browsed_path_entry_base() = default;
	browsed_path_entry_base(const augs::path_type& p) : p(p) {}

	bool operator<(const browsed_path_entry_base& b) const {
		return p < b.p;
	}

	auto get_filename() const {
		return p.filename();
	}

	auto get_displayed_directory() const {
		auto dir = augs::path_type(p).replace_filename("").string();
		return cut_preffix(dir, "content/");
	}

	const auto& get_full_path() const {
		return p;
	}
};

template <class F, class C, class... ColumnCallbacks>
void browse_path_tree(
	path_tree_settings& settings,
	const C& all_paths,
	F path_callback,
	const path_tree_detail detail,
	std::array<const char*, sizeof...(ColumnCallbacks)> column_names,
	ColumnCallbacks... column_callbacks
) {
	using namespace augs::imgui;

	constexpr auto extra_columns = sizeof...(column_callbacks);

	const auto prettify_filenames = settings.prettify_filenames;

	thread_local ImGuiTextFilter filter;
	filter.Draw();

	if (detail.acquire_keyboard) {
		ImGui::SetKeyboardFocusHere();
	}

	auto prettify = [prettify_filenames](const std::string& filename) {
		return prettify_filenames ? format_field_name(augs::path_type(filename).stem()) : filename;
	};

	auto files_view = scoped_child("Files view", detail.files_view_size);

	if (settings.linear_view) {
		ImGui::Columns(2 + extra_columns);

		text_disabled(prettify_filenames ? "Name" : "Filename");
		ImGui::NextColumn();
		text_disabled("Location");
		ImGui::NextColumn();

		for (const auto& name : column_names) {
			text_disabled(name);
			ImGui::NextColumn();
		}

		ImGui::Separator();

		for (const auto& l : all_paths) {
			const auto prettified = prettify(l.get_filename());
			const auto displayed_dir = l.get_displayed_directory();

			if (!filter.PassFilter(prettified.c_str()) && !filter.PassFilter(displayed_dir.c_str())) {
				continue;
			}

			path_callback(l, prettified);
			ImGui::NextColumn();

			text_disabled(displayed_dir);

			ImGui::NextColumn();

			auto col = [&](auto&& callback) {
				callback(l);
				ImGui::NextColumn();
			};

			(col(column_callbacks), ...);
		}
	}
	else {

	}
}

template <class F, class C>
void browse_path_tree(
	path_tree_settings& settings,
	const C& all_paths,
	F path_callback,
	const path_tree_detail detail
) {
	browse_path_tree(settings, all_paths, path_callback, detail, {});
}
