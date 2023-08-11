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
#include "application/setups/editor/commands/create_resource_command.hpp"
#include "application/setups/editor/commands/edit_node_command.hpp"
#include "application/setups/editor/commands/edit_project_settings_command.hpp"
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
#include "application/setups/editor/packaged_official_content.h"

#include "application/setups/editor/official/create_official_resources.h"
#include "application/setups/editor/official/create_official_prefabs.h"
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
#include "augs/window_framework/window.h"

#include "application/setups/editor/commands/replace_whole_project_command.hpp"

#include "application/setups/editor/editor_official_resource_map.hpp"
#include "application/setups/editor/defaults/editor_resource_defaults.h"
#include "application/arena/arena_handle.h"
#include "augs/readwrite/json_readwrite_errors.h"

#include "application/arena/build_arena_from_editor_project.hpp"
#include "game/cosmos/for_each_entity.h"
#include "game/detail/passes_filter.h"

render_layer_filter get_layer_filter_for_miniature();

template <class R, class F>
void packaged_official_content::for_each_resource(F callback) {
	resources.template get_pool_for<R>().for_each_id_and_object(
		[&](const auto& raw_id, const auto& object) {
			const bool official = true;
			const auto typed_id = editor_typed_resource_id<R>::from_raw(raw_id, official);

			callback(typed_id, object);
		}
	);
}

packaged_official_content::packaged_official_content(sol::state& lua) {
	built_content.populate_official_content(
		lua,
		60
	);

	::create_official_resources(built_content, resources);

	auto map_with_tag = [&](const auto id, auto& obj) {
		ensure(obj.official_tag.has_value());

		if (obj.official_tag) {
			resource_map[*obj.official_tag] = id;
		}
	};

	auto map_with_type = [&](const auto id, auto& obj) {
		resource_map[obj.editable.type] = id;
	};

	resources.pools.for_each_container(
		[&]<typename P>(const P&) {
			using R = typename P::mapped_type;

			if constexpr(!is_one_of_v<R, editor_prefab_resource, editor_game_mode_resource>) {
				if constexpr(is_one_of_v<R, editor_area_marker_resource, editor_point_marker_resource>) {
					for_each_resource<R>(map_with_type);
				}
				else {
					for_each_resource<R>(map_with_tag);
				}
			}
		}
	);

	create_official_prefabs();
	for_each_resource<editor_prefab_resource>(map_with_type);

	auto find_lambda = [&](auto id) {
		return resources.find_typed(id);
	};

	::setup_resource_defaults_after_creating_officials(find_lambda, resource_map);
}

editor_setup::editor_setup(
	const editor_settings& settings,
	const packaged_official_content& official,
	const augs::path_type& project_path
) : settings(settings), last_autosave_settings(settings.autosave), official(official), paths(project_path) {
	create_official_filesystems();

	LOG("Loading editor project at: %x", project_path);

	constexpr bool convert_legacy_autosave = true;

	if (convert_legacy_autosave) {
		if (augs::exists(paths.legacy_autosave_json)) {
			if (augs::exists(paths.project_json)) {
				std::filesystem::rename(paths.project_json, paths.last_saved_json);
			}

			std::filesystem::rename(paths.legacy_autosave_json, paths.project_json);

			simple_popup new_autosave_popup;

			new_autosave_popup.title = "SUCCESS";
			new_autosave_popup.warning_notice_above = "Converted legacy autosave.json filenames.";
			new_autosave_popup.message = "No further action is required.";

			autosave_popup = new_autosave_popup;
		}
	}

	auto try_read_saved_revision_from = [&](const auto& path) {
		try {
			project = editor_project_readwrite::read_project_json(
				path,
				official.resources,
				official.resource_map
			);

			/* Will be overwritten to the new name upon the first save. */
			project.meta.name = paths.arena_name;

			history.mark_revision_as_saved();

			rescan_physical_filesystem();

			return true;
		}
		catch (const augs::json_deserialization_error& err) {
			throw augs::json_deserialization_error("(%x):\n%x", path.filename().string(), err.what());
		}
		catch (...) {
			LOG("No %x file was found.", path.string());
		}

		return false;
	};

	auto try_read_autosave_revision_from = [&](const auto& path) {
		try {
			auto autosaved_project = std::make_unique<editor_project>(editor_project_readwrite::read_project_json(
				path,
				official.resources,
				official.resource_map
			));

			autosaved_project->meta.name = paths.arena_name;

			simple_popup new_autosave_popup;
			new_autosave_popup.title = "WARNING";

			replace_whole_project_command cmd;
			cmd.after = std::move(autosaved_project);
			cmd.before = std::make_unique<editor_project>(project);
			cmd.built_description = std::string("Loaded autosave from ") + path.filename().string();

			if (settings.autosave.if_loaded_autosave_show == editor_autosave_load_option::LAST_SAVED_VERSION) {
				new_autosave_popup.warning_notice_above = "Autosaved changes are available in: " + path.filename().string();

				new_autosave_popup.message = "You're now on the last saved version from: " + paths.last_saved_json.filename().string() + ".\n";
				new_autosave_popup.message += "To load the autosave, press Redo (CTRL+SHIFT+Z).\n\nIt is best practice to save your work (CTRL+S) before exiting the game.";
			}
			else {
				new_autosave_popup.warning_notice_above = "Loaded autosaved changes from " + path.filename().string() + ".";

				new_autosave_popup.message = "To go back to last saved changes instead,\npress Undo (CTRL+Z).\n\nIt is best practice to save your work (CTRL+S) before exiting the game.";

			}

			if (!autosave_popup.has_value()) {
				if (settings.autosave.alert_when_loaded_autosave) {
					autosave_popup = new_autosave_popup;
				}
			}

			post_new_command(std::move(cmd));
			history.mark_revision_as_autosaved();
			autosave_timer.reset();

			/*
				We need this so that the autosave file isn't removed 
				when the user exits the editor on a saved revision without explicitly saving it.
			*/

			dirty_after_loading_autosave = true;
			//recent_message.set("Loaded an autosave file.\nTo go back to last saved changes instead,\npress Undo (CTRL+Z).");
			//recent_message.show_for_at_least_ms = 10000;

			return true;
		}
		catch (const augs::json_deserialization_error& err) {
			throw augs::json_deserialization_error("(%x):\n%x", path.filename().string(), err.what());
		}
		catch (...) {
			LOG("No autosaved changes were found.");
		}

		return false;
	};

	auto do_on_project_assigned = [&]() {
		/*
			If we load autosave, this is already called in replace_whole_project_command (which calls assign_project).
			If we don't load autosave, we need to manually call this for the current project.

			Autosave will be triggered here if any redirects happen. That's not a bad thing,
			because if we called this, it means no autosave exists.
		*/

		const bool undoing_to_first_revision = false;
		on_project_assigned(undoing_to_first_revision);
	};

	if (try_read_saved_revision_from(paths.last_saved_json)) {
		if (!try_read_autosave_revision_from(paths.project_json)) {
			do_on_project_assigned();
		}
	}
	else if (try_read_saved_revision_from(paths.project_json)) {
		do_on_project_assigned();
	}
	else {
		/* At least one of either project.json or last_saved.json must exist. */
		throw augs::json_deserialization_error("Error: %x not found.", paths.project_json.string());
	}

	load_gui_state();
	open_default_windows();

	rebuild_arena();
	save_last_project_location();

	if (settings.autosave.if_loaded_autosave_show == editor_autosave_load_option::LAST_SAVED_VERSION) {
		undo();
	}
}

editor_setup::~editor_setup() {
	autosave_now_if_needed();

	/*
		Restore last_saved.json to project.json only if we're at saved revision.

		Note we're "dirty" if we've just loaded autosave and the user hasn't yet saved any revision to decide which one they want.
		We're also "dirty" if we haven't saved after auto-redirecting pathed resources.

		In these cases, last_saved.json will not be restored even if we're at a saved revision in history.
	*/

	if (everything_completely_saved()) {
		restore_last_saved_json();
	}

	LOG("DTOR finished: ~editor_setup");
}

void editor_setup::create_official_filesystems() {
	::create_official_filesystem_from(
		official.built_content,
		official.resources,
		official_files_root
	);

	gui.filesystem.rebuild_official_special_filesystem(*this);
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

	if (in.e.msg == message::activate || in.e.msg == message::click_activate) {
		on_window_activate();
	}

	if (in.e.msg == message::deactivate) {
		if (settings.autosave.on_lost_focus) {
			autosave_now_if_needed();
		}
	}

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

	return false;
}

bool editor_setup::confirm_modal_popup() {
	if (autosave_popup) {
		autosave_popup = std::nullopt;
		return true;
	}
	else if (invalid_filenames_popup) {
		invalid_filenames_popup = std::nullopt;
		return true;
	}
	else if (redirect_or_missing_popup) {
		redirect_or_missing_popup = std::nullopt;
		return true;
	}

	return false;
}

void editor_setup::prepare_for_online_playtesting() {
	autosave_now_if_needed();
}

void editor_setup::request_online_playtesting() {
	imgui_return_once = custom_imgui_result::PLAYTEST_ONLINE;
}

bool editor_setup::handle_input_before_game(
	handle_input_before_game_input in
) {
	using namespace augs::event;
	using namespace augs::event::keys;

	if (arena_gui_base::handle_input_before_game(in)) {
		return true;
	}

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
					case key::E: in.window.reveal_in_explorer(paths.project_json); return true;
					case key::Z: redo(); return true;
					default: break;
				}
			}

			switch (k) {
				case key::A: select_all_entities(); return true;
				case key::Z: undo(); return true;
				case key::P: request_online_playtesting(); return true;
				case key::D: cut_selection(); return true;
				case key::S: save(); return true;
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
				case key::U: inspect_project_settings(); return true;
				default: break;
			}
		}

		if (has_shift && !has_ctrl) {
			switch (k) {
				case key::N: move_inspected_to_new_layer(); return true;
				case key::R: rotate_selection_once_by(-90); return true;
				case key::H: flip_selection_horizontally(); return true;
				case key::V: flip_selection_vertically(); return true;
				case key::T: 
					if (!settings.warp_cursor_when_moving_nodes) {
						warp_cursor_to_center(in.window);
					}

					start_moving_selection(); 

					return true;

				default: break;
			}
		}

		if (!has_shift && !has_ctrl) {
			switch (k) {
				case key::F2: start_renaming_selection(); return true;

				case key::N: create_new_layer(); return true;

				case key::U: gui.request_toggle_sounds_preview = true; return true;

				case key::C: clone_selection(); return true;
				case key::DEL: delete_selection(); return true;

				case key::T: 
					if (settings.warp_cursor_when_moving_nodes) {
						warp_cursor_to_center(in.window);
					}

					start_moving_selection(); 

					return true;

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
	config.window.name = typesafe_sprintf("Hypersomnia Editor - %x", get_arena_name_with_star());

	if (is_playtesting()) {
		get_arena_handle().adjust(config.drawing);
	}
	else {
		config.drawing.auto_zoom = false;
		config.drawing.custom_zoom = 1.0f;

		config.drawing.draw_area_markers.is_enabled = false;
		config.drawing.draw_aabb_highlighter = false;
		config.interpolation.enabled = false;
		
		config.sound.allow_sounds_without_character_listener = gui.sounds_preview;

		config.sound.processing_frequency = sound_processing_frequency::EVERY_SINGLE_FRAME;
	}

	if (miniature_generator.has_value()) {
		config.drawing.draw_aabb_highlighter = false;
		config.interpolation.enabled = false;
		config.drawing.draw_area_markers.is_enabled = false;
		config.drawing.draw_callout_indicators.is_enabled = false;
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

void editor_setup::autosave_if_redirected(
	const editor_paths_changed_report& report,
	const bool during_activate,
	const bool undoing_to_first_revision
) {
	if (report.redirects.empty()) {
		return;
	}

	/*
		Corner case:

		We do not want to overwrite the current autosave in case we undo/redo back to the saved revision
		and it happens to automatically redirect some resources.

		However what if this is caused not by a seek to a saved revision
		but just an alt-tab while we are already at a saved revision?
			In that case we probably want to produce a fresh autosave.
			The current autosave would've been removed anyway if there had been no redirect intead and the game quit.

		Both of the above could happen during the "autosave dilemma"
		(which is the situation when there is only the load autosave command)

		(report_changed_paths is only called on assign_project and on_window_activate, no other case

		Now what if autosave revision gets ctrl+s'd and seeking to it causes a redirect?
			Should be autosaved all-right
				Now what if we undo back to the - now unsaved - 0 revision and it causes a redirect?
					well, this one naturally becomes the autosaved revision on next launch

		
		
		THEREFORE
		
		The only moment we skip autosave here is when we're on the 0 revision and it's still a saved but DIRTY revision.
		dirty_after_loading_autosave implies the 0 revision is a saved one and dirty (because no other save has yet been made). 

		Note it will also be nice to preserve it when there's more commands above the autosave command, because why not?
	*/

	/*
	   	Note history.is_revision_oldest() will mistakenly report true while it is redoing from oldest to autosave revision.
		But if we're on this oldest revision, we need to skip the autosave during window activate as well.
	*/

	const bool due_to_activate_on_oldest = during_activate && history.is_revision_oldest();
	const bool due_to_undo_to_oldest = undoing_to_first_revision;

	const bool preserve_current_autosave = 
		dirty_after_loading_autosave
		&& (due_to_activate_on_oldest || due_to_undo_to_oldest);

	if (preserve_current_autosave) {
		LOG("Preserving current autosave despite redirects.\nThis is the oldest revision, and the loaded autosave has not yet been saved explicitly, so there's no point to replace that autosave.");
	}
	else {
		LOG("Autosaving due to redirects.");
		force_autosave();
	}
}

void editor_setup::on_window_activate() {
	rescan_physical_filesystem();

	const bool during_activate = true;
	const bool undoing_to_first_revision = false;

	const auto result = rebuild_pathed_resources();

	if (is_playtesting()) {
		if (result.any()) {
			stop_playtesting();
		}
	}

	report_changed_paths(result);	
	autosave_if_redirected(result, during_activate, undoing_to_first_revision);

	if (!is_playtesting()) {
		rebuild_arena();
	}
}

void editor_setup::rescan_physical_filesystem() {
	last_invalid_paths.clear();

	auto should_skip_sanitization = [&](auto path) {
		return paths.is_project_specific_file(path.make_preferred());
	};

	files.rebuild_from(paths.project_folder, should_skip_sanitization, last_invalid_paths);
	gui.filesystem.clear_drag_drop();
	rebuild_ad_hoc_atlas = true;

	if (last_invalid_paths.size() > 0) {
		simple_popup ignored_popup;

		std::size_t can_be_renamed_num = 0;

		for (const auto& r : last_invalid_paths) {
			if (r.can_be_renamed()) {
				++can_be_renamed_num;
			}
		}

		std::string summary;
		std::string details;

		if (can_be_renamed_num > 0) {
			summary = typesafe_sprintf("%x files/folders have invalid filenames.\n%x can be renamed automatically.\n\nIf you cancel, they will be ignored until you name them correctly.\nIf they were in the project before, they will be reported as missing.", last_invalid_paths.size(), can_be_renamed_num == last_invalid_paths.size() ? std::string("All") : std::to_string(can_be_renamed_num));
		}
		else {
			summary = typesafe_sprintf("%x files/folders have invalid filenames.\nThey cannot be renamed automatically.\n\nThey will be ignored until you name them correctly.\nIf they were in the project before, they will be reported as missing.", last_invalid_paths.size());
		}

		for (const auto& r : last_invalid_paths) {
			details += r.forbidden_path.string();

			if (r.can_be_renamed()) {
				details += std::string(" ->\n") + r.get_suggested_path().string();
			}
			else {
				details += "\n";
			   	details	+= sanitization::describe(r.reason);
			}

			details += "\n\n";
		}

		ignored_popup.title = "Invalid filenames detected";
		ignored_popup.message = summary;
		ignored_popup.details = details;

		invalid_filenames_popup = ignored_popup;
	}
	else {
		invalid_filenames_popup = std::nullopt;
	}
}

void editor_setup::report_changed_paths(const editor_paths_changed_report& changes) {
	if (!changes.any()) {
		return;
	}

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
		details += typesafe_sprintf("\n%x ->\n%x\n", r.first.string(), r.second.string());
	}

	changes_popup.title = "Detected changes in filesystem";
	changes_popup.message = summary;
	changes_popup.details = details;

	if (changes.redirects.size() > 0) {
		changes_popup.warning_notice = "Please save the project to remember the redirected paths.";
		dirty_after_redirecting_paths = true;
	}

	redirect_or_missing_popup = changes_popup;
}

void editor_setup::rebuild_project_internal_resources_gui() {
	gui.filesystem.rebuild_project_special_filesystem(*this);
}

void editor_setup::on_project_assigned(const bool undoing_to_first_revision) {
	const bool during_activate = false;
	const auto result = rebuild_pathed_resources();

	redirect_or_missing_popup = std::nullopt;
	report_changed_paths(result);	
	autosave_if_redirected(result, during_activate, undoing_to_first_revision);

	rebuild_project_internal_resources_gui();
}

void editor_setup::assign_project(const editor_project& new_project, const bool undoing_to_first_revision) {
	project = new_project;
	on_project_assigned(undoing_to_first_revision);
}

augs::path_type editor_setup::resolve_project_path(const augs::path_type& path_in_project) const {
	return paths.resolve(path_in_project);
}

/*
	Corner cases:

	If someone moves a file AND modifies it too, it will simply be considered missing.
	Then every time you activate the window the editor will try redirecting the deleted files.

	But you should still be able to clear all references to said resource and forcefully forget it.
*/

editor_paths_changed_report editor_setup::rebuild_pathed_resources() {
	auto& rebuilt_project = project;

	editor_paths_changed_report changes;
	rebuild_ad_hoc_atlas = true;

	/*
		Before allocating a resource, we want to first check if one exists with this content hash,
		and with own path no longer existing. Only then do we redirect.

		Otherwise we could end up with duplicate resources.
	*/

	auto existing_paths = std::unordered_set<std::string>();

	auto register_resources_found_on_disk = [&](editor_filesystem_node& file) {
		if (file.sanitization_skipped) {
			return;
		}

		const auto path_in_project = file.get_path_in_project().string();
		existing_paths.emplace(path_in_project);
	};

	files.root.for_each_file_recursive(register_resources_found_on_disk);

	rebuilt_project.last_unbacked_resources.clear();

	auto handle_pool = [&]<typename P>(P& pool, const editor_filesystem_node_type type) {
		using resource_type = typename P::value_type;
		using id_type = editor_typed_resource_id<resource_type>;

		/*
			Note we don't update this map when we redirect
			(by changing paths in resource properties, essentially rendering resource_by_path map incorrect).
			That is because paths are guaranteed to be unique.
				i.e. once we redirect to a editor_filesystem_node&, we wont meet one with the same path again.

			Hashes don't change during this process.
		*/

		std::unordered_map<std::string,     std::vector<id_type>> resources_by_hash;
		std::unordered_map<augs::path_type, id_type> resource_by_path;

		auto map_all_resources = [&](const auto id, const auto& entry) {
			const auto& r = entry.external_file;

			entry.found_on_disk = found_in(existing_paths, r.path_in_project.string());

			resources_by_hash[r.file_hash].push_back(id);
			resource_by_path[r.path_in_project]    = id;
		};

		auto add_if_new_or_redirect = [&](editor_filesystem_node& file) {
			if (file.type != type) {
				return;
			}

			if (file.sanitization_skipped) {
				return;
			}

			const auto path_in_project = file.get_path_in_project();
			const auto full_path = resolve_project_path(path_in_project);

			auto match_path_to_existing_resource = [&]() {
				if (const auto found_resource_id = mapped_or_nullptr(resource_by_path, path_in_project)) {
					if (const auto found_resource = find_resource(*found_resource_id)) {
						found_resource->external_file.maybe_rehash(full_path, file.last_write_time);
						file.associated_resource = found_resource_id->operator editor_resource_id();

						return true;
					}
				}

				return false;
			};

			auto redirect_by_hash_or_register_new = [&]() {
				std::string new_resource_hash;

				try {
					new_resource_hash = augs::to_hex_format(augs::secure_hash(augs::file_to_bytes(full_path)));
				}
				catch (...) {
					LOG("WARNING! Couldn't get a hash from %x", full_path);
					return;
				}

				auto redirect_by_hash = [&]() {
					/*
						Note that if we match with an existing resource to be redirected,
						if it's the same hash it will *very likely* have the same extension.
						So we should look by stems only.
					*/

					const auto searched_stem = full_path.stem();

					auto get_stem = [&](const auto& of) {
						return of.external_file.path_in_project.stem();
					};

					if (const auto maybe_moved_resources = mapped_or_nullptr(resources_by_hash, new_resource_hash)) {
						auto most_resembling_resource_idx = [&]() -> std::optional<std::size_t> {
							const auto& moved_resources = *maybe_moved_resources;

							if (moved_resources.empty()) {
								return std::nullopt;
							}

							/*
								First see if there is any missing one with a matching stem.
							*/

							for (std::size_t i = 0; i < moved_resources.size(); ++i) {
								const auto& candidate = *find_resource(moved_resources[i]);

								if (candidate.unbacked_on_disk() && searched_stem == get_stem(candidate)) {
									return i;
								}
							}

							/*
								If none found, it must have been renamed in addition to being moved, 
								so take whichever comes.
							*/

							for (std::size_t i = 0; i < moved_resources.size(); ++i) {
								const auto& candidate = *find_resource(moved_resources[i]);

								if (candidate.unbacked_on_disk()) {
									return i;
								}
							}

							/* 
								None of the resources with the given hash are missing.
								Register a new resource.
							*/

							return std::nullopt;
						}();

						if (most_resembling_resource_idx == std::nullopt) {
							return false;
						}

						auto& moved_resources = *maybe_moved_resources;
						auto& moved_resource = *find_resource(moved_resources[*most_resembling_resource_idx]);

						const auto& new_path = path_in_project;
						auto& moved = moved_resource.external_file;

						changes.redirects.emplace_back(moved.path_in_project, new_path);

						moved.path_in_project = new_path;
						moved.set_hash_stamp(file.last_write_time);

						const auto moved_id = pool.get_id_of(moved_resource);

						file.associated_resource.set<resource_type>(moved_id, false);

						std::swap(moved_resources.back(), moved_resources[*most_resembling_resource_idx]);
						moved_resources.pop_back();

						moved_resource.found_on_disk = true;

						return true;
					}

					return false;
				};

				auto register_new_resource = [&]() {
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

					::setup_resource_defaults(new_resource.editable, official.resource_map);

					file.associated_resource.set<resource_type>(new_id, false);
					new_resource.found_on_disk = true;
				};

				if (!redirect_by_hash()) {
					register_new_resource();
				}
			};

			/* In case it's missing */

			file.associated_resource = {};

			if (!match_path_to_existing_resource()) {
				redirect_by_hash_or_register_new();
			}
		};

		auto check_if_unbacked = [&](const auto id, const auto& entry) {
			if (entry.unbacked_on_disk()) {
				/* 
					If the file is still missing after all redirecting, 
					then it is indeed unbacked.
				*/

				rebuilt_project.last_unbacked_resources.push_back(id.operator editor_resource_id());
			}
		};

		rebuilt_project.for_each_resource<resource_type>(map_all_resources);
		files.root.for_each_file_recursive(add_if_new_or_redirect);
		rebuilt_project.for_each_resource<resource_type>(check_if_unbacked);
	};

	auto& sprite_pool = rebuilt_project.resources.pools.template get_for<editor_sprite_resource>();
	auto& sound_pool  = rebuilt_project.resources.pools.template get_for<editor_sound_resource>();

	handle_pool(sprite_pool, editor_filesystem_node_type::IMAGE);
	handle_pool(sound_pool,  editor_filesystem_node_type::SOUND);

	const auto num_unbacked = rebuilt_project.last_unbacked_resources.size();

	if (num_unbacked > 0) {
		LOG("%x resources are unbacked (*potentially* missing), so they'll be rescanned after every command.", num_unbacked);
	}
	else {
		LOG("All resources ever seen during this session are still on disk.");
	}

	/*
		We'll need to know if any resources we actually reference are now missing.
	*/

	rescan_missing_pathed_resources(std::addressof(changes.missing));

	return changes;
}

void editor_setup::on_resource_references_changed() {
	should_recount_internal_resource_references = true;
	rescan_missing_pathed_resources();
}

void editor_setup::rescan_missing_pathed_resources(std::vector<augs::path_type>* const out_report) { 
	auto& rebuilt_project = project;
	rebuilt_project.last_missing_resources.clear();

	if (rebuilt_project.last_unbacked_resources.empty()) {
		/* There will be none regardless of reference counts. */
		return;
	}

	// LOG("RESCAN_MISSING_RESOURCES");

	struct entry {
		std::string path;
		editor_resource_id id;

		bool operator<(const entry& b) const {
			return augs::natural_order(path, b.path);
		}
	};

	std::vector<entry> missing_entries;

	rebuilt_project.rescan_resources_to_track(official.resources, official.resource_map);

	auto check_missing = [&]<typename R>(const R& resource, const editor_typed_resource_id<R> id) {
		if constexpr(is_pathed_resource_v<R>) {
			if (resource.unbacked_on_disk() && resource.should_be_tracked()) {
				// LOG("MISSING: %x", resource.external_file.path_in_project);

				missing_entries.push_back({ 
					resource.external_file.path_in_project.string(), 
					id.operator editor_resource_id() 
				});
			}
		}
	};

	for (auto& unbacked : rebuilt_project.last_unbacked_resources) {
		on_resource(unbacked, check_missing);
	}

	sort_range(missing_entries);

	if (out_report) {
		for (auto& e : missing_entries) {
			out_report->push_back(e.path);
		}
	}

	for (auto& e : missing_entries) {
		rebuilt_project.last_missing_resources.push_back(e.id);
	}

	// LOG_NVPS(rebuilt_project.last_missing_resources.size(), rebuilt_project.last_unbacked_resources.size());
}

void editor_setup::remove_last_saved_json() {
	augs::remove_file(paths.last_saved_json);
}

void editor_setup::restore_last_saved_json() {
	if (augs::exists(paths.last_saved_json)) {
		std::filesystem::rename(paths.last_saved_json, paths.project_json);
	}
}

void editor_setup::save() {
	history.mark_revision_as_saved();
	autosave_timer.reset();

	dirty_after_loading_autosave = false;
	dirty_after_redirecting_paths = false;

	save_project_file_as(paths.project_json);

	recent_message.set("Saved the project to %x", paths.project_json.filename().string());

	remove_last_saved_json();
}

void editor_setup::save_project_file_as(const augs::path_type& path) {
	LOG("Saving project file as: %x", path);

	project.meta.version_timestamp = augs::date_time::get_utc_timestamp();

	if (project.about.author.empty()) {
		project.about.author = std::string(simulated_client.nickname);
	}

	editor_project_readwrite::write_project_json(
		path,
		project,
		official.resources,
		official.resource_map
	);
}

bool editor_setup::is_dirty() const {
	return dirty_after_loading_autosave || dirty_after_redirecting_paths;
}

bool editor_setup::everything_completely_saved() const {
	return !is_dirty() && history.at_saved_revision();
}

bool editor_setup::has_unsaved_changes() const {
	return !everything_completely_saved();
}

std::string editor_setup::get_arena_name() const {
	return std::string(paths.arena_name);
}

std::string editor_setup::get_arena_name_with_star() const {
	if (has_unsaved_changes()) {
		return get_arena_name() + "*";
	}

	return get_arena_name();
}

bool editor_setup::autosave_needed() const {
	/*
		The reason we don't need to check for if it's dirty is because
		once it becomes dirty for whatever reason, we force autosave.
	*/

	return !history.at_saved_revision() && !history.at_autosaved_revision();
}

void editor_setup::force_autosave() {
	bool just_backed_up_message = false;

	if (!augs::exists(paths.last_saved_json)) {
		std::filesystem::rename(paths.project_json, paths.last_saved_json);
		just_backed_up_message = true;
	}

	save_project_file_as(paths.project_json);

	if (just_backed_up_message) {
		recent_message.set("Autosaved current changes to: %x\nPrevious version backup at: %x", paths.project_json.filename().string(), paths.last_saved_json.filename().string());
	}
	else {
		recent_message.set("Autosaved current changes to: %x", paths.project_json.filename().string());
	}

	history.mark_revision_as_autosaved();
	autosave_timer.reset();
}

void editor_setup::autosave_now_if_needed() {
	if (autosave_needed()) {
		LOG("Not on either autosaved or saved revision. Forcing autosave.");
		force_autosave();
	}

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

void editor_setup::inspect_project_settings(const bool scroll) {
	inspect_only(inspected_special::PROJECT_SETTINGS);

	if (scroll) {
		scroll_once_to(inspected_special::PROJECT_SETTINGS);
	}
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
	return { *this, settings, skip_inspector };
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
	while (can_undo()) {
		const bool repeat = is_last_command_child();

		const auto prev_inspected = get_all_inspected<editor_node_id>();

		gui.history.scroll_to_current_once = true;

		bool should_rebuild = true;
		bool should_rescan_missing = true;

		auto check_rebuild = [&]<typename T>(const T&) {
			if constexpr(skip_scene_rebuild_v<T>) {
				should_rebuild = false;
			}

			if constexpr(skip_missing_resources_check_v<T>) {
				should_rescan_missing = false;
			}
		};

		std::visit(check_rebuild, history.last_command());
		history.undo(make_command_input());

		gui.filesystem.clear_drag_drop();

		if (should_rebuild) {
			rebuild_arena();
		}

		if (should_rescan_missing) {
			on_resource_references_changed();
		}

		if (prev_inspected != get_all_inspected<editor_node_id>()) {
			inspected_to_entity_selector_state();
		}

		if (!repeat) {
			break;
		}
	}
}

void editor_setup::redo() {
	while (can_redo()) {
		const auto prev_inspected = get_all_inspected<editor_node_id>();

		gui.history.scroll_to_current_once = true;

		bool should_rebuild = true;
		bool should_rescan_missing = true;

		auto check_rebuild = [&]<typename T>(const T&) {
			if constexpr(skip_scene_rebuild_v<T>) {
				should_rebuild = false;
			}

			if constexpr(skip_missing_resources_check_v<T>) {
				should_rescan_missing = false;
			}
		};

		std::visit(check_rebuild, history.next_command());

		history.redo(make_command_input());

		gui.filesystem.clear_drag_drop();

		/*
			Ok... apparently we need to call these for all intermediate commands
			because our node mover is still kinda legacy
			and operates on entities instead of nodes
		*/

		if (should_rebuild) {
			rebuild_arena();
		}

		if (should_rescan_missing) {
			on_resource_references_changed();
		}

		if (prev_inspected != get_all_inspected<editor_node_id>()) {
			inspected_to_entity_selector_state();
		}
		
		if (!is_next_command_child()) {
			break;
		}
	}
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

	if (stamp_when_hashed == fresh_stamp_utc && file_hash.size() > 0) {
		return;
	}

	try {
		file_hash = augs::to_hex_format(augs::secure_hash(augs::file_to_bytes(full_path)));
		stamp_when_hashed = fresh_stamp_utc;
	}
	catch (...) {
		file_hash = "";
	}
}

std::string editor_pathed_resource::get_display_name() const {
	return path_in_project.stem().string();
}

editor_pathed_resource::editor_pathed_resource(
	const augs::path_type& path_in_project, 
	const std::string& file_hash,
	const augs::file_time_type& stamp
) : 
	path_in_project(path_in_project),
	file_hash(file_hash)
{
	set_hash_stamp(stamp);
}

void editor_pathed_resource::set_hash_stamp(const augs::file_time_type& stamp) {
	stamp_when_hashed = stamp;
}

editor_layer* editor_setup::find_layer(const editor_layer_id& id) {
	return project.find_layer(id);
}

const editor_layer* editor_setup::find_layer(const editor_layer_id& id) const {
	return project.find_layer(id);
}

editor_layer* editor_setup::find_layer(const std::string& name) {
	return project.find_layer(name);
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
	if (gui.inspector.inspects_only<editor_resource_id>()) {
		gui.filesystem.request_rename = true;
	}
	else {
		gui.layers.request_rename = true;
	}
}

name_to_node_map_type editor_setup::make_name_to_node_map() const {
	return project.make_name_to_node_map();
}

std::unordered_map<std::string, editor_resource_id> editor_setup::make_name_to_internal_resource_map() const {
	std::unordered_map<std::string, editor_resource_id> result;

	project.resources.for_each(
		[&]<typename P>(const P& resource_pool) {
			using R = typename P::mapped_type;

			if constexpr(!is_pathed_resource_v<R>) {
				auto register_resource = [&]<typename T>(const auto id, const T& object) {
					result[object.get_display_name()] = editor_typed_resource_id<T> { id }.operator editor_resource_id();
				};

				resource_pool.for_each_id_and_object(register_resource);
			}
		}
	);

	return result;
}

std::string editor_setup::get_free_internal_resource_name_for(const std::string& new_name) const {
	if (new_name.empty()) {
		return get_free_internal_resource_name_for("Unnamed resource");
	}

	const auto name_map = make_name_to_internal_resource_map();

	auto is_resource_name_free = [&](const auto& name) {
		return !found_in(name_map, name);
	};

	return augs::first_free_string(
		new_name,
		" (%x)",
		is_resource_name_free
	);
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
	const bool is_default = name_pattern == "Layer";
	return augs::first_free_string(
		name_pattern, 
		" %x", 
		[this](const auto& candidate){ return nullptr == find_layer(candidate); },
		is_default,
		is_default ? 1 : 0
	);
}

std::string editor_setup::get_free_layer_name() const {
	return get_free_layer_name_for("Layer");
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

std::optional<parent_layer_info> editor_setup::find_parent_layer(const editor_node_id node_id) const {
	return project.find_parent_layer(node_id);
}

std::string editor_setup::get_name(const entity_id id) const {
	return get_name(to_node_id(id));
}

std::size_t editor_setup::get_project_pathed_resource_count() const {
	auto n = std::size_t(0);

	project.resources.pools.for_each_container(
		[&]<typename P>(const P& pool) {
			using R = typename P::mapped_type;

			if constexpr(is_pathed_resource_v<R>) {
				n += pool.size();
			}
		}
	);

	return n;
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
	gui.filesystem.scroll_once_to = id;

	if (auto node = std::get_if<editor_node_id>(std::addressof(id))) {
		if (auto parent = find_parent_layer(*node)) {
			if (auto layer = find_layer(parent->layer_id)) {
				layer->is_open = true;
			}
		}
	}
}

editor_node_id editor_setup::to_node_id(entity_id id) const {
	return ::entity_to_node_id(scene_entity_to_node, id);
}

augs::path_type editor_setup::get_unofficial_content_dir() const {
	return paths.project_folder;
}

camera_eye editor_setup::get_camera_eye() const {
	if (miniature_generator.has_value()) {
		return miniature_generator->get_current_camera();
	}

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
	arena_gui_base::draw_custom_gui(in);

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

		[&](const auto typed_handle, const auto image_id, const transformr world_transform, rgba color) {
			/* It's better if we see all icons clearly */
			color.a = 255;

			const auto screen_space = transformr(vec2i(on_screen(world_transform.pos)), world_transform.rotation);

			{
				bool is_prefab_child = false;

				auto parent_node = to_node_id(typed_handle.get_id());

				on_node(
					parent_node,
					[&]<typename N>(const N& node, const auto) {
						if constexpr(std::is_same_v<N, editor_prefab_node>) {
							/* 
								Prefab's scene_entity_id points to the anchor, i.e. itself. 
								Therefore it's an unselectable child if entity points to the prefab node but prefab points to something else.
							*/

							is_prefab_child = node.scene_entity_id != typed_handle.get_id();
						}
					}
				);

				if (is_prefab_child) {
					return;
				}
			}

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

void editor_setup::draw_custom_gui_over_imgui(const draw_setup_gui_input& in) {
	draw_recent_message(in);
}

void editor_setup::draw_recent_message(const draw_setup_gui_input& in) {
	const auto& cfg = settings.action_notification;

	if (!cfg.enabled) {
		return;
	}

	const auto& h = history;

	using namespace augs::gui::text;
	using O = augs::history_op_type;

	const auto& fnt = in.gui_fonts.gui;

	auto colored = [&](const auto& s, const rgba col = white) {
		const auto st = style(fnt, col);
		return formatted_string(s, st);
	};

	auto make_colorized = [&](auto dest) {
		formatted_string result;

		auto try_preffix = [&](const auto& preffix, const auto col) {
			if (begins_with(dest, preffix)) {
				cut_preffix(dest, preffix);
				result = colored(preffix, col) + colored(dest);
				return true;
			}

			return false;
		};

		if (try_preffix("Deleted", red)
			|| try_preffix("Erased", red)
			|| try_preffix("Removed", red)
			|| try_preffix("Cannot", red)
			|| try_preffix("Discarded", orange)
			|| try_preffix("Reapplied", pink)
			|| try_preffix("Showing", green)
			|| try_preffix("Successfully", green)
			|| try_preffix("Filled", green)
			|| try_preffix("Exported", green)
			|| try_preffix("Written", green)
			|| try_preffix("Saved the project", green)
			|| try_preffix("Loaded autosaved changes.", green)
			|| try_preffix("Saved", green)
			|| try_preffix("Loaded", green)
			|| try_preffix("Opened", green)
			|| try_preffix("Altered", yellow)
			|| try_preffix("Grouped", yellow)
			|| try_preffix("Ungrouped", orange)
			|| try_preffix("Set", green)
			|| try_preffix("Restored defaults", green)
			|| try_preffix("Reset", green)
			|| try_preffix("Unset", green)
			|| try_preffix("Switched", green)
			|| try_preffix("Edited", green)
			|| try_preffix("Enabled", green)
			|| try_preffix("Disabled", gray)
			|| try_preffix("Hiding", gray)
			|| try_preffix("Selected", yellow)
			|| try_preffix("Moved", green)
			|| try_preffix("Rotated", green)
			|| try_preffix("Flipped", green)
			|| try_preffix("Resized", green)
			|| try_preffix("Renamed", green)
			|| try_preffix("Changed", green)
			|| try_preffix("Created", green)
			|| try_preffix("Autosaved", green)
			|| try_preffix("Started", green)				
			|| try_preffix("Started tracking", green)				
			|| try_preffix("Cloned", cyan)
			|| try_preffix("Mirrored", cyan)
			|| try_preffix("NAT detection", orange)
		) {
			return result;
		}

		return colored(dest);
	};

	augs::date_time considered_stamp;
	formatted_string message_text;
	auto considered_limit = cfg.show_for_ms;

	if (const auto op = h.get_last_op(); op.stamp > recent_message.stamp) {
		considered_stamp = op.stamp;

		auto get_description = [&](const auto& from_command) -> decltype(auto) {
			return std::visit(
				[&](const auto& typed_command) -> decltype(auto) {
					const auto description = typed_command.describe();

					if (op.type == O::EXECUTE_NEW) {
						if (begins_with(description, "Moved")
							|| begins_with(description, "Rotated")
							|| begins_with(description, "Resized")
						) {
							return colored("");
						}
					}

					return make_colorized(description);

				},
				from_command
			);
		};

		const auto description = [&]() {
			if (op.type == O::UNDO) {
				if (h.has_next_command()) {
					const auto preffix = colored("Undo ", orange);
					const auto& cmd = h.next_command();

					return preffix + get_description(cmd);
				}
			}
			else if (op.type == O::REDO) {
				if (h.has_last_command()) {
					const auto preffix = colored("Redo ", gray);
					const auto& cmd = h.last_command();

					return preffix + get_description(cmd);
				}
			}
			else if (op.type == O::EXECUTE_NEW) {
				if (h.has_last_command()) {
					const auto& cmd = h.last_command();

					return get_description(cmd);
				}

			}

			return colored("Unknown op type");
		}();

		if (description.size() > 0) {
			message_text = colored(typesafe_sprintf("#%x: ", 1 + h.get_current_revision())) + description;
		}
	}
	else {
		message_text = make_colorized(recent_message.content);
		considered_stamp = recent_message.stamp;
		considered_limit = std::max(recent_message.show_for_at_least_ms, considered_limit);
	}

	if (message_text.size() > 0) {
		const auto passed = considered_stamp.seconds_ago();
		const auto limit = considered_limit  / 1000;

		if (passed <= limit) {
			/* TODO: (LOW) Improve granularity to milliseconds */

			const auto ss = in.screen_size;
			auto rb_space = cfg.offset;

			if (is_playtesting()) {
				rb_space.y = std::max(rb_space.y, 180);
			}

			const auto text_padding = cfg.text_padding;
			const auto wrapping = cfg.max_width;

			const auto bbox = get_text_bbox(message_text, wrapping, false);
			//const auto line_h = static_cast<int>(fnt.metrics.get_height());
			//bbox.y = std::max(bbox.y, line_h * 2);

			const auto& out = in.get_drawer();

			const auto bbox_padded = bbox + text_padding * 2;
			const auto rect_pos = vec2i(ss.x/2, ss.y) - vec2i(0, rb_space.y) - vec2i(bbox_padded.x / 2, bbox_padded.y);
			const auto text_pos = rect_pos + text_padding;
			const auto rect_size = bbox + text_padding * 2;
			const auto rect = xywh(rect_pos, rect_size);

			out.aabb_with_border(rect, cfg.bg_color, cfg.bg_border_color);

			print_stroked(
				out,
				text_pos,
				message_text,
				augs::ralign_flags {},
				black,
				wrapping
			);
		}
	}
}

void editor_setup::apply(const config_lua_table& cfg) {
	settings = cfg.editor;
	faction_view = cfg.faction_view;
	simulated_client = cfg.client;
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

	bool inspected_any_active = false;

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
			if (is_node_active(node_id) && !inspected(node_id)) {
				inspected_any_active = true;

				inspect_add_quiet(node_id);
				remember_as_inspected(node_id);
			}
		}
	}

	if (!inspected_any_active) {
		for (const auto layer_id : reverse(project.layers.order)) {
			auto layer = find_layer(layer_id);
			ensure(layer != nullptr);

			for (const auto node_id : reverse(layer->hierarchy.nodes)) {
				if (!is_node_active(node_id) && !inspected(node_id)) {
					inspected_any_active = true;

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
		if (arena_gui.escape()) {
			return setup_escape_result::JUST_FETCH;
		}

		stop_playtesting();
		return setup_escape_result::JUST_FETCH;
	}

	if (autosave_popup) {
		autosave_popup = std::nullopt;
		return setup_escape_result::JUST_FETCH;
	}
	else if (invalid_filenames_popup) {
		invalid_filenames_popup = std::nullopt;
		return setup_escape_result::JUST_FETCH;
	}
	else if (redirect_or_missing_popup) {
		redirect_or_missing_popup = std::nullopt;
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
			layer_command.built_description += typesafe_sprintf(" (with %x nodes)", nodes_command.size());
			nodes_command.built_description = layer_command.built_description;

			post_new_command(std::move(nodes_command));
			post_new_command(std::move(layer_command));
			make_last_command_a_child();
		}
		else if (!layer_command.empty()) {
			layer_command.built_description += " (empty)";

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
		ensure(parent_of_topmost.has_value());
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
		ensure(parent_of_topmost.has_value());
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

static auto peel_clone_suffix(std::string s) {
	return cut_trailing_number_and_spaces(s);
}

void editor_setup::mirror_selection(const vec2i direction, const bool move_if_only_clone) {
	if (gui.inspector.empty()) {
		return;
	}

	const bool only_cloning = direction.is_zero();
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
		const auto dupli_or_mirr = std::string(only_cloning ? "Cloned " : "Mirrored ");
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
				clone_nodes_command clone;
				clone.mirror_direction = direction;
				clone.omit_inspector = true;
				clone.custom_aabb = custom_aabb;

				for (const auto source_node : source_layer->hierarchy.nodes) {
					clone.push_entry(source_node);
				}

				create_layer_command new_layer;
				new_layer.created_layer = *source_layer;
				new_layer.created_layer.hierarchy.nodes.clear();
				new_layer.created_layer.unique_name = get_free_layer_name_for(peel_clone_suffix(source_layer->unique_name));
				new_layer.at_index = find_layer_index(layer_id);
				new_layer.omit_inspector = true;

				{
					const auto& executed = post_new_command(std::move(new_layer));
					const auto new_layer_id = executed.get_created_id();

					if (!first) {
						make_last_command_a_child();
					}

					clone.target_new_layer = new_layer_id;

					all_new_layers.emplace_back(new_layer_id);
				}

				if (!clone.empty()) {
					any_nonempty = true;

					const auto& executed = post_new_command(std::move(clone));
					concatenate(all_new_nodes, executed.get_all_cloned());
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
			if (move_if_only_clone && only_cloning) {
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
		auto command = make_command_from_selected_nodes<clone_nodes_command>(
			only_cloning ? "Cloned " : "Mirrored ",
			only_active_nodes()
		);

		auto description = command.built_description;

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

			if (move_if_only_clone && only_cloning) {
				if (start_moving_selection()) {
					make_last_command_a_child();

					if (history.has_last_command()) {
						if (auto cmd = std::get_if<move_nodes_command>(&history.last_command())) {
							cmd->built_description = description;
						}
					}
				}
			}
		}
	}
}

void editor_setup::clone_selection(bool start_moving) {
	mirror_selection(vec2i(0, 0), start_moving);
}

bool editor_setup::is_node_active(const editor_node_id id) const {
	if (const auto found_layer = find_parent_layer(id)) {
		if (found_layer->layer_ptr) {
			if (!found_layer->layer_ptr->is_active()) {
				return false;
			}
		}
	}
	else {
		return false;
	}

	bool active = false;

	on_node(id, [&active](const auto& typed_node, const auto node_id) {
		(void)node_id;

		if (typed_node.active) {
			active = true;
		}
	});

	return active;
}

double editor_setup::get_interpolation_ratio() const {
	if (is_playtesting()) {
		return timer.next_step_progress_fraction(get_viewed_cosmos().get_fixed_delta().in_seconds<double>());
	}

	return global_time_seconds / get_inv_tickrate();
}

std::optional<parent_layer_info> editor_setup::find_best_layer_for_new_node() const {
	return std::visit(
		[&]<typename T>(const T& inspected_id) -> std::optional<parent_layer_info> {
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

std::optional<parent_layer_info> editor_setup::convert_to_parent_layer_info(const editor_layer_id layer_id) const {
	return project.convert_to_parent_layer_info(layer_id);
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

void editor_setup::set_inspector_tab(const inspected_project_tab_type type) {
	gui.inspector.project_current_tab = type;
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

	if (view.show_grid) {
		recent_message.set("Showing grid (%x px)", get_current_grid_size());
	}
	else {
		recent_message.set("Hiding grid (%x px)", get_current_grid_size());
	}
}

void editor_setup::toggle_snapping() {
	view.toggle_snapping();

	if (view.snapping_enabled) {
		recent_message.set("Enabled grid snapping (%x px)", get_current_grid_size());
	}
	else {
		recent_message.set("Disabled grid snapping (%x px)", get_current_grid_size());
	}
}

void editor_setup::clamp_units() {
	view.grid.clamp_units(8, settings.grid.render.get_maximum_unit());
}

void editor_setup::sparser_grid() {
	view.grid.increase_grid_size();
	clamp_units();

	recent_message.set("Set grid size to %x px", get_current_grid_size());
}

void editor_setup::denser_grid() {
	view.grid.decrease_grid_size();
	clamp_units();

	recent_message.set("Set grid size to %x px", get_current_grid_size());
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

	bool needs_rebuild = false;

	if (project.settings.include_disabled_nodes) {
		needs_rebuild = true;
	}

	if (auto mode = project.get_game_modes().find(project.playtesting.mode.raw)) {
		if (mode->editable.common.activate_layers.size() > 0) {
			needs_rebuild = true;
		}

		if (mode->editable.common.deactivate_layers.size() > 0) {
			needs_rebuild = true;
		}
	}

	if (needs_rebuild) {
		rebuild_arena(false);
	}

	recent_message.set("Playtesting offline. Press ESC to quit.");

	arena_gui.choose_team.show = false;

	playtesting = true;

	clean_round_state = scene.world.get_solvable().significant;

	total_collected.clear();

	auto& cosm = scene.world;

	get_arena_handle().on_mode_with_input([&]<typename M>(M& mode, const auto& input) {
		if constexpr(std::is_same_v<M, test_mode>) {
			mode.playtesting_context = make_playtesting_context();
			local_player_id = mode.add_player(input, simulated_client.nickname, project.playtesting.starting_faction);
		}
		else {
			local_player_id = mode.add_player(input, simulated_client.nickname);
			mode.choose_faction(input, local_player_id, project.playtesting.starting_faction);
		}
	});

	cosm.request_resample();
}

void editor_setup::stop_playtesting() {
	playtesting = false;
	rebuild_arena();

	recent_message.set("Ended playtesting session.");

	/* Would interrupt the sounds which would be unpleasant to the ear */
	//scene.world.request_resample();
}

bool editor_setup::is_playtesting() const {
	return playtesting;
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

bool editor_setup::can_undo() const {
	return !history.is_revision_oldest();
}

bool editor_setup::can_redo() const {
	return !history.empty() && !history.is_revision_newest();
}

bool editor_setup::can_resize_nodes() const {
	return true;
}

bool editor_setup::can_transform_nodes() const {
	return true;
}

void editor_setup::warp_cursor_to_center(augs::window& window) {
	const auto screen_center = window.get_screen_size() / 2;
	ImGui::GetIO().MousePos = ImVec2(screen_center);
	window.set_cursor_pos(screen_center);
	warp_cursor_once = true;
}

bool editor_setup::can_resize_selected_nodes() const {
	bool any_can = false;
	bool all_can = true;

	for_each_inspected<editor_node_id>(
		[&](const editor_node_id node_id) {
			const auto id = to_entity_id(node_id);
			const auto handle = scene.world[id];

			const bool can = handle && handle.can_resize();

			if (can) {
				any_can = true;
			}

			if (!can) {
				all_can = false;
			}
		}
	);

	return any_can;
	//return any_can && all_can;
}

bool editor_setup::should_warp_cursor_before_cloning() const {
	if (inspects_only<editor_layer_id>()) {
		const bool inspecting_any_layers_with_nodes = [&]() {
			const auto all_source_layers = get_all_inspected<editor_layer_id>();

			for (const auto layer_id : all_source_layers) {
				if (const auto source_layer = find_layer(layer_id)) {
					if (source_layer->hierarchy.nodes.size() > 0) {
						return true;
					}
				}
			}

			return false;
		}();

		return inspecting_any_layers_with_nodes;
	}

	return true;
}

void editor_setup::toggle_sounds_preview() {
	gui.sounds_preview = !gui.sounds_preview;
}

template <class F>
void editor_setup::rebuild_prefab_nodes(
	const editor_typed_node_id<editor_prefab_node> prefab_node_id,
	F&& on_created_child,
	const bool call_reverse
) {
	const auto prefab_node = find_node(prefab_node_id);

	if (prefab_node == nullptr) {
		return;
	}

	auto find_res_lbd = [&](auto&&... args) -> decltype(auto) { 
		return find_resource(std::forward<decltype(args)>(args)... ); 
	};

	::rebuild_prefab_nodes(
		*prefab_node,
		find_res_lbd,
		std::forward<F>(on_created_child),
		call_reverse
	);
}

void unpack_prefab_command::redo(editor_command_input in) {
	create_cmds.clear();

	const bool do_inspector = true;

	if (do_inspector) {
		in.setup.clear_inspector();
	}

	const auto parent_layer = in.setup.find_parent_layer(prefab_id.operator editor_node_id());

	if (parent_layer == std::nullopt) {
		return;
	}

	auto rebuilder = [&]<typename T>(const T& created_child) {
		create_node_command<T> cmd;

		cmd.created_node = created_child;
		cmd.layer_id = parent_layer->layer_id;
		cmd.index_in_layer = parent_layer->index_in_layer;
		cmd.omit_inspector = true;

		cmd.redo(in);

		if (do_inspector) {
			in.setup.inspect_add_quiet(cmd.get_node_id());
		}

		create_cmds.emplace_back(std::move(cmd));
	};

	in.setup.rebuild_prefab_nodes(prefab_id, rebuilder);

	delete_nodes_command cmd;
	cmd.push_entry(prefab_id.operator editor_node_id());
	cmd.omit_inspector = true;

	del_prefab_cmd = std::move(cmd);
	del_prefab_cmd.redo(in);

	if (do_inspector) {
		in.setup.after_quietly_adding_inspected();
	}
}

void unpack_prefab_command::undo(editor_command_input in) {
	const bool do_inspector = true;

	del_prefab_cmd.undo(in);

	for (auto& c : reverse(create_cmds)) {
		std::visit([&](auto& typed_cmd) { typed_cmd.undo(in); }, c);
	}

	if (do_inspector) {
		in.setup.inspect_only(prefab_id.operator editor_node_id());
	}
}

void editor_setup::unpack_prefab(const editor_typed_node_id<editor_prefab_node> prefab_id) {
	const auto prefab = find_node(prefab_id);

	if (prefab == nullptr) {
		return;
	}

	unpack_prefab_command cmd;
	cmd.prefab_id = prefab_id;
	cmd.built_description = typesafe_sprintf("Unpacked prefab: %x", prefab->get_display_name());

	post_new_command(std::move(cmd));
}

editor_arena_handle<false> editor_setup::get_arena_handle() {
	return get_arena_handle_impl<editor_arena_handle<false>>(*this);
}

editor_arena_handle<true> editor_setup::get_arena_handle() const {
	return get_arena_handle_impl<editor_arena_handle<true>>(*this);
}

const intercosm& editor_setup::get_built_official_content() const {
	return official.built_content;
}

const editor_resource_pools& editor_setup::get_official_resources() const { return official.resources; }
editor_resource_pools& editor_setup::get_mut_official_resources() { return official.resources; }

const editor_official_resource_map& editor_setup::get_official_resource_map() const {
	return official.resource_map;
}

const editor_resource_pools& official_get_resources(const packaged_official_content& official) {
	return official.resources;
}

const editor_official_resource_map& official_get_resource_map(const packaged_official_content& official) {
	return official.resource_map;
}

arena_playtesting_context editor_setup::make_playtesting_context() const {
	return { 
		get_camera_eye().transform.pos,
		project.playtesting.starting_faction
	};
}

#include "application/main/game_frame_buffer.h"
#include "augs/graphics/renderer_backend.h"

void editor_setup::after_all_drawcalls(game_frame_buffer& write_buffer) {
	if (miniature_generator.has_value()) {
		miniature_generator->request_screenshot(write_buffer.renderers.all[renderer_type::GENERAL]);
	}
}

void editor_setup::do_game_main_thread_synced_op(renderer_backend_result& result) {
	if (miniature_generator.has_value()) {
		auto& ss = result.result_screenshot;

		if (ss.has_value()) {
			miniature_generator->acquire(*ss);
		}
	}
}

augs::maybe<render_layer_filter> editor_setup::get_render_layer_filter() const {
	if (miniature_generator.has_value()) {
		return get_layer_filter_for_miniature();
	}

	return render_layer_filter::disabled();
}

render_layer_filter get_layer_filter_for_miniature() {
	auto result = render_layer_filter();
	auto& l = result.layers;

	for (auto i = render_layer::GROUND; i <= render_layer::DIM_WANDERING_PIXELS; i = render_layer(int(i) + 1)) {
		l[i] = true;
	}

	return result;
}

void editor_setup::request_arena_screenshot(const augs::path_type& output_path, const int max_size, const bool reveal) {
	const auto filter = get_layer_filter_for_miniature();

	const auto& cosm = get_viewed_cosmos();

	auto for_each_target = [&](auto combiner) {
		cosm.for_each_entity(
			[&](const auto& typed_entity) {
				if (auto sprite = typed_entity.template find<components::sprite>()) {
					if (sprite->colorize.a == 0) {
						return;
					}
				}

				if (auto sprite = typed_entity.template find<invariants::sprite>()) {
					if (sprite->color.a == 0) {
						return;
					}
				}

				if (filter.passes(typed_entity)) {
					combiner(typed_entity);
				}
			}
		);
	};

	if (const auto world_captured_region = ::find_aabb_of(cosm, for_each_target)) {
		miniature_generator_state request;
		auto cam = find_current_camera_eye();

		request.output_path = output_path;
		request.world_captured_region = *world_captured_region;

		if (const bool is_miniature = !reveal) {
			const auto bigger_side = request.world_captured_region.get_size().bigger_side();
			if (bigger_side > max_size) {
				request.zoom = max_size / bigger_side;

				/* Allow zooming to four times the size of the miniature */
				request.zoom *= 4.0f;
				request.zoom = std::min(request.zoom, 1.0f);
			}
			else {
				request.zoom = 1.0f;
			}
		}
		else {
			request.zoom = cam ? cam->zoom : 1.0f;
		}

		request.reveal_when_complete = reveal;

		const auto screen_size = vec2i(ImGui::GetIO().DisplaySize);
		request.screen_size = screen_size;
		request.max_miniature_size = max_size;
		LOG("Requesting miniature. Screen size: %x. Max size: %x. World captured region: %x (%x). Zoom: %x", screen_size, max_size, *world_captured_region, world_captured_region->get_size(), request.zoom);

		miniature_generator.reset();
		miniature_generator.emplace(std::move(request));
	}
}

bool editor_setup::is_generating_miniature() const {
	return miniature_generator.has_value();
}

void editor_setup::recount_internal_resource_references_if_needed() {
	if (should_recount_internal_resource_references) {
		project.recount_references(official.resources, false);
		should_recount_internal_resource_references = false;
	}
}

template struct create_resource_command<editor_material_resource>;

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
template struct edit_resource_command<editor_tool_resource>;
template struct edit_resource_command<editor_melee_resource>;
template struct edit_resource_command<editor_explosive_resource>;

template struct edit_resource_command<editor_prefab_resource>;
template struct edit_resource_command<editor_game_mode_resource>;

template struct edit_node_command<editor_sprite_node>;
template struct edit_node_command<editor_sound_node>;
template struct edit_node_command<editor_light_node>;
template struct edit_node_command<editor_particles_node>;
template struct edit_node_command<editor_wandering_pixels_node>;
template struct edit_node_command<editor_point_marker_node>;
template struct edit_node_command<editor_area_marker_node>;

template struct edit_node_command<editor_firearm_node>;
template struct edit_node_command<editor_ammunition_node>;
template struct edit_node_command<editor_tool_node>;
template struct edit_node_command<editor_melee_node>;
template struct edit_node_command<editor_explosive_node>;

template struct edit_node_command<editor_prefab_node>;

template struct create_node_command<editor_sprite_node>;
template struct create_node_command<editor_sound_node>;
template struct create_node_command<editor_light_node>;
template struct create_node_command<editor_particles_node>;
template struct create_node_command<editor_wandering_pixels_node>;
template struct create_node_command<editor_point_marker_node>;
template struct create_node_command<editor_area_marker_node>;

template struct create_node_command<editor_firearm_node>;
template struct create_node_command<editor_ammunition_node>;
template struct create_node_command<editor_tool_node>;
template struct create_node_command<editor_melee_node>;
template struct create_node_command<editor_explosive_node>;

template struct create_node_command<editor_prefab_node>;

#include "application/network/network_common.h"
template void build_arena_from_editor_project<online_arena_handle<false>>(online_arena_handle<false> arena_handle, build_arena_input);
