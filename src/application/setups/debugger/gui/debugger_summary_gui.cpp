#include "augs/string/string_templates.h"
#include "augs/misc/compress.h"

#include "application/setups/debugger/debugger_setup.h"
#include "application/setups/debugger/gui/debugger_summary_gui.h"

#include "augs/templates/chrono_templates.h"

#include "augs/misc/imgui/imgui_utils.h"
#include "augs/misc/imgui/imgui_scope_wrappers.h"
#include "augs/misc/imgui/imgui_control_wrappers.h"
#include "augs/misc/readable_bytesize.h"

#include "augs/misc/pool/pool_io.hpp"
#include "augs/readwrite/memory_stream.h"
#include "augs/readwrite/byte_readwrite.h"

#include "game/detail/passes_filter.h"
#include "application/network/net_solvable_stream.h"

void debugger_summary_gui::perform(debugger_setup& setup) {
	using namespace augs::imgui;

	auto summary = make_scoped_window(ImGuiWindowFlags_AlwaysAutoResize);

	if (!summary || !setup.anything_opened()) {
		return;
	}

	const auto& v = setup.view();
	const auto& f = setup.folder();
	const auto& cosm = setup.work().world;

	text(typesafe_sprintf("Folder path: %x", f.current_path));

	const auto total_text = 
		typesafe_sprintf("Total entities: %x###totalentities", cosm.get_entities_count())
	;

	if (auto total = scoped_tree_node(total_text.c_str())) {
		{
			augs::byte_counter_stream counter_stream;

			const auto& solvable = cosm.get_solvable().significant;

			augs::write_bytes(counter_stream, solvable);

			text("Solvable size: %x", readable_bytesize(counter_stream.size()));

			if (auto scope = scoped_tree_node("(Net) Test compression")) {
				augs::timer t;

				thread_local std::vector<std::byte> input_buf;
				thread_local std::vector<std::byte> compressed_buf;
				thread_local auto compression_state = augs::make_compression_state();

				input_buf.clear();
				compressed_buf.clear();

				{
					const auto& clean_round_state = setup.is_gameplay_on() ? setup.get_arena_handle().clean_round_state : solvable;

					auto s = net_solvable_stream_ref(cosm.get_common_significant().flavours, clean_round_state, solvable, input_buf);
					augs::write_bytes(s, solvable);
				}

				text("\n(Net) Solvable size: %x", readable_bytesize(input_buf.size()));

				text("Raw write time: %x ms", t.template get<std::chrono::milliseconds>());

				t.reset();

				augs::compress(compression_state, input_buf, compressed_buf);

				text("Compression time: %x ms", t.template get<std::chrono::milliseconds>());
				text("Compressed size: %x", readable_bytesize(compressed_buf.size()));

				t.reset();

				augs::decompress(compressed_buf, input_buf);

				text("Decompression time: %x ms", t.template get<std::chrono::milliseconds>());
			}
		}

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
			const auto ca = p.max_size();
			const auto percent = static_cast<float>(si) / static_cast<float>(ca) * 100;

			lines.push_back(
				typesafe_sprintf("%1f", percent) + "% " 
				+ typesafe_sprintf("(%x/%x) - %x\n", si, ca, format_field_name(get_type_name<T>()))
			);
		});

		add_sorted_lines();
		text(content);

		lines.clear();
		content.clear();

		text("Total percentages: ");

		s.for_each_entity_pool([&](const auto& p){
			using T = entity_type_of<typename remove_cref<decltype(p)>::mapped_type>;

			const auto si = p.size();
			const auto ca = cosm.get_entities_count();
			const auto percent = static_cast<float>(si) / static_cast<float>(ca) * 100;

			lines.push_back(
				typesafe_sprintf("%x (%1f", si, percent) + "%) - "  + format_field_name(get_type_name<T>()) + "\n"
			);
		});

		add_sorted_lines();
		text(content);
	}

	text("World time: %x (%x steps at %x Hz)",
		format_mins_secs_ms(cosm.get_total_seconds_passed()),
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

#include "view/rendering_scripts/find_aabb_of.h"
#include "application/setups/debugger/debugger_paths.h"
#include "game/cosmos/for_each_entity.h"

render_layer_filter get_layer_filter_for_miniature();

void debugger_coordinates_gui::perform(
	debugger_setup& setup,
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

	auto& miniature_generator = setup.miniature_generator;
	auto& f = setup.folder();

	thread_local int requested_size = 400;

	if (miniature_generator == std::nullopt) {
		ImGui::SliderInt("Miniature size", &requested_size, 16, 10000);

		if (ImGui::Button("Create miniature")) {
			const auto filter = get_layer_filter_for_miniature();

			const auto& cosm = setup.work().world;

			auto for_each_target = [&](auto combiner) {
				cosm.for_each_entity(
					[&](const auto& typed_entity) {
						if (filter.passes(typed_entity)) {
							combiner(typed_entity);
						}
					}
				);
			};

			if (const auto world_captured_region = ::find_aabb_of(cosm, for_each_target)) {
				miniature_generator_state request;
				auto cam = setup.find_current_camera_eye();

				request.output_path = f.get_paths().arena.miniature_file_path;
				request.world_captured_region = *world_captured_region;
				request.zoom = cam ? cam->zoom : 1.0f;

				request.screen_size = screen_size;
				request.max_miniature_size = requested_size;

				miniature_generator.emplace(std::move(request));
			}
		}
	}

	if (miniature_generator.has_value()) {
		if (miniature_generator->complete()) {
			miniature_generator.reset();
		}
	}
}
