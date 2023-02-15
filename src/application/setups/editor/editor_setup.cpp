#include "game/cosmos/logic_step.h"
#include "application/setups/debugger/property_debugger/widgets/asset_sane_default_provider.h"
#include "game/organization/all_messages_includes.h"
#include "augs/misc/pool/pool_allocate.h"
#include "augs/misc/secure_hash.h"
#include "augs/string/first_free.h"

#include "view/viewables/viewables_loading_type.h"

#include "application/config_lua_table.h"
#include "application/setups/editor/editor_setup.h"
#include "application/setups/editor/editor_paths.h"
#include "application/setups/editor/project/editor_project_readwrite.h"

#include "augs/filesystem/directory.h"
#include "augs/readwrite/byte_readwrite.h"
#include "augs/readwrite/byte_file.h"
#include "augs/log.h"

#include "augs/misc/time_utils.h"
#include "application/setups/editor/resources/editor_sprite_resource.h"
#include "application/setups/editor/resources/editor_sound_resource.h"

#include "application/setups/editor/commands/create_layer_command.hpp"
#include "application/setups/editor/commands/edit_node_command.hpp"
#include "application/setups/editor/commands/edit_layer_command.hpp"
#include "application/setups/editor/commands/edit_resource_command.hpp"
#include "application/setups/editor/commands/create_node_command.hpp"
#include "application/setups/editor/commands/reorder_nodes_command.hpp"
#include "application/setups/editor/commands/reorder_layers_command.hpp"

#include "application/setups/editor/editor_setup.hpp"

#include "augs/templates/history.hpp"

#include "game/cosmos/create_entity.hpp"
#include "application/setups/editor/editor_camera.h"
#include "application/setups/draw_setup_gui_input.h"

#include "game/detail/visible_entities.h"
#include "game/detail/get_hovered_world_entity.h"
#include "application/setups/editor/official/create_official_resources.h"
#include "augs/gui/text/printer.h"
#include "view/rendering_scripts/draw_area_indicator.h"
#include "view/rendering_scripts/for_each_iconed_entity.h"
#include "augs/drawing/general_border.h"
#include "augs/drawing/drawing.hpp"
#include "augs/templates/wrap_templates.h"
#include "application/setups/editor/selector/editor_entity_selector.hpp"
#include "application/setups/editor/editor_setup_for_each_inspected_entity.hpp"
#include "application/setups/editor/editor_setup_find_aabb_of_nodes.hpp"
#include "augs/templates/traits/has_size.h"
#include "augs/templates/traits/has_flip.h"
#include "application/setups/editor/detail/make_command_from_selections.h"
#include "application/setups/editor/editor_rebuild_scene.hpp"
#include "application/setups/editor/has_thumbnail_id.h"
#include "game/detail/snap_interpolation_to_logical.h"

editor_setup::editor_setup(
	sol::state& lua,
	const augs::path_type& project_path
) : paths(project_path) {
	initial_scene.populate_official_content(
		lua,
		60,
		default_bomb_ruleset,
		default_test_ruleset
	);

	create_official();

	LOG("Loading editor project at: %x", project_path);
	project = editor_project_readwrite::read_project_json(paths.project_json);

	load_gui_state();
	open_default_windows();

	on_window_activate();
	save_last_project_location();
}

editor_setup::~editor_setup() {
	save_gui_state();
	LOG("DTOR finished: ~editor_setup");
}

void editor_setup::create_official() {
	::create_official_resources(initial_scene, official_resources);
	::create_official_filesystem_from(
		initial_scene,
		official_resources,
		official_files_root
	);

	gui.filesystem.rebuild_official_special_filesystem(*this);
	gui.filesystem.rebuild_special_filesystem(*this);
}

void editor_setup::open_default_windows() {
	gui.inspector.open();
	gui.layers.open();
	gui.filesystem.open();
	gui.history.open();
	gui.toolbar.open();
}

bool editor_setup::handle_input_before_imgui(
	handle_input_before_imgui_input in
) {
	using namespace augs::event;

	if (is_playtesting()) {
		return false;
	}

	if (in.e.was_pressed(keys::key::LMOUSE)) {
		if (mover.do_left_press(make_mover_input())) {
			return true;	
		}
	}

	/* 
		We can't use RMB to cancel operations because we're using RMB to move around the screen.
		Only MMB is left.
	*/

	if (in.e.was_pressed(keys::key::MMOUSE)) {
		if (mover.is_active(history)) {
			escape();
			return true;
		}
	}

	if (in.e.was_any_key_pressed()) {
		using namespace augs::event::keys;

		const auto k = in.e.data.key.key;

		const auto& state = in.common_input_state;

		const bool has_alt{ state[key::LALT] };
		const bool has_ctrl{ state[key::LCTRL] };
		const bool has_shift{ state[key::LSHIFT] };
		const bool no_modifiers = !has_alt && !has_ctrl && !has_shift;

		if (gui.layers.is_focused()) {
			if ((k == key::UP && no_modifiers) || (k == key::TAB && has_shift)) {
				gui.layers.pressed_arrow = vec2i(0, -1);
				ImGui::GetIO().KeysDown[int(key::ENTER)] = true;
				gui.layers.request_confirm_rename = true;

				return true;
			}
			else if ((k == key::DOWN && no_modifiers) || k == key::TAB) {
				gui.layers.pressed_arrow = vec2i(0, 1);
				ImGui::GetIO().KeysDown[int(key::ENTER)] = true;
				gui.layers.request_confirm_rename = true;

				return true;
			}
		}

		if (no_modifiers && !ImGui::GetIO().WantCaptureKeyboard) {
			switch (k) {
				case key::LEFT: if (gui.layers.is_focused()) { gui.layers.pressed_arrow = vec2i(-1, 0); return true; } else break;
				case key::RIGHT: if (gui.layers.is_focused()) { gui.layers.pressed_arrow = vec2i(1, 0); return true; } else break;
				case key::UP: if (gui.layers.is_focused()) { gui.layers.pressed_arrow = vec2i(0, -1); return true; } else break;
				case key::DOWN: if (gui.layers.is_focused()) { gui.layers.pressed_arrow = vec2i(0, 1); return true; } else break;

				default: break;
			}
		}
	}

	if (in.e.msg == message::ldoubleclick) {
		handle_doubleclick_in_layers_gui = true;
	}

	if (in.e.msg == message::activate || in.e.msg == message::click_activate) {
		on_window_activate();
	}

	if (in.e.msg == message::deactivate) {
		force_autosave_now();
	}

	return false;
}

bool editor_setup::confirm_modal_popup() {
	if (ok_only_popup) {
		ok_only_popup = std::nullopt;
		return true;
	}

	return false;
}

bool editor_setup::handle_input_before_game(
	handle_input_before_game_input in
) {
	using namespace augs::event;
	using namespace augs::event::keys;

	if (is_playtesting()) {
		return false;
	}

	const auto& state = in.common_input_state;
	const auto& e = in.e;

	const bool has_ctrl{ state[key::LCTRL] };
	const bool has_shift{ state[key::LSHIFT] };

	if (e.was_any_key_pressed()) {
		const auto k = e.data.key.key;

		if (!has_ctrl && k == key::ENTER) {
			if (confirm_modal_popup()) {
				return true;
			}

			if (mover.escape()) {
				return true;
			}
		}	

		if (has_ctrl) {
			if (has_shift) {
				switch (k) {
					case key::Z: redo(); return true;
					default: break;
				}
			}

			switch (k) {
				case key::A: select_all_entities(); return true;
				case key::Z: undo(); return true;
				//case key::C: copy(); return true;
				//case key::X: cut(); return true;
				//case key::V: paste(); return true;

				case key::LEFT: mirror_selection(vec2i(-1, 0)); return true;
				case key::RIGHT: mirror_selection(vec2i(1, 0)); return true;
				case key::UP: mirror_selection(vec2i(0, -1)); return true;
				case key::DOWN: mirror_selection(vec2i(0, 1)); return true;

				case key::R: rotate_selection_once_by(90); return true;
				case key::E: start_resizing_selection(true); return true;
				case key::T: gui.toolbar.toggle(); return true;
				case key::F: gui.filesystem.open(); return true;
				case key::L: gui.layers.open(); return true;
				case key::H: gui.history.open(); return true;
				case key::I: gui.inspector.open(); return true;
				default: break;
			}
		}

		if (has_shift && !has_ctrl) {
			switch (k) {
				case key::N: move_inspected_to_new_layer(); return true;
				case key::R: rotate_selection_once_by(-90); return true;
				case key::H: flip_selection_horizontally(); return true;
				case key::V: flip_selection_vertically(); return true;
				default: break;
			}
		}

		if (!has_shift && !has_ctrl) {
			switch (k) {
				case key::F2: start_renaming_selection(); return true;

				case key::N: create_new_layer(); return true;

				case key::C: duplicate_selection(); return true;
				case key::D: cut_selection(); return true;
				case key::DEL: delete_selection(); return true;

				case key::T: start_moving_selection(); return true;
				case key::E: start_resizing_selection(false); return true;
				case key::R: start_rotating_selection(); return true;
				case key::W: reset_rotation_of_selected_nodes(); return true;
				case key::F: center_view_at_selection(); return true;
				case key::SPACE: start_playtesting(); return true;
				case key::G: toggle_grid(); return true;
				case key::S: toggle_snapping(); return true;
				case key::OPEN_SQUARE_BRACKET: sparser_grid(); return true;
				case key::CLOSE_SQUARE_BRACKET: denser_grid(); return true;

				default: break;
			}
		}
	}

	const auto world_cursor_pos = get_world_cursor_pos();
	const auto current_eye = get_camera_eye();

	const auto screen_size = vec2i(ImGui::GetIO().DisplaySize);
	const auto current_cone = camera_cone(current_eye, screen_size);

	auto& cosm = scene.world;

	if (editor_detail::handle_camera_input(
		{},
		current_cone,
		state,
		e,
		world_cursor_pos,
		view.panned_camera
	)) {
		return true;
	}

	get_all_selected_by_selector(cached_selected_comparison);

	auto check_changed = [&]() {
		get_all_selected_by_selector(cached_selected_comparison_after);

		sort_range(cached_selected_comparison);
		sort_range(cached_selected_comparison_after);

		if (cached_selected_comparison_after != cached_selected_comparison) {
			inspect(cached_selected_comparison_after);
		}
	};

	if (e.msg == message::ldoubleclick) {
		if (auto node = get_hovered_node(); node.is_set()) {
			center_view_at_selection();
			reset_zoom();
		}
	}

	if (e.msg == message::mousemotion) {
		if (mover.do_mousemotion(make_mover_input(), world_cursor_pos)) {
			return true;
		}

		selector.do_mousemotion(
			in.sizes_for_icons,
			cosm,
			view.rect_select_mode,
			world_cursor_pos,
			current_eye,
			state[key::LMOUSE],
			render_layer_filter::all()
		);

		check_changed();

		return true;
	}

	auto& selections = entity_selector_state;

	if (e.was_pressed(key::LMOUSE)) {
		selector.do_left_press(cosm, has_ctrl, world_cursor_pos, selections);
		check_changed();

		if (const auto node_id = to_node_id(selector.get_held()); node_id.is_set()) {
			scroll_once_to(node_id);
		}

		return true;
	}

	else if (e.was_released(key::LMOUSE)) {
		selections = selector.do_left_release(has_ctrl, make_selector_input());
		check_changed();
	}

	return false;
}

void editor_setup::inspect_from_selector_state() {
	get_all_selected_by_selector(cached_selected_comparison);

	inspect(cached_selected_comparison);
}

void editor_setup::get_all_selected_by_selector(std::vector<entity_id>& into) const {
	into.clear();

	selector.for_each_selected_entity(
		[&](const entity_id id) {
			into.emplace_back(id);
		},
		entity_selector_state
	);

	const auto held = selector.get_held();

	if (scene.world[held]) {
		if (!found_in(into, held)) {
			into.emplace_back(held);
		}
	}
}

vec2 editor_setup::get_world_cursor_pos() const {
	return get_world_cursor_pos(get_camera_eye());
}

vec2 editor_setup::get_world_cursor_pos(const camera_eye eye) const {
	const auto mouse_pos = vec2i(ImGui::GetIO().MousePos);
	const auto screen_size = vec2i(ImGui::GetIO().DisplaySize);

	return camera_cone(eye, screen_size).to_world_space(mouse_pos);
}

void editor_setup::customize_for_viewing(config_lua_table& config) const {
	config.window.name = typesafe_sprintf("Hypersomnia Editor - %x", project.meta.name);
	if (is_playtesting()) {

	}
	else {
		config.drawing.draw_area_markers.is_enabled = false;
		config.drawing.draw_aabb_highlighter = false;
		config.interpolation.enabled = false;
	}
}

std::optional<ad_hoc_atlas_subjects> editor_setup::get_new_ad_hoc_images() {
	if (rebuild_ad_hoc_atlas) {
		rebuild_ad_hoc_atlas = false;

		std::unordered_map<augs::path_type, ad_hoc_entry_id> subject_ids;
		auto next_id = files.root.fill_thumbnail_entries(paths.project_folder, subject_ids);
		next_id = official_files_root.fill_thumbnail_entries(augs::path_type(OFFICIAL_CONTENT_DIR), subject_ids, next_id);
		next_id = gui.filesystem.official_special_root.fill_thumbnail_entries(augs::path_type(OFFICIAL_CONTENT_DIR), subject_ids, next_id);

		ad_hoc_atlas_subjects new_subjects;
		new_subjects.reserve(subject_ids.size());

		for (auto& s : subject_ids) {
			new_subjects.push_back({ s.second, s.first });
		}

		auto cache_thumbnail_id_in_resource = [&](const auto& file_node) {
			on_resource(
				file_node.associated_resource,
				[&]<typename T>(T& typed_resource, const auto resource_id) {
					(void)resource_id;

					if constexpr(has_thumbnail_id_v<T>) {
						typed_resource.thumbnail_id = file_node.file_thumbnail_id;
					}
				}
			);
		};

		gui.filesystem.official_special_root.for_each_file_recursive(cache_thumbnail_id_in_resource);
		official_files_root.for_each_file_recursive(cache_thumbnail_id_in_resource);
		files.root.for_each_file_recursive(cache_thumbnail_id_in_resource);

		if (new_subjects == last_ad_hoc_subjects) {
			return std::nullopt;
		}

		last_ad_hoc_subjects = new_subjects;
		return new_subjects;
	}

	return std::nullopt;
}

void editor_setup::on_window_activate() {
	rebuild_filesystem();
	rebuild_scene();
}

void editor_setup::rebuild_filesystem() {
	files.rebuild_from(paths.project_folder);
	gui.filesystem.clear_drag_drop();
	rebuild_ad_hoc_atlas = true;

	const auto changes = rebuild_pathed_resources();

	if (changes.any()) {
		simple_popup changes_popup;

		changes_popup.title = "Detected changes in filesystem";
		std::string summary;

		const int redirs = changes.redirects.size();
		const int missing = changes.missing.size();

		if (missing > 0) {
			auto f = missing == 1 ? "file is" : "files are";
			summary += typesafe_sprintf("%x %x missing!\n", missing, f);
		}

		if (redirs > 0) {
			auto f = redirs == 1 ? "file has" : "files have";
			summary += typesafe_sprintf("%x %x been automatically redirected.\n", redirs, f);
		}

		std::string details;

		for (auto& c : changes.missing) {
			details += typesafe_sprintf("Missing: %x\n", c.string());
		}

		for (auto& r : changes.redirects) {
			details += typesafe_sprintf("Redirected: %x -> %x\n", r.first.string(), r.second.string());
		}

		changes_popup.title = "Detected changes in filesystem";
		changes_popup.message = summary;
		changes_popup.details = details;

		ok_only_popup = changes_popup;
	}
}

augs::path_type editor_setup::resolve_project_path(const augs::path_type& path_in_project) const {
	return paths.project_folder / path_in_project;
}

editor_paths_changed_report editor_setup::rebuild_pathed_resources() {
	editor_paths_changed_report changes;

	/*
		Before allocating a resource, we want to first check if one exists with this content hash,
		and with own path no longer existing. Only then do we redirect.

		Otherwise we could end up with duplicate resources.
	*/

	auto handle_pool = [&]<typename P>(P& pool, const editor_filesystem_node_type type) {
		using resource_type = typename P::value_type;

		std::unordered_map<std::string,     resource_type*> resource_by_hash;
		std::unordered_map<augs::path_type, resource_type*> resource_by_path;

		auto find = [&](auto& in, const auto& by) -> resource_type* {
			if (auto found = mapped_or_nullptr(in, by)) {
				return *found;
			}

			return nullptr;
		};

		for (auto& entry : pool) {
			auto& r = entry.external_file;

			resource_by_hash[r.content_hash]    = std::addressof(entry);
			resource_by_path[r.path_in_project] = std::addressof(entry);
		}

		auto add_if_new = [&](editor_filesystem_node& file) {
			if (file.type != type) {
				return;
			}

			const auto path_in_project = file.get_path_in_project();
			const auto full_path = resolve_project_path(path_in_project);

			if (auto found_resource = find(resource_by_path, path_in_project)) {
				found_resource->external_file.maybe_rehash(full_path, file.last_write_time);

				const auto existing_id = pool.get_id_of(*found_resource);
				file.associated_resource.set<resource_type>(existing_id, false);
			}
			else {
				std::string new_resource_hash;

				try {
					new_resource_hash = augs::secure_hash(augs::file_to_bytes(full_path));
				}
				catch (...) {

				}

				if (new_resource_hash.empty()) {
					LOG("WARNING! Couldn't get a hash from %x", full_path);
					return;
				}

				auto moved_resource = find(resource_by_hash, new_resource_hash);

				if (moved_resource && !augs::exists(resolve_project_path(moved_resource->external_file.path_in_project))) {
					const auto& new_path = path_in_project;
					auto& moved = moved_resource->external_file;

					changes.redirects.emplace_back(moved.path_in_project, new_path);

					moved.path_in_project = new_path;
					moved.set_hash_stamp(file.last_write_time);

					const auto moved_id = pool.get_id_of(*moved_resource);

					file.associated_resource.set<resource_type>(moved_id, false);
				}
				else {
					const auto [new_id, new_resource] = pool.allocate(editor_pathed_resource(path_in_project, new_resource_hash, file.last_write_time));

					if constexpr(std::is_same_v<editor_sprite_resource, resource_type>) {
						try {
							new_resource.editable.size = augs::image::get_size(full_path);

							if (full_path.extension() == ".gif") {
								new_resource.animation_frames = augs::image::read_gif_frame_meta(full_path);
							}
							else {
								new_resource.animation_frames.clear();
							}
						}
						catch (...) {
							new_resource.editable.size.set(32, 32);
							new_resource.animation_frames.clear();
						}
					}

					file.associated_resource.set<resource_type>(new_id, false);
				}
			}
		};

		files.root.for_each_file_recursive(add_if_new);

		for (auto& entry : pool) {
			auto& r = entry.external_file;

			if (!augs::exists(resolve_project_path(r.path_in_project))) {
				/* 
					If it's still missing after redirect, 
					then it is indeed missing.
				*/

				changes.missing.emplace_back(r.path_in_project);
			}
		}
	};

	auto& sprite_pool = project.resources.pools.template get_for<editor_sprite_resource>();
	auto& sound_pool  = project.resources.pools.template get_for<editor_sound_resource>();

	handle_pool(sprite_pool, editor_filesystem_node_type::IMAGE);
	handle_pool(sound_pool,  editor_filesystem_node_type::SOUND);

	/*
		Corner cases:

		If someone deletes a resource file (e.g. .png or .ogg), it's still in memory and its parameters will be written to the json file.
		If someone moves a file AND modifies it too, it will simply be considered missing.

		Every time you activate the window the editor will try redirecting the deleted files.
	*/

	return changes;
}

void editor_setup::force_autosave_now() {
	save_gui_state();
}

editor_history::index_type editor_setup::get_last_command_index() const {
	return history.get_last_revision();
}

bool editor_setup::exists(const editor_resource_id& id) const {
	return on_resource(id, [&](auto&&...) { return true; }) == std::optional<bool>(true);
}

void editor_setup::clear_inspector() {
	gui.inspector.clear();
}

void editor_setup::inspect(const current_selections_type& selections) {
	bool found_next_marked = false;

	for (const auto entity : selections) {
		if (!found_in(gui.inspector.all_inspected, inspected_variant(to_node_id(entity)))) {
			gui.inspector.mark_last_inspected(to_node_id(entity), selections.size() == 1);
			found_next_marked = true;
			break;
		}
	}

	clear_inspector();

	for (const auto entity : selections) {
		gui.inspector.all_inspected.emplace_back(to_node_id(entity));
	}

	sort_inspected();

	if (!found_next_marked) {
		if (gui.inspector.all_inspected.size() > 0) {
			gui.inspector.mark_last_inspected(gui.inspector.all_inspected.front(), false);
		}
	}
}

void editor_setup::inspect(const std::vector<entity_id>& selections) {
	bool found_next_marked = false;

	for (const auto entity : selections) {
		if (!found_in(gui.inspector.all_inspected, inspected_variant(to_node_id(entity)))) {
			gui.inspector.mark_last_inspected(to_node_id(entity), selections.size() == 1);
			found_next_marked = true;
			break;
		}
	}

	clear_inspector();

	for (const auto entity : selections) {
		gui.inspector.all_inspected.emplace_back(to_node_id(entity));
	}

	sort_inspected();

	if (!found_next_marked) {
		if (gui.inspector.all_inspected.size() > 0) {
			gui.inspector.mark_last_inspected(gui.inspector.all_inspected.front(), false);
		}
	}
}

void editor_setup::inspect_only(const std::vector<editor_node_id>& selections) {
	clear_inspector();

	for (const auto node : selections) {
		gui.inspector.all_inspected.emplace_back(node);
	}

	inspected_to_entity_selector_state();
	sort_inspected();
}

void editor_setup::inspect_only(const std::vector<inspected_variant>& new_inspected) {
	gui.inspector.all_inspected = new_inspected;

	inspected_to_entity_selector_state();
	sort_inspected();
}

void editor_setup::inspected_to_entity_selector_state() {
	entity_selector_state.clear();
	selector.clear();

	for_each_inspected_entity(
		[&](const entity_id id) {
			entity_selector_state.emplace(id);
		}
	);
}

void editor_setup::inspect_add_quiet(inspected_variant new_inspected) {
	gui.inspector.inspect(new_inspected, true);
}

void editor_setup::after_quietly_adding_inspected() {
	inspected_to_entity_selector_state();
	sort_inspected();

	gui.inspector.tweaked_widget.reset();
}

void editor_setup::inspect(inspected_variant new_inspected) {
	gui.inspector.inspect(new_inspected, wants_multiple_selection());

	inspected_to_entity_selector_state();
	sort_inspected();
}

void editor_setup::sort_inspected() {
	auto& orders = gui.inspector.prepare_for_sorting();

	std::size_t i = 0;

	if (gui.inspector.inspects_only<editor_node_id>()) {
		gui.inspector.for_each_inspected<editor_node_id>(
			[&](const editor_node_id node_id) {
				if (const auto parent = find_parent_layer(node_id)) {
					orders[i++].first = { parent->layer_index, parent->index_in_layer }; 
				}
			}
		);
	}
	else if (gui.inspector.inspects_only<editor_layer_id>()) {
		gui.inspector.for_each_inspected<editor_layer_id>(
			[&](const editor_layer_id layer_id) {
				orders[i++].first = { find_layer_index(layer_id), 0 }; 
			}
		);
	}

	sort_range(orders, [](const auto& a, const auto& b) { return a.first < b.first; } );

	gui.inspector.set_from(orders);
}

void editor_setup::inspect_only(inspected_variant new_inspected) {
	gui.inspector.inspect(new_inspected, false);

	inspected_to_entity_selector_state();
	sort_inspected();
}

bool editor_setup::is_inspected(inspected_variant inspected) const {
	return found_in(gui.inspector.all_inspected, inspected);
}

std::vector<inspected_variant> editor_setup::get_all_inspected() const {
	return gui.inspector.all_inspected;
}

editor_node_id editor_setup::get_topmost_inspected_node() const {
	if (const auto id = gui.inspector.get_first_inspected<editor_node_id>()) {
		return *id;
	}

	return {};
}

editor_command_input editor_setup::make_command_input(const bool skip_inspector) {
	return { *this, skip_inspector };
}

void editor_setup::seek_to_revision(const editor_history::index_type n) {
	if (history.get_current_revision() < n) {
		while (history.get_current_revision() < n) {
			redo();
		}

		return;
	}

	if (history.get_current_revision() > n) {
		while (history.get_current_revision() > n) {
			undo();
		}

		return;
	}
}

void editor_setup::undo_quiet() {
	history.undo(make_command_input());
}

bool editor_setup::is_next_command_child() const {
	if (!history.has_next_command()) {
		return false;
	}

	auto get_is_child = [](auto& command) { 
		return command.meta.is_child;
	};

	return std::visit(get_is_child, history.next_command());
}

bool editor_setup::is_last_command_child() const {
	if (!history.has_last_command()) {
		return false;
	}

	auto get_is_child = [](auto& command) { 
		return command.meta.is_child;
	};

	return std::visit(get_is_child, history.last_command());
}

void editor_setup::undo() {
	bool repeat = false;

	do {
		repeat = is_last_command_child();

		const auto prev_inspected = get_all_inspected<editor_node_id>();

		gui.history.scroll_to_current_once = true;
		history.undo(make_command_input());

		gui.filesystem.clear_drag_drop();
		rebuild_scene();

		if (prev_inspected != get_all_inspected<editor_node_id>()) {
			inspected_to_entity_selector_state();
		}
	} while(repeat);
}

void editor_setup::redo() {
	do {
		const auto prev_inspected = get_all_inspected<editor_node_id>();

		gui.history.scroll_to_current_once = true;
		history.redo(make_command_input());

		gui.filesystem.clear_drag_drop();
		rebuild_scene();

		if (prev_inspected != get_all_inspected<editor_node_id>()) {
			inspected_to_entity_selector_state();
		}
	} while(is_next_command_child());
}

void editor_setup::load_gui_state() {
	/*
		To be decided what to do about it.
		Generally ImGui will save the important layouts on its own.
		The identifiers (e.g. currently inspected object id) might become out of date after reloading from json.
	*/
	try {
		view = editor_project_readwrite::read_editor_view(paths.editor_view);
	}
	catch (...) { 
		view = {};
	}
}

void editor_setup::save_gui_state() {
	editor_project_readwrite::write_editor_view(paths.editor_view, view);
	save_last_project_location();
}

void editor_setup::save_last_project_location() {
	augs::save_as_text(get_editor_last_project_path(), paths.project_folder.string());
}

void editor_pathed_resource::maybe_rehash(const augs::path_type& full_path, const augs::file_time_type& fresh_stamp) {
	const auto fresh_stamp_utc = fresh_stamp;

	if (stamp_when_hashed == fresh_stamp_utc && content_hash.size() > 0) {
		return;
	}

	try {
		content_hash = augs::secure_hash(augs::file_to_bytes(full_path));
		stamp_when_hashed = fresh_stamp_utc;
	}
	catch (...) {
		content_hash = "";
	}
}

std::string editor_pathed_resource::get_display_name() const {
	return path_in_project.filename().replace_extension("").string();
}

editor_pathed_resource::editor_pathed_resource(
	const augs::path_type& path_in_project, 
	const std::string& content_hash,
	const augs::file_time_type& stamp
) : 
	path_in_project(path_in_project),
	content_hash(content_hash)
{
	set_hash_stamp(stamp);
}

void editor_pathed_resource::set_hash_stamp(const augs::file_time_type& stamp) {
	stamp_when_hashed = stamp;
}

editor_layer* editor_setup::find_layer(const editor_layer_id& id) {
	return project.layers.pool.find(id);
}

const editor_layer* editor_setup::find_layer(const editor_layer_id& id) const {
	return project.layers.pool.find(id);
}

editor_layer* editor_setup::find_layer(const std::string& name) {
	for (auto& layer : project.layers.pool) {
		if (layer.unique_name == name) {
			return std::addressof(layer);
		}
	}

	return nullptr;
}

const editor_layer* editor_setup::find_layer(const std::string& name) const {
	for (const auto& layer : project.layers.pool) {
		if (layer.unique_name == name) {
			return std::addressof(layer);
		}
	}

	return nullptr;
}

void editor_setup::start_renaming_selection() {
	gui.layers.request_rename = true;
}

std::unordered_map<std::string, editor_node_id> editor_setup::make_name_to_node_map() const {
	std::unordered_map<std::string, editor_node_id> result;

	project.nodes.for_each(
		[&](const auto& node_pool) {
			auto register_node = [&]<typename T>(const auto id, const T& object) {
				result[object.get_display_name()] = editor_typed_node_id<T> { id }.operator editor_node_id();
			};

			node_pool.for_each_id_and_object(register_node);
		}
	);

	return result;
}

std::string editor_setup::get_free_node_name_for(const std::string& new_name) const {
	if (new_name.empty()) {
		return get_free_node_name_for("Unnamed node");
	}

	const auto name_map = make_name_to_node_map();

	auto is_node_name_free = [&](const auto& name) {
		return !found_in(name_map, name);
	};

	return augs::first_free_string(
		new_name,
		" (%x)",
		is_node_name_free
	);
}

std::string editor_setup::get_free_layer_name_for(const std::string& name_pattern) const {
	return augs::first_free_string(
		name_pattern, 
		" %x", 
		[this](const auto& candidate){ return nullptr == find_layer(candidate); }
	);
}

std::string editor_setup::get_free_layer_name() const {
	return get_free_layer_name_for("New layer");
}


void editor_setup::create_new_layer(const std::string& name_pattern) {
	create_layer_command cmd;
	cmd.created_layer.unique_name = get_free_layer_name_for(name_pattern);

	if (const auto best_above = find_best_layer_for_new_node()) {
		cmd.at_index = best_above->layer_index;
	}

	post_new_command(cmd);
}

bool editor_setup::wants_multiple_selection() const {
	return ImGui::GetIO().KeyCtrl;
}

std::size_t editor_setup::find_layer_index(const editor_layer_id id) const {
	return ::find_index_in(project.layers.order, id);
}

std::optional<editor_setup::parent_layer_info> editor_setup::find_parent_layer(const editor_node_id node_id) const {
	if (!node_id.is_set()) {
		return std::nullopt;
	}

	parent_layer_info info;

	const auto& layers = project.layers.order;

	for (std::size_t l = 0; l < layers.size(); ++l) {
		const auto layer_id = layers[l];

		if (const auto layer = find_layer(layer_id)) {
			const auto& nodes = layer->hierarchy.nodes;

			for (std::size_t i = 0; i < nodes.size(); ++i) {
				if (nodes[i] == node_id) {
					return parent_layer_info { layer_id, layer, l, i };
				}
			}
		}
	}

	return std::nullopt;
}

std::string editor_setup::get_name(const entity_id id) const {
	return get_name(to_node_id(id));
}

std::size_t editor_setup::get_node_count() const {
	auto n = std::size_t(0);

	for (const auto layer_id : project.layers.order) {
		if (const auto layer = find_layer(layer_id)) {
			const auto& nodes = layer->hierarchy.nodes;

			n += nodes.size();
		}
	}

	return n;
}

std::string editor_setup::get_name(inspected_variant v) const {
	auto get_object_name = [&]<typename T>(const T& inspected_id) {
		std::string found_name;

		auto name_getter = [&found_name](const auto& object, const auto) {
			found_name = object.get_display_name();
		};

		if constexpr(std::is_same_v<T, editor_node_id>) {
			on_node(inspected_id, name_getter);
		}
		else if constexpr(std::is_same_v<T, editor_resource_id>) {
			on_resource(inspected_id, name_getter);
		}
		else if constexpr(std::is_same_v<T, editor_layer_id>) {
			if (const auto layer = find_layer(inspected_id)) {
				found_name = layer->get_display_name();
			}
		}

		(void)inspected_id;
		return found_name;
	};

	return std::visit(get_object_name, v);
}

entity_id editor_setup::get_hovered_entity(const necessary_images_in_atlas_map& sizes_for_icons, const std::optional<vec2> at) const {
	return selector.calc_hovered_entity(
		scene.world,
		sizes_for_icons,
		get_camera_eye().zoom,
		at ? *at : get_world_cursor_pos(),
		render_layer_filter::all()
	);
}

editor_node_id editor_setup::get_hovered_node() const{ 
	return to_node_id(selector.get_hovered());
}

editor_node_id editor_setup::get_hovered_node(const necessary_images_in_atlas_map& sizes_for_icons, const std::optional<vec2> at) const {
	return to_node_id(get_hovered_entity(sizes_for_icons, at));
}

void editor_setup::scroll_once_to(inspected_variant id) {
	gui.layers.scroll_once_to = id;
}

editor_node_id editor_setup::to_node_id(entity_id id) const {
	if (!id.is_set()) {
		return {};
	}

	const auto& mapping = scene_entity_to_node[id.type_id.get_index()];
	const auto node_index = id.raw.indirection_index;

	if (node_index < mapping.size()) {
		return mapping[node_index];
	}

	return {};
}

augs::path_type editor_setup::get_unofficial_content_dir() const {
	return paths.project_folder;
}

camera_eye editor_setup::get_camera_eye() const {
	return view.panned_camera;
}

void editor_setup::set_zoom(const float zoom) {
	view.panned_camera.zoom = zoom;
}

std::unordered_set<entity_id> editor_setup::get_all_inspected_entities() const {
	auto result = std::unordered_set<entity_id>();

	for_each_inspected_entity(
		[&](const auto id) {
			result.emplace(id);
		}
	);

	return result;
}

template <class F>
void editor_setup::for_each_dashed_line(F&& callback) const {
	const auto& world = scene.world;

	auto draw_dashes_around = [&](const entity_id hovered_id) {
		if (const auto handle = world[hovered_id]) {
			if (const auto tr = handle.find_logic_transform()) {
				/* Draw dashed lines around the selected entity */
				const auto ps = augs::make_rect_points(tr->pos, handle.get_logical_size(), tr->rotation);

				for (std::size_t i = 0; i < ps.size(); ++i) {
					const auto& v = ps[i];
					const auto& nv = wrap_next(ps, i);

					callback(v, nv, settings.entity_selector.hovered_dashed_line_color, 0);
				}
			}
		}
	};

	draw_dashes_around(selector.get_hovered());

	draw_dashes_around(gui.filesystem.entity_to_highlight);
	draw_dashes_around(gui.layers.entity_to_highlight);

	auto dashed_line_handler = [&](const entity_id id) {
		const auto handle = world[id];

		if (handle.dead()) {
			return;
		}

		handle.dispatch_on_having_all<components::light>([&](const auto typed_handle) {
			const auto center = typed_handle.get_logic_transform().pos;

			const auto& light = typed_handle.template get<components::light>();

			const auto light_color = light.color;

			auto draw_reach_indicator = [&](const auto reach, const auto col) {
				callback(center, center + reach / 2, col);

				augs::general_border_from_to(
					ltrb(xywh::center_and_size(center, reach)),
					0,
					[&](const vec2 from, const vec2 to) {
						callback(from, to, col);
					}
				);
			};

			draw_reach_indicator(light.calc_reach_trimmed(), light_color);
			draw_reach_indicator(light.calc_wall_reach_trimmed(), rgba(light_color).mult_alpha(0.7f));
		});

		if (is_mover_active()) {
			handle.dispatch_on_having_all<components::overridden_geo>([&](const auto& typed_handle) {
				const auto s = typed_handle.get_logical_size();
				const auto tr = typed_handle.get_logic_transform();

				const auto& last = history.last_command();

				if (const auto* const cmd = std::get_if<resize_nodes_command>(std::addressof(last))) {
					const auto active = cmd->get_active_edges();
					const auto edges = ltrb::center_and_size(tr.pos, s).make_edges();

					auto draw_edge = [&](auto e) {
						callback(e[0].rotate(tr), e[1].rotate(tr), red, global_time_seconds * 8, true);
					};

					if (active.top) {
						draw_edge(edges[0]);
					}
					if (active.right) {
						draw_edge(edges[1]);
					}
					if (active.bottom) {
						draw_edge(edges[2]);
					}
					if (active.left) {
						draw_edge(edges[3]);
					}
				}
			});
		}
	};

	for_each_inspected_entity(dashed_line_handler);
}

void editor_setup::draw_custom_gui(const draw_setup_gui_input& in) { 
	if (is_playtesting()) {
		return;
	}

	auto cone = in.cone;

	auto& eye = cone.eye;
	eye.transform.pos.discard_fract();

	auto on_screen = [&](const auto p) {
		return cone.to_screen_space(p);
	};

	auto triangles = in.get_drawer();
	auto lines = in.get_line_drawer();
	const auto screen_size = in.screen_size;

	auto& editor_cfg = in.config.editor;

	if (view.show_grid) {
		triangles.grid(
			screen_size,
			view.grid.unit_pixels,
			eye,
			editor_cfg.grid.render
		);
	}

	::for_each_iconed_entity(
		scene.world, 
		in.all_visible,
		in.config.faction_view,

		[&](const auto typed_handle, const auto image_id, const transformr world_transform, const rgba color) {
			const auto screen_space = transformr(vec2i(on_screen(world_transform.pos)), world_transform.rotation);

			const auto is_invalid = image_id == assets::necessary_image_id::INVALID;
			const auto image_size = is_invalid ? vec2u::square(32) : in.necessary_images[image_id].get_original_size();

			const auto blank_tex = triangles.default_texture;
			(void)blank_tex;

			if (auto active_color = find_highlight_color_of(typed_handle.get_id())) {
				active_color->a = static_cast<rgba_channel>(std::min(1.8 * active_color->a, 255.0));

				augs::detail_sprite(
					triangles.output_buffer,
					blank_tex,
					image_size + vec2i(10, 10),
					screen_space.pos,
					screen_space.rotation,
					*active_color
				);

				active_color->a = static_cast<rgba_channel>(std::min(2.2 * active_color->a, 255.0));

				lines.border(
					image_size,
					screen_space.pos,
					screen_space.rotation,
					*active_color,
					border_input { 1, 0 }
				);
			}

			if (is_invalid) {
				using namespace augs::gui::text;

#if 0
				const auto& callout_name = typed_handle.get_name();
#endif
				// TODO: Add an entity type with a callout as component instead of reading this from node name!!!
				const auto& callout_name = get_name(typed_handle.get_id());

				print_stroked(
					triangles,
					on_screen(typed_handle.get_logic_transform().pos),
					formatted_string { callout_name, { in.gui_fonts.gui, white } },
					{ augs::ralign::CX, augs::ralign::CY }
				);
			}
			else {
				augs::detail_sprite(
					triangles.output_buffer,
					in.necessary_images.at(image_id),
					screen_space.pos,
					screen_space.rotation,
					color
				);

				lines.border(
					image_size,
					screen_space.pos,
					screen_space.rotation,
					color,
					border_input { 1, 2 }
				);
			}

			::draw_area_indicator(typed_handle, lines, screen_space, 1.0f, drawn_indicator_type::EDITOR, eye.zoom, color);
		}	
	);

	if (const auto selection_aabb = find_selection_aabb()) {
		auto col = white;

		if (is_mover_active()) {
			col.a = 120;
		}

		triangles.border(
			cone.to_screen_space(*selection_aabb),
			col,
			border_input { 1, -1 }
		);
	}

	for_each_dashed_line(
		[&](vec2 from, vec2 to, const rgba color, const double secs = 0.0, bool fatten = false) {
			const auto a = on_screen(from);
			const auto b = on_screen(to);

			lines.dashed_line(a, b, color, 5.f, 5.f, secs);

			if (fatten) {
				const auto ba = b - a;
				const auto perp = ba.perpendicular_cw().normalize();
				lines.dashed_line(a + perp, b + perp, color, 5.f, 5.f, secs);
				lines.dashed_line(a + perp * 2, b + perp * 2, color, 5.f, 5.f, secs);
			}
		}	
	);

	if (const auto r = find_screen_space_rect_selection(screen_size, in.mouse_pos)) {
		triangles.aabb_with_border(
			*r,
			editor_cfg.rectangular_selection_color,
			editor_cfg.rectangular_selection_border_color
		);
	}
}

void editor_setup::apply(const config_lua_table& cfg) {
	settings = cfg.editor;
	faction_view = cfg.faction_view;
}

void editor_setup::unhover() {
	selector.unhover();
}

void editor_setup::clear_id_caches() {
	// Should we call it every time a node that alters entity existence is called?
	selector.clear();

	// Note that calling in.setup.clear_node_id_caches(); 
	// during create_node_command could break hover detection in selector.
	// Let's maybe always check for selected entities existence everywhere.
	// (anyways it's a massive corner case that someone would ctrl+z/y during a rectangular selection.)
	// Even without rect selection it could show some unrelated entity hovered for a split second, though.
}

entity_selector_input editor_setup::make_selector_input() const {
	return { entity_selector_state, true };
}

std::optional<ltrb> editor_setup::find_selection_aabb() const {
	return selector.find_selection_aabb(scene.world, make_selector_input());
}

std::optional<ltrb> editor_setup::find_screen_space_rect_selection(
	const vec2i screen_size,
	const vec2i mouse_pos
) const {
	return selector.find_screen_space_rect_selection(camera_cone(get_camera_eye(), screen_size), mouse_pos);
}

void editor_setup::finish_rectangular_selection() {
	selector.finish_rectangular(entity_selector_state);
}

void editor_setup::select_all_entities() {
	if (gui.inspector.inspects_only<editor_node_id>()) {
		if (gui.inspector.all_inspected.size() == get_node_count()) {
			clear_inspector();

			for (const auto layer_id : project.layers.order) {
				inspect_add_quiet(layer_id);
			}

			after_quietly_adding_inspected();

			return;
		}
	}

	if (gui.inspector.inspects_any_different_than<editor_node_id>()) {
		clear_inspector();
	}

	bool inspected_any_visible = false;

	std::unordered_set<editor_node_id> nodes;

	auto remember_as_inspected = [&](const editor_node_id& node_id) {
		nodes.emplace(node_id);
	};

	for_each_inspected<editor_node_id>(remember_as_inspected);

	auto inspected = [&](const auto next_node) {
		return found_in(nodes, next_node);
	};

	for (const auto layer_id : reverse(project.layers.order)) {
		auto layer = find_layer(layer_id);
		ensure(layer != nullptr);

		for (const auto node_id : reverse(layer->hierarchy.nodes)) {
			if (is_node_visible(node_id) && !inspected(node_id)) {
				inspected_any_visible = true;

				inspect_add_quiet(node_id);
				remember_as_inspected(node_id);
			}
		}
	}

	if (!inspected_any_visible) {
		for (const auto layer_id : reverse(project.layers.order)) {
			auto layer = find_layer(layer_id);
			ensure(layer != nullptr);

			for (const auto node_id : reverse(layer->hierarchy.nodes)) {
				if (!is_node_visible(node_id) && !inspected(node_id)) {
					inspected_any_visible = true;

					inspect_add_quiet(node_id);
					remember_as_inspected(node_id);
				}
			}
		}
	}

	after_quietly_adding_inspected();
}

void editor_setup::center_view_at_selection() {
	if (const auto aabb = find_selection_aabb()) {
		view.center_at(aabb->get_center());
	}
}

void editor_setup::center_view_at(const editor_node_id node_id) {
	on_node(
		node_id,
		[&](const auto& node, const auto id) {
			(void)id;

			view.center_at(node.editable.pos);
		}
	);
}

entity_id editor_setup::to_entity_id(const editor_node_id node_id) const {
	if (const auto result = on_node(
		node_id,
		[&](const auto& node, const auto id) {
			(void)id;

			return node.scene_entity_id;
		}
	)) {
		return *result;
	}

	return {};
}

node_mover_input editor_setup::make_mover_input() {
	return { *this };
}

setup_escape_result editor_setup::escape() {
	if (is_playtesting()) {
		stop_playtesting();
		return setup_escape_result::JUST_FETCH;
	}

	if (ok_only_popup) {
		ok_only_popup = std::nullopt;
		return setup_escape_result::JUST_FETCH;
	}
	else if (mover.escape()) {
		undo();
		return setup_escape_result::JUST_FETCH;
	}

	return setup_escape_result::LAUNCH_INGAME_MENU;
}

std::optional<rgba> editor_setup::find_highlight_color_of(const entity_id id) const {
	return selector.find_highlight_color_of(
		settings.entity_selector, id, make_selector_input()
	);
}

bool editor_setup::start_moving_selection() {
	return mover.start_moving_selection(make_mover_input());
}

void editor_setup::finish_moving_selection() {
	mover.escape();
}

void editor_setup::show_absolute_mover_pos_once() {
	auto& last = history.last_command();

	if (auto* const cmd = std::get_if<move_nodes_command>(std::addressof(last))) {
		cmd->show_absolute_mover_pos_in_ui = true;
	}
}

void editor_setup::make_last_command_a_child() {
	auto set_is_child = [](auto& command) { 
		command.meta.is_child = true; 
	};

	std::visit(set_is_child, history.last_command());
}

void editor_setup::cut_selection() {
	/* TODO: some clipboard mechanic? */
	delete_selection();
}

void editor_setup::delete_selection() {
	if (gui.inspector.inspects_any<editor_node_id>()) {
		auto command = make_command_from_selected_nodes<delete_nodes_command>("Deleted ");

		if (!command.empty()) {
			post_new_command(std::move(command));
		}
	}
	else if (gui.inspector.inspects_any<editor_layer_id>()) {
		auto layer_command = make_command_from_selected_layers<delete_layers_command>("Deleted ");

		delete_nodes_command nodes_command;
		nodes_command.omit_inspector = true;

		for (auto& entry : layer_command.entries) {
			if (const auto layer = find_layer(entry.layer_id)) {
				for (const auto node_id : layer->hierarchy.nodes) {
					nodes_command.push_entry(node_id);
				}
			}
		}

		if (!nodes_command.empty()) {
			post_new_command(std::move(nodes_command));
			post_new_command(std::move(layer_command));
			make_last_command_a_child();
		}
		else if (!layer_command.empty()) {
			post_new_command(std::move(layer_command));
		}
	}
}

bool editor_setup::register_node_in_layer(const editor_node_id node, const editor_node_id over_node) {
	if (const auto parent = find_parent_layer(over_node)) {
		return register_node_in_layer(node, parent->layer_id, parent->index_in_layer);
	}

	return false;
}

bool editor_setup::register_node_in_layer(const editor_node_id node_id, const editor_layer_id layer_id, const std::size_t index_in_layer) {
	if (auto* const layer = find_layer(layer_id)) {
		auto& nodes = layer->hierarchy.nodes;

		if (index_in_layer == static_cast<std::size_t>(-1)) {
			nodes.emplace_back(node_id);
		}
		else {
			nodes.insert(nodes.begin() + index_in_layer, node_id);
		}

		layer->is_open = true;
		return true;
	}

	return false;
}

void editor_setup::unregister_node_from_layer(const editor_node_id node_id) {
	if (const auto parent = find_parent_layer(node_id)) {
		unregister_node_from_layer(node_id, parent->layer_id);
	}
}

void editor_setup::unregister_node_from_layer(const editor_node_id node_id, const editor_layer_id layer_id) {
	if (auto* const layer = find_layer(layer_id)) {
		erase_element(layer->hierarchy.nodes, node_id);
	}
}

void inspect_command::undo(const editor_command_input in) {
	in.setup.inspect_only(inspected_before);
}

void inspect_command::redo(const editor_command_input in) {
	in.setup.inspect_only(to_inspect);
}

void editor_setup::move_inspected_to_new_layer() {
	const auto all_inspected = get_all_inspected<editor_node_id>();

	if (all_inspected.empty()) {
		create_new_layer();
		return;
	}

	reorder_nodes_command command;

	command.create_layer = create_layer_command();
	command.create_layer->created_layer.unique_name = get_free_layer_name();
	command.nodes_to_move = all_inspected;
	{
		const auto parent_of_topmost = find_parent_layer(all_inspected[0]);
		ensure(parent_of_topmost != std::nullopt);
		command.create_layer->at_index = parent_of_topmost->layer_index;
	}

	if (all_inspected.size() > 1) {
		command.built_description = typesafe_sprintf("Moved %x nodes to a new layer", all_inspected.size());
	}
	else {
		command.built_description = typesafe_sprintf("Moved %x to a new layer", get_name(all_inspected[0]));
	}

	post_new_command(std::move(command));
}

void editor_setup::move_dragged_to_new_layer(const editor_node_id dragged_node) {
	const auto all_inspected = get_all_inspected<editor_node_id>();

	reorder_nodes_command command;

	command.create_layer = create_layer_command();
	command.create_layer->created_layer.unique_name = get_free_layer_name();
	{
		const auto parent_of_topmost = find_parent_layer(all_inspected[0]);
		ensure(parent_of_topmost != std::nullopt);
		command.create_layer->at_index = parent_of_topmost->layer_index;
	}

	if (is_inspected(dragged_node) && all_inspected.size() > 1) {
		command.nodes_to_move = all_inspected;
		command.built_description = typesafe_sprintf("Moved %x nodes to a new layer", all_inspected.size());
	}
	else {
		command.nodes_to_move = { dragged_node };
		command.built_description = typesafe_sprintf("Moved %x to a new layer", get_name(dragged_node));
	}

	post_new_command(std::move(command));
}

static auto peel_duplicate_suffix(std::string s) {
	return cut_trailing_number_and_spaces(s);
}

void editor_setup::mirror_selection(const vec2i direction, const bool move_if_only_duplicate) {
	const bool only_duplicating = direction.is_zero();
	gui.filesystem.clear_drag_drop();

	if (gui.inspector.inspects_only<editor_layer_id>()) {
		bool first = true;
		bool any_nonempty = false;

		const auto before_inspected = get_all_inspected<editor_layer_id>();

		std::vector<inspected_variant> all_new_nodes;
		std::vector<inspected_variant> all_new_layers;

		// In case the commands in progress modify the inspector,
		// just to be sure, save the inspection result beforehand.
		const auto all_source_layers = get_all_inspected<editor_layer_id>();
		const auto dupli_or_mirr = std::string(only_duplicating ? "Duplicated " : "Mirrored ");
		const auto final_description = all_source_layers.size() == 1 
			? dupli_or_mirr + get_name(all_source_layers[0])
			: typesafe_sprintf("%x%x layers", dupli_or_mirr, all_source_layers.size())
		;

		auto for_each_source_node_id = [&](auto callback) { 
			for (const auto layer_id : all_source_layers) {
				if (const auto source_layer = find_layer(layer_id)) {
					for (const auto source_node : source_layer->hierarchy.nodes) {
						callback(source_node);
					}
				}
			}
		};

		const auto custom_aabb = find_aabb_of_nodes(for_each_source_node_id);

		for (const auto layer_id : all_source_layers) {
			if (const auto source_layer = find_layer(layer_id)) {
				duplicate_nodes_command duplicate;
				duplicate.mirror_direction = direction;
				duplicate.omit_inspector = true;
				duplicate.custom_aabb = custom_aabb;

				for (const auto source_node : source_layer->hierarchy.nodes) {
					duplicate.push_entry(source_node);
				}

				create_layer_command new_layer;
				new_layer.created_layer = *source_layer;
				new_layer.created_layer.hierarchy.nodes.clear();
				new_layer.created_layer.unique_name = get_free_layer_name_for(peel_duplicate_suffix(source_layer->unique_name));
				new_layer.at_index = find_layer_index(layer_id);
				new_layer.omit_inspector = true;

				{
					const auto& executed = post_new_command(std::move(new_layer));
					const auto new_layer_id = executed.get_created_id();

					if (!first) {
						make_last_command_a_child();
					}

					duplicate.target_new_layer = new_layer_id;

					all_new_layers.emplace_back(new_layer_id);
				}

				if (!duplicate.empty()) {
					any_nonempty = true;

					const auto& executed = post_new_command(std::move(duplicate));
					concatenate(all_new_nodes, executed.get_all_duplicated());
					make_last_command_a_child();
				}

				first = false;
			}
		}

		{
			inspect_command inspect;

			if (!all_new_nodes.empty()) {
				inspect.to_inspect = std::move(all_new_nodes);
			}
			else {
				inspect.to_inspect = std::move(all_new_layers);
			}

			assign_begin_end(inspect.inspected_before, before_inspected);

			post_new_command(std::move(inspect));
			make_last_command_a_child();
		}

		if (any_nonempty) {
			if (move_if_only_duplicate && only_duplicating) {
				if (start_moving_selection()) {
					make_last_command_a_child();
				}
			}
		}

		if (history.has_last_command()) {
			if (auto cmd = std::get_if<move_nodes_command>(&history.last_command())) {
				cmd->built_description = final_description;
			}

			if (auto cmd = std::get_if<inspect_command>(&history.last_command())) {
				cmd->built_description = final_description;
			}
		}
	}
	else if (gui.inspector.inspects_only<editor_node_id>()) {
		auto command = make_command_from_selected_nodes<duplicate_nodes_command>(
			only_duplicating ? "Duplicated " : "Mirrored ",
			only_visible_nodes()
		);

		std::optional<std::pair<editor_layer_id, std::size_t>> found_parent;
		bool found_different = false;

		for_each_inspected<editor_node_id>(
			[&](const editor_node_id node_id) {
				if (const auto this_parent = find_parent_layer(node_id)) {
					if (found_parent == std::nullopt) {
						found_parent = { this_parent->layer_id, this_parent->index_in_layer };
					}

					if (this_parent->layer_id != found_parent->first) {
						found_different = true;
					}
				}
			}
		);

		if (found_parent && !found_different) {
			command.target_unified_location = *found_parent;
			command.reverse_order();
		}

		if (!command.empty()) {
			command.mirror_direction = direction;
			post_new_command(std::move(command));

			if (move_if_only_duplicate && only_duplicating) {
				if (start_moving_selection()) {
					make_last_command_a_child();
				}
			}
		}
	}
}

void editor_setup::duplicate_selection(bool start_moving) {
	mirror_selection(vec2i(0, 0), start_moving);
}

bool editor_setup::is_node_visible(const editor_node_id id) const {
	if (const auto found_layer = find_parent_layer(id)) {
		if (found_layer->layer_ptr) {
			if (!found_layer->layer_ptr->visible) {
				return false;
			}
		}
	}
	else {
		return false;
	}

	bool visible = false;

	on_node(id, [&visible](const auto& typed_node, const auto node_id) {
		(void)node_id;

		if (typed_node.visible) {
			visible = true;
		}
	});

	return visible;
}

double editor_setup::get_interpolation_ratio() const {
	if (is_playtesting()) {
		return timer.fraction_of_step_until_next_step(get_viewed_cosmos().get_fixed_delta().in_seconds<double>());
	}

	return global_time_seconds / get_inv_tickrate();
}

entity_id editor_setup::get_viewed_character_id() const {
	return viewed_character_id;
}

std::optional<editor_setup::parent_layer_info> editor_setup::find_best_layer_for_new_node() const {
	return std::visit(
		[&]<typename T>(const T& inspected_id) -> std::optional<editor_setup::parent_layer_info> {
			if constexpr(std::is_same_v<T, editor_layer_id>) {
				return convert_to_parent_layer_info(inspected_id);
			}
			else if constexpr(std::is_same_v<T, editor_node_id>) {
				return find_parent_layer(inspected_id);
			}
			else {
				return std::nullopt;
			}
		},
		gui.inspector.get_last_inspected_layer_or_node()
	);
}

std::optional<editor_setup::parent_layer_info> editor_setup::convert_to_parent_layer_info(const editor_layer_id layer_id) const {
	parent_layer_info info;

	const auto& layers = project.layers.order;

	for (std::size_t l = 0; l < layers.size(); ++l) {
		if (layer_id == layers[l]) {
			if (const auto layer = find_layer(layer_id)) {
				return parent_layer_info { layer_id, layer, l, 0 };
			}
		}
	}

	return std::nullopt;
}

void editor_setup::quiet_set_last_inspected_layer_or_node(const inspected_variant inspected) {
	gui.inspector.last_inspected_layer_or_node = inspected;
}

void editor_setup::quiet_set_last_inspected_any(const inspected_variant inspected) {
	gui.inspector.last_inspected_any = inspected;
}

void editor_setup::set_inspector_tab(const inspected_node_tab_type type) {
	gui.inspector.node_current_tab = type;
}

void editor_setup::start_rotating_selection() {
	mover.start_rotating_selection(make_mover_input()); 
}

void editor_setup::start_resizing_selection(const bool two_edges, resize_nodes_command::active_edges custom_edges) {
	mover.start_resizing_selection(make_mover_input(), two_edges, custom_edges); 
}

void editor_setup::rotate_selection_once_by(const int degrees) {
	mover.rotate_selection_once_by(make_mover_input(), degrees);
}

void editor_setup::flip_selection_horizontally() {
	mover.flip_selection(make_mover_input(), flip_flags::make_horizontally());
}

void editor_setup::flip_selection_vertically() {
	mover.flip_selection(make_mover_input(), flip_flags::make_vertically());
}

void editor_setup::reset_rotation_of_selected_nodes() {
	mover.reset_rotation(make_mover_input());
}

void editor_setup::toggle_grid() {
	view.toggle_grid();
}

void editor_setup::toggle_snapping() {
	view.toggle_snapping();
}

void editor_setup::clamp_units() {
	view.grid.clamp_units(8, settings.grid.render.get_maximum_unit());
}

void editor_setup::sparser_grid() {
	view.grid.increase_grid_size();
	clamp_units();
}

void editor_setup::denser_grid() {
	view.grid.decrease_grid_size();
	clamp_units();
}

int editor_setup::get_current_grid_size() const {
	return view.grid.unit_pixels;
}

bool editor_setup::is_grid_enabled() const {
	return view.show_grid;
}

bool editor_setup::is_snapping_enabled() const {
	return view.snapping_enabled;
}

node_mover_op editor_setup::get_current_node_transforming_op() const {
	return mover.get_current_op(history);
}

void editor_setup::reset_zoom() {
	view.panned_camera.zoom = 1.f;
}

bool editor_setup::is_view_centered_at_selection() const {
	if (const auto aabb = find_selection_aabb()) {
		auto pos = aabb->get_center();
		return view.panned_camera.transform.pos == pos.discard_fract();
	}

	return true;
}

void editor_setup::start_playtesting() {
	if (is_playtesting()) {
		return;
	}

	total_collected.clear();

	mode = test_mode();
	auto& cosm = scene.world;
	local_player_id = mode.add_player({ default_test_ruleset, cosm }, faction_type::METROPOLIS);

	const auto new_character = cosm[mode.lookup(local_player_id)];

	const auto spawn_transform = get_camera_eye().transform;

	const auto mouse_dir = (get_world_cursor_pos() - spawn_transform.pos).normalize();
	new_character.dispatch_on_having_all<components::sentience>([&](const auto& typed_handle) {
		typed_handle.set_logic_transform(spawn_transform);
		::snap_interpolated_to(typed_handle, spawn_transform);

		if (const auto crosshair = typed_handle.find_crosshair()) {
			crosshair->base_offset = mouse_dir * 100;
		}
	});

	viewed_character_id = new_character.get_id();
}

void editor_setup::stop_playtesting() {
	viewed_character_id.unset();
	rebuild_scene();
}

bool editor_setup::is_playtesting() const {
	return viewed_character_id.is_set();
}

void editor_setup::accept_game_gui_events(const game_gui_entropy_type& events) {
	control(events);
}

std::optional<camera_eye> editor_setup::find_current_camera_eye() const {
	if (is_playtesting() && gui.playtest_immersive) {
		return std::nullopt;
	}

	return get_camera_eye();
}

template struct edit_resource_command<editor_sprite_resource>;
template struct edit_resource_command<editor_sound_resource>;
template struct edit_resource_command<editor_light_resource>;
template struct edit_resource_command<editor_material_resource>;
template struct edit_resource_command<editor_particles_resource>;
template struct edit_resource_command<editor_wandering_pixels_resource>;
template struct edit_resource_command<editor_point_marker_resource>;
template struct edit_resource_command<editor_area_marker_resource>;

template struct edit_resource_command<editor_firearm_resource>;
template struct edit_resource_command<editor_ammunition_resource>;
template struct edit_resource_command<editor_melee_resource>;
template struct edit_resource_command<editor_explosive_resource>;

template struct edit_node_command<editor_sprite_node>;
template struct edit_node_command<editor_sound_node>;
template struct edit_node_command<editor_light_node>;
template struct edit_node_command<editor_particles_node>;
template struct edit_node_command<editor_wandering_pixels_node>;
template struct edit_node_command<editor_point_marker_node>;
template struct edit_node_command<editor_area_marker_node>;

template struct edit_node_command<editor_firearm_node>;
template struct edit_node_command<editor_ammunition_node>;
template struct edit_node_command<editor_melee_node>;
template struct edit_node_command<editor_explosive_node>;

template struct create_node_command<editor_sprite_node>;
template struct create_node_command<editor_sound_node>;
template struct create_node_command<editor_light_node>;
template struct create_node_command<editor_particles_node>;
template struct create_node_command<editor_wandering_pixels_node>;
template struct create_node_command<editor_point_marker_node>;
template struct create_node_command<editor_area_marker_node>;

template struct create_node_command<editor_firearm_node>;
template struct create_node_command<editor_ammunition_node>;
template struct create_node_command<editor_melee_node>;
template struct create_node_command<editor_explosive_node>;
