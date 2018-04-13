#include "augs/string/string_templates.h"

#include "augs/misc/imgui/imgui_scope_wrappers.h"
#include "augs/misc/imgui/imgui_control_wrappers.h"

#include "application/setups/editor/gui/editor_assets_gui.h"
#include "application/setups/editor/editor_folder.h"
#include "application/intercosm.h"

void editor_images_gui::perform(editor_command_input in) {
	using namespace augs::imgui;

	auto window = make_scoped_window();

	if (!window) {
		return;
	}

	checkbox("Linear view", linear_view);
	ImGui::SameLine();
	checkbox("Prettify filenames", prettify_filenames);
	ImGui::Separator();

	auto& work = *in.folder.work;
	auto& cosm = work.world;

	const auto& viewables = work.viewables;

	struct sorted_path_entry {
		std::string filename;
		std::string directory;

		bool operator<(const sorted_path_entry& b) const {
			const auto& f1 = augs::path_type(filename);
			const auto& f2 = augs::path_type(b.filename);

			return std::tie(f1, directory) < std::tie(f2, b.directory);
		}
	};

	auto make_display_path = [this](const augs::path_type& from) {
		sorted_path_entry output;

		if (prettify_filenames) {
			output.filename = format_field_name(from.stem());
		}
		else {
			output.filename = from.filename();
		}

		output.directory = augs::path_type(from).replace_filename("").string();

		return output;
	};

	std::vector<sorted_path_entry> all_paths;

	for (const auto& l : viewables.image_loadables) {
		all_paths.push_back(make_display_path(l.source_image_path));
	}

	sort_range(all_paths);

	if (linear_view) {
		ImGui::Columns(3);

		text_disabled(prettify_filenames ? "Name" : "Filename");
		ImGui::NextColumn();
		text_disabled("Location");
		ImGui::NextColumn();
		text_disabled("Details");
		ImGui::NextColumn();

		ImGui::Separator();

		for (const auto& l : all_paths) {
			ImGui::Selectable(l.filename.c_str());

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
