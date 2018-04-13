#include "augs/string/string_templates.h"

#include "augs/misc/imgui/imgui_scope_wrappers.h"
#include "augs/misc/imgui/imgui_control_wrappers.h"

#include "application/setups/editor/gui/editor_assets_gui.h"
#include "application/setups/editor/editor_folder.h"
#include "application/intercosm.h"

struct sorted_path_entry {
	std::string filename;
	std::string directory;

	sorted_path_entry() = default;
	sorted_path_entry(augs::path_type from) {
		filename = from.filename();
		directory = from.replace_filename("");
	}

	bool operator<(const sorted_path_entry& b) const {
		const auto& f1 = filename;
		const auto& f2 = b.filename;

		return std::tie(directory, f1) < std::tie(b.directory, f2);
	}
};

auto prettify(const std::string& filename) {
	return format_field_name(augs::path_type(filename).stem());
}

template <class F>
void browse_project_files(
	const char* const folder_name,
	const files_browser_settings& settings,
	const std::vector<sorted_path_entry>& entries,
	F client
) {
	using namespace augs::imgui;

	const auto official_directory = typesafe_sprintf("content/official/%x", folder_name);
	const auto prettify_filenames = settings.prettify_filenames;

	if (settings.linear_view) {
		ImGui::Columns(3);

		text_disabled(prettify_filenames ? "Name" : "Filename");
		ImGui::NextColumn();
		text_disabled("Location");
		ImGui::NextColumn();
		text_disabled("Details");
		ImGui::NextColumn();

		ImGui::Separator();

		for (const auto& l : entries) {
			if (prettify_filenames) {
				ImGui::Selectable(prettify(l.filename).c_str());
			}
			else {
				ImGui::Selectable(l.filename.c_str());
			}

			ImGui::NextColumn();

			auto displayed_dir = l.directory;
			cut_preffix(displayed_dir, "content/");
			text_disabled(displayed_dir);

			ImGui::NextColumn();
			ImGui::NextColumn();
		}
	}
	else {

	}
}

void files_browser_settings::do_tweakers() {
	using namespace augs::imgui;

	checkbox("Linear view", linear_view);
	ImGui::SameLine();
	checkbox("Prettify filenames", prettify_filenames);
}

struct gfx_browser_client {

};

void editor_images_gui::perform(editor_command_input in) {
	using namespace augs::imgui;

	auto window = make_scoped_window();

	if (!window) {
		return;
	}

	browser_settings.do_tweakers();
	ImGui::Separator();

	auto& work = *in.folder.work;
	auto& cosm = work.world;

	const auto& viewables = work.viewables;

	thread_local std::vector<sorted_path_entry> all_paths;
	all_paths.clear();

	for (const auto& l : viewables.image_loadables) {
		all_paths.emplace_back(l.source_image_path);
	}

	sort_range(all_paths);

	browse_project_files(
		"gfx",
		browser_settings,
		all_paths,
		gfx_browser_client()
	);
}
