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

template <class F, class C>
void browse_path_tree(
	path_tree_settings& settings,
	const C& all_paths,
	F path_callback,
	const path_tree_detail detail = {}
) {
	using namespace augs::imgui;

	settings.do_tweakers();
	ImGui::Separator();

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
		ImGui::Columns(3);

		text_disabled(prettify_filenames ? "Name" : "Filename");
		ImGui::NextColumn();
		text_disabled("Location");
		ImGui::NextColumn();
		text_disabled("Details");
		ImGui::NextColumn();

		ImGui::Separator();

		for (const auto& l : all_paths) {
			const auto prettified = prettify(l.get_filename());

			auto displayed_dir = l.get_displayed_directory();

			if (!filter.PassFilter(prettified.c_str()) && !filter.PassFilter(displayed_dir.c_str())) {
				continue;
			}

			path_callback(l, prettified);
			ImGui::NextColumn();

			text_disabled(displayed_dir);

			ImGui::NextColumn();
			ImGui::NextColumn();
		}
	}
	else {

	}
}
