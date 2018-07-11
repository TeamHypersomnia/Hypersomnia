#include "augs/string/string_templates.h"

#include "application/setups/editor/editor_setup.h"
#include "application/setups/editor/gui/editor_summary_gui.h"

#include "augs/templates/chrono_templates.h"

#include "augs/misc/imgui/imgui_utils.h"
#include "augs/misc/imgui/imgui_scope_wrappers.h"
#include "augs/misc/imgui/imgui_control_wrappers.h"

void editor_summary_gui::perform(editor_setup& setup) {
	using namespace augs::imgui;

	auto summary = make_scoped_window(ImGuiWindowFlags_AlwaysAutoResize);

	if (!summary || !setup.anything_opened()) {
		return;
	}

	auto& v = setup.view();
	auto& f = setup.folder();
	auto& cosm = setup.work().world;

	text(typesafe_sprintf("Folder path: %x", f.current_path));

	const auto total_text = 
		typesafe_sprintf("Total entities: %x###totalentities", cosm.get_entities_count())
	;

	if (auto total = scoped_tree_node(total_text.c_str())) {
		text("Usage of maximum pool space: ");

		std::vector<std::string> lines;

		std::string content;

		auto add_sorted_lines = [&]() {
			stable_sort_range(
				lines,
				[](const auto& l1, const auto& l2) {
					return stof(l1) > stof(l2);
				}
			);	

			for (const auto& l : lines) {
				content += l;
			}
		};

		const auto& s = cosm.get_solvable().significant;

		s.for_each_entity_pool([&](const auto& p){
			using T = entity_type_of<typename remove_cref<decltype(p)>::mapped_type>;

			const auto si = p.size();
			const auto ca = p.capacity();
			const auto percent = static_cast<float>(si) / static_cast<float>(ca) * 100;

			lines.push_back(
				typesafe_sprintf("%1f", percent) + "% " 
				+ typesafe_sprintf("(%x/%x) - %x\n", si, ca, format_field_name(get_type_name<T>()))
			);
		});

		add_sorted_lines();
		text(content);
	}

	text("World time: %x (%x steps at %x Hz)",
		standard_format_seconds(cosm.get_total_seconds_passed()),
		cosm.get_total_steps_passed(),
		1.0f / cosm.get_fixed_delta().in_seconds()
	);

	const auto viewed = setup.get_viewed_character();

	text("Currently controlling: %x",
		viewed.alive() ? viewed.get_name() : "no entity"
	);

	if (v.ignore_groups) {
		text("Groups disabled");
	}
	else {
		text("Groups enabled");
	}
}

void editor_coordinates_gui::perform(
	editor_setup& setup,
	const vec2i screen_size,
	const vec2i mouse_pos,
   	const std::unordered_set<entity_id>& all_selected
) {
	using namespace augs::imgui;

	auto coordinates = make_scoped_window(ImGuiWindowFlags_AlwaysAutoResize);

	if (!coordinates) {
		return;
	}

	const auto settings = setup.settings;
	auto& v = setup.view();

	if (const auto current_eye = setup.find_current_camera_eye()) {
		const auto cone = camera_cone(*current_eye, screen_size);
		const auto world_cursor_pos = cone.to_world_space(mouse_pos);

		text("Grid size: %x/%x", v.grid.unit_pixels, settings.grid.render.get_maximum_unit());

		text("Cursor: %x", world_cursor_pos);
		text("View center: %x", vec2(current_eye->transform.pos).discard_fract());
		text("Camera AABB: %x", cone.get_visible_world_rect_aabb());

		{
			auto zoom = current_eye->zoom * 100.f;

			if (slider("Zoom: ", zoom, 1.f, 1000.f, "%.3f%%")) {
				if (!v.panned_camera.has_value()) {
					v.panned_camera = current_eye;
				}

				zoom = std::clamp(zoom, 1.f, 1000.f);
				v.panned_camera->zoom = zoom / 100.f;
			}
		}
	}

	text("Rect select mode: %x", format_enum(v.rect_select_mode));

	if (!all_selected.empty()) {
		text("Selected %x entities", all_selected.size());

		if (const auto aabb = setup.find_selection_aabb()) {
			const auto size = aabb->get_size();
			text("AABB:   %x x %x pixels\ncenter: %x\nlt:     %x\nrb:     %x", size.x, size.y, aabb->get_center(), aabb->left_top(), aabb->right_bottom());
		}
	}
	else {
		text("No entity selected");
	}
}
