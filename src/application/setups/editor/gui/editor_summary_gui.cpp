#include "application/setups/editor/editor_setup.h"
#include "application/setups/editor/gui/editor_summary_gui.h"

#include "augs/templates/chrono_templates.h"

#include "augs/misc/imgui/imgui_utils.h"
#include "augs/misc/imgui/imgui_scope_wrappers.h"
#include "augs/misc/imgui/imgui_control_wrappers.h"

void editor_summary_gui::open() {
	show = true;
}

void editor_summary_gui::perform(editor_setup& setup) {
	if (!show || !setup.anything_opened()) {
		return;
	}

	using namespace augs::imgui;

	auto& v = setup.view();
	auto& f = setup.folder();
	auto& cosm = setup.work().world;
	const auto settings = setup.settings;

	auto summary = scoped_window("Summary", &show, ImGuiWindowFlags_AlwaysAutoResize);

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

		const auto& s = cosm.get_solvable();

		s.for_each_pool([&](const auto& p){
			using T = entity_type_of<typename std::decay_t<decltype(p)>::mapped_type>;

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

void editor_coordinates_gui::open() {
	show = true;
}

void editor_coordinates_gui::perform(
	editor_setup& setup,
	const vec2i screen_size,
	const vec2i mouse_pos,
   	const std::unordered_set<entity_id>& all_selected
) {
	if (!show) {
		return;
	}

	using namespace augs::imgui;

	const auto settings = setup.settings;
	auto& v = setup.view();
	auto coordinates = scoped_window("Coordinates", &show, ImGuiWindowFlags_AlwaysAutoResize);

	if (const auto current_cone = setup.find_current_camera()) {
		const auto world_cursor_pos = current_cone->to_world_space(screen_size, mouse_pos);

		text("Grid size: %x/%x", v.grid.unit_pixels, settings.grid.render.get_maximum_unit());

		text("Cursor: %x", world_cursor_pos);
		text("View center: %x", vec2(current_cone->transform.pos).discard_fract());

		{
			auto zoom = current_cone->zoom * 100.f;

			if (slider("Zoom: ", zoom, 1.f, 1000.f, "%.3f%%")) {
				if (!v.panned_camera.has_value()) {
					v.panned_camera = current_cone;
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
			text("AABB:   %x x %x pixels\ncenter: %x\nlt:     %x", size.x, size.y, aabb->get_center(), aabb->left_top());
		}
	}
	else {
		text("No entity selected");
	}
}
