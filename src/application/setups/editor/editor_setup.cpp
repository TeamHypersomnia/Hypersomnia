#include "augs/templates/string_templates.h"
#include "augs/templates/algorithm_templates.h"
#include "augs/misc/time_utils.h"
#include "augs/misc/imgui/imgui_utils.h"
#include "augs/filesystem/directory.h"
#include "augs/templates/thread_templates.h"
#include "augs/templates/chrono_templates.h"
#include "augs/window_framework/window.h"
#include "augs/window_framework/platform_utils.h"
#include "augs/window_framework/shell.h"

#include "game/detail/visible_entities.h"
#include "game/detail/describers.h"

#include "application/config_lua_table.h"
#include "application/setups/editor/editor_setup.h"
#include "application/setups/editor/editor_paths.h"
#include "application/setups/editor/editor_camera.h"

#include "application/setups/editor/gui/editor_tab_gui.h"

#include "3rdparty/imgui/imgui_internal.h"

#include "augs/readwrite/byte_file.h"
#include "augs/readwrite/lua_file.h"

std::optional<ltrb> editor_setup::get_screen_space_rect_selection(
	const vec2i screen_size,
	const vec2i mouse_pos
) const {
	if (const auto cam = get_current_camera()) {
		return selector.get_screen_space_rect_selection(*cam, screen_size, mouse_pos);
	}

	return std::nullopt;
}

void editor_setup::open_last_folders(sol::state& lua) {
	catch_popup([&]() { ::open_last_folders(lua, signi); });
	base::refresh();
}

double editor_setup::get_audiovisual_speed() const {
	return player.get_speed();
}

const cosmos& editor_setup::get_viewed_cosmos() const {
	return anything_opened() ? work().world : cosmos::zero; 
}

real32 editor_setup::get_interpolation_ratio() const {
	return timer.fraction_of_step_until_next_step(get_viewed_cosmos().get_fixed_delta());
}

entity_id editor_setup::get_viewed_character_id() const {
	return anything_opened() ? work().local_test_subject : entity_id();
}

const_entity_handle editor_setup::get_viewed_character() const {
	return get_viewed_cosmos()[get_viewed_character_id()];
}

const all_viewables_defs& editor_setup::get_viewable_defs() const {
	return anything_opened() ? work().viewables : all_viewables_defs::empty;
}

void editor_setup::unhover() {
	selector.unhover();
}

bool editor_setup::is_editing_mode() const {
	return player.is_editing_mode();
}

std::optional<camera_cone> editor_setup::get_current_camera() const {
	if (anything_opened()) {
		return editor_detail::calculate_camera(player, view(), get_matching_go_to_entity(), work());
	}

	return std::nullopt;
}

const_entity_handle editor_setup::get_matching_go_to_entity() const {
	return go_to_entity_gui.get_matching_go_to_entity(work().world);
}

void editor_setup::on_folder_changed() {
	player.paused = true;
	selector.clear();
}

void editor_setup::set_popup(const editor_popup p) {
	ok_only_popup = p;

	const auto logged = typesafe_sprintf(
		"%x\n%x\n%x", 
		p.title, p.message, p.details
	);

	LOG(logged);

	augs::save_as_text(LOG_FILES_DIR "last_editor_message.txt", augs::date_time().get_readable() + '\n' + logged);
}

void editor_setup::set_locally_viewed(const entity_id id) {
	work().local_test_subject = id;
	view().panned_camera = std::nullopt;
}

editor_setup::editor_setup(
	sol::state& lua
) : 
	destructor_input{ lua },
	recent(lua) 
{
	augs::create_directories(get_untitled_dir());

	open_last_folders(lua);
}

editor_setup::editor_setup(
	sol::state& lua, 
	const augs::path_type& intercosm_path
) : 
	destructor_input{ lua },
	recent(lua) 
{
	augs::create_directories(get_untitled_dir());

	open_last_folders(lua);
	open_folder_in_new_tab({ lua, intercosm_path });
}

void editor_setup::force_autosave_now() const {
	autosave.save(destructor_input.lua, signi);
}

editor_setup::~editor_setup() {
	force_autosave_now();
}

void editor_setup::control(
	const cosmic_entropy& entropy
) {
	total_collected_entropy += entropy;
}

void editor_setup::accept_game_gui_events(
	const cosmic_entropy& entropy
) {
	control(entropy);
}

void editor_setup::customize_for_viewing(config_lua_table& config) const {
	if (anything_opened()) {
		config.window.name = "Editor - " + folder().get_display_path();
	}
	else {
		config.window.name = "Editor";
	}

	if (player.paused) {
		config.drawing.draw_aabb_highlighter = false;
		config.interpolation.enabled = false;
	}

	return;
}

void editor_setup::apply(const config_lua_table& cfg) {
	settings = cfg.editor;
	return;
}

void editor_setup::open_folder_in_new_tab(const path_operation op) {
	for (std::size_t i = 0; i < signi.folders.size(); ++i) {
		if (signi.folders[i].current_path == op.path) {
			set_current(static_cast<folder_index>(i));
			return;
		}
	}

	try_to_open_new_folder(
		[this, op](editor_folder& f) {
			f.set_folder_path(op.lua, op.path, recent);
			f.load_folder();
			f.history.mark_as_just_saved();
		}
	);
}

void editor_setup::save_current_folder() {
	folder().save_folder();
	folder().history.mark_as_just_saved();
}

void editor_setup::save_current_folder_to(const path_operation op) {
	folder().set_folder_path(op.lua, op.path, recent);
	save_current_folder();
}

void editor_setup::fill_with_minimal_scene(sol::state& lua) {
	if (anything_opened()) {
		clear_id_caches();

		fill_with_test_scene_command command;
		command.minimal = true;

		folder().history.execute_new(command, make_command_input());
	}
}

void editor_setup::fill_with_test_scene(sol::state& lua) {
	if (anything_opened()) {
		clear_id_caches();

		fill_with_test_scene_command command;
		command.minimal = false;

		folder().history.execute_new(command, make_command_input());
	}
}

void editor_setup::perform_custom_imgui(
	sol::state& lua,
	augs::window& owner,
	const bool in_direct_gameplay
) {
	using namespace augs::imgui;

	autosave.advance(lua, signi, settings.autosave);

	auto path_op = [&](const auto& path) {
		return path_operation{ lua, path };
	};

	auto item_if_tabs_and = [this](const bool condition, const char* label, const char* shortcut = nullptr) {
		return ImGui::MenuItem(label, shortcut, nullptr, condition && anything_opened());
	};

	auto item_if_tabs = [&](const char* label, const char* shortcut = nullptr) {
		return item_if_tabs_and(true, label, shortcut);
	};

	const auto mouse_pos = vec2i(ImGui::GetIO().MousePos);
	const auto screen_size = vec2i(ImGui::GetIO().DisplaySize);

	const auto& g = *ImGui::GetCurrentContext();
	const auto menu_bar_size = ImVec2(g.IO.DisplaySize.x, g.FontBaseSize + g.Style.FramePadding.y * 2.0f);

	if (!in_direct_gameplay) {
		{
			/* We don't want ugly borders in our menu bar */
			auto window_border_size = scoped_style_var(ImGuiStyleVar_WindowBorderSize, 0.0f);
			auto frame_border_size = scoped_style_var(ImGuiStyleVar_FrameBorderSize, 0.0f);

			if (auto main_menu = scoped_main_menu_bar()) {
				if (auto menu = scoped_menu("File")) {
					if (ImGui::MenuItem("New", "CTRL+N")) {
						new_tab();
					}

					if (ImGui::MenuItem("Open", "CTRL+O")) {
						open(owner);
					}

					if (auto menu = scoped_menu("Recent files", !recent.empty())) {
						/*	
							IMPORTANT! recent.paths can be altered in the loop by loading a intercosm,
							thus we need to copy its contents.
						*/

						const auto recent_paths = recent.paths;

						for (const auto& target_path : recent_paths) {
							const auto str = augs::to_display_path(target_path).string();

							if (ImGui::MenuItem(str.c_str())) {
								open_folder_in_new_tab(path_op(target_path));
							}
						}

						ImGui::Separator();

						if (ImGui::MenuItem("Clear Recent Items")) {
							recent.clear(lua);
						}
					}

					ImGui::Separator();

					if (item_if_tabs("Save", "CTRL+S")) {
						save(lua, owner);
					}

					if (item_if_tabs("Save as", "F12")) {
						save_as(owner);
					}

					ImGui::Separator();

					const auto close_str = [&]() -> std::string {
						if (anything_opened()) {
							return std::string("Close ") + folder().get_display_path();
						}

						return "Close";
					}();

					if (item_if_tabs(close_str.c_str(), "CTRL+W")) {
						close_folder();
					}

					if (item_if_tabs("Close all")) {
						while (anything_opened()) {
							close_folder();
						}
					}
				}

				if (auto menu = scoped_menu("Edit")) {
					if (item_if_tabs_and(!folder().history.is_revision_oldest(), "Undo", "CTRL+Z")) { undo(); }
					if (item_if_tabs_and(!folder().history.is_revision_newest(), "Redo", "CTRL+SHIFT+Z")) { redo(); }

					ImGui::Separator();
					if (item_if_tabs("Cut", "CTRL+X")) {}
					if (item_if_tabs("Copy", "CTRL+C")) {}
					if (item_if_tabs("Paste", "CTRL+V")) {}
					ImGui::Separator();

#if BUILD_TEST_SCENES
					if (item_if_tabs("Fill with test scene", "SHIFT+F5")) {
						fill_with_test_scene(lua);
					}

					if (item_if_tabs("Fill with minimal scene", "CTRL+SHIFT+F5")) {
						fill_with_minimal_scene(lua);
					}
#else
					if (item_if_tabs_and(false, "Fill with test scene", "SHIFT+F5")) {}
					if (item_if_tabs_and(false, "Fill with minimal scene", "CTRL+SHIFT+F5")) {}
#endif
				}
				if (auto menu = scoped_menu("View")) {
					if (item_if_tabs("Summary")) {
						show_summary = true;
					}
					if (item_if_tabs("Player")) {
						player.show = true;
					}

					ImGui::Separator();
					ImGui::MenuItem("(State)", NULL, false, false);

					if (item_if_tabs("Common")) {
						show_common_state = true;
					}

					if (item_if_tabs("All entities")) {
						all_entities_gui.open();
					}

					if (item_if_tabs("History")) {
						history_gui.open();
					}
				}
			}
		}

		if (anything_opened()) {
			perform_editor_tab_gui(
				[&](const auto index_to_close){ close_folder(index_to_close); },
				[&](const auto index_to_set){ set_current(index_to_set); },
				signi,
				menu_bar_size.y
			);
		}
	}

	if (anything_opened()) {
		history_gui.perform(make_command_input());

		if (show_summary) {
			auto summary = scoped_window("Summary", &show_summary, ImGuiWindowFlags_AlwaysAutoResize);

			if (anything_opened()) {
				text(typesafe_sprintf("Folder path: %x", folder().current_path));
				//text("Tick rate: %x/s", get_viewed_cosmos().get_solvable().get_steps_per_second()));

				if (const auto current_cone = get_current_camera()) {
					const auto world_cursor_pos = current_cone->to_world_space(screen_size, mouse_pos);
					text("Cursor: %x", world_cursor_pos);
					text("View center: %x", current_cone->transform.pos);
					text(typesafe_sprintf("Zoom: %x", current_cone->zoom * 100.f) + " %");
				}

				text("Total entities: %x",
					get_viewed_cosmos().get_entities_count()
				);

				text("World time: %x (%x steps at %x Hz)",
					standard_format_seconds(get_viewed_cosmos().get_total_seconds_passed()),
					get_viewed_cosmos().get_total_steps_passed(),
					1.0f / get_viewed_cosmos().get_fixed_delta().in_seconds()
				);

				text(L"Currently controlling: %x",
					get_viewed_character().alive() ? get_viewed_character().get_name() : L"no entity"
				);
			}
		}

		if (player.show) {
			auto player_window = scoped_window("Player", &player.show, ImGuiWindowFlags_AlwaysAutoResize);

			if (ImGui::Button("Play")) {
				play();
			}
			ImGui::SameLine();
			
			if (ImGui::Button("Pause")) {
				pause();
			}
			ImGui::SameLine();
			
			if (ImGui::Button("Stop")) {
				stop();
			}
		}

		if (show_common_state) {
			auto common = scoped_window("Common", &show_common_state, ImGuiWindowFlags_AlwaysAutoResize);
		}

		all_entities_gui.perform(make_command_input());

		const auto go_to_dialog_pos = vec2 { static_cast<float>(screen_size.x / 2), menu_bar_size.y * 2 + 1 };

		if (const auto confirmation = 
			go_to_entity_gui.perform(settings.go_to, work().world, go_to_dialog_pos)
		) {
			::standard_confirm_go_to(*confirmation, view());
		}
	}

	if (open_folder_dialog.valid() && is_ready(open_folder_dialog)) {
		const auto result_path = open_folder_dialog.get();

		if (result_path) {
			open_folder_in_new_tab(path_op(*result_path));
		}
	}

	if (save_project_dialog.valid() && is_ready(save_project_dialog)) {
		if (const auto result_path = save_project_dialog.get()) {
			if (::is_untitled_path(*result_path)) {
				set_popup({"Error", "Can't save to a directory with untitled projects.", ""});
			}
			else {
				const auto previous_path = folder().current_path;
				const bool was_untitled = folder().is_untitled();

				save_current_folder_to(path_op(*result_path));

				if (was_untitled) {
					augs::remove_directory(previous_path);
				}
			}
		}
	}

	if (ok_only_popup && ok_only_popup->perform()) {
		ok_only_popup = std::nullopt;
	}
}

void editor_setup::clear_id_caches() {
	selector.clear();

	if (anything_opened()) {
		view().selected_entities.clear();
	}
}

void editor_setup::finish_rectangular_selection() {
	if (anything_opened()) {
		selector.finish_rectangular(view().selected_entities);
	}
}

std::optional<setup_escape_result> editor_setup::escape() {
	if (ok_only_popup) {
		ok_only_popup = std::nullopt;
		return std::nullopt;
	}
	else if (!player.paused) {
		player.paused = true;
		return setup_escape_result::SWITCH_TO_GAME_GUI;
	}

	return setup_escape_result::LAUNCH_INGAME_MENU;
}

bool editor_setup::confirm_modal_popup() {
	if (ok_only_popup) {
		ok_only_popup = std::nullopt;
		return true;
	}

	return false;
}

void editor_setup::open(const augs::window& owner) {
	if (ok_only_popup) {
		return;
	}

	open_folder_dialog = std::async(
		std::launch::async,
		[&](){
			return owner.choose_directory_dialog("Open folder with project files");
		}
	);
}

void editor_setup::save(sol::state& lua, const augs::window& owner) {
	if (!anything_opened()) {
		return;
	}

	if (folder().is_untitled()) {
		save_as(owner);
	}
	else {
		save_current_folder();
	}
}

void editor_setup::save_as(const augs::window& owner) {
	if (!anything_opened() || ok_only_popup) {
		return;
	}

	save_project_dialog = std::async(
		std::launch::async,
		[&](){
			return owner.choose_directory_dialog("Choose folder for project files");
		}
	);
}

void editor_setup::undo() {
	if (anything_opened()) {
		folder().history.undo(make_command_input());
	}
}

void editor_setup::redo() {
	if (anything_opened()) {
		folder().history.redo(make_command_input());
	}
}

void editor_setup::copy() {

}

void editor_setup::cut() {

}

void editor_setup::paste() {

}

void editor_setup::del() {
	if (anything_opened()) {
		thread_local std::vector<entity_id> all_deleted;
		all_deleted.clear();

		const auto& cosm = work().world;
		delete_entities_command command;

		for_each_selected_entity(
			[&](const auto e) {
				command.push_entry(cosm[e]);
				all_deleted.push_back(e);
			}
		);

		if (all_deleted.size() == cosm.get_entities_count()) {
			command.built_description = "Deleted all entities";
		}
		else {
			command.built_description = to_string(L"Deleted " + ::describe_names_of(all_deleted, cosm));
		}

		if (!command.empty()) {
			folder().history.execute_new(std::move(command), make_command_input());
			clear_id_caches();
		}
	}
}

void editor_setup::go_to_all() {

}

void editor_setup::go_to_entity() {
	go_to_entity_gui.init();
}

void editor_setup::reveal_in_explorer(const augs::window& owner) {
	owner.reveal_in_explorer(folder().get_paths().int_file);
}

void editor_setup::play() {
	player.paused = false;
}

void editor_setup::pause() {
	player.paused = true;
}

void editor_setup::play_pause() {
	bool& f = player.paused;
	f = !f;
}

void editor_setup::stop() {
	player.paused = true;
}

void editor_setup::prev() {
	player.paused = true;
}

void editor_setup::next() {
	player.paused = true;
}

void editor_setup::new_tab() {
	try_to_open_new_folder([&](editor_folder& t) {
		const auto new_path = get_first_free_untitled_path("Project%x");
		augs::create_directories(augs::path_type(new_path) += "/");
		t.current_path = new_path;

		/* 
			Initial write-out so that we have some empty intercosms
			in the folder to be loaded later on restart
		*/

		t.save_folder();
	});
}

void editor_setup::next_tab() {
	if (anything_opened()) {
		set_current((signi.current_index + 1) % signi.folders.size());
	}
}

void editor_setup::prev_tab() {
	if (anything_opened()) {
		set_current(signi.current_index == 0 ? static_cast<folder_index>(signi.folders.size() - 1) : signi.current_index - 1);
	}
}

void editor_setup::close_folder(const folder_index i) {
	auto& folder_to_close = signi.folders[i];

	if (folder_to_close.at_unsaved_revision()) {
		set_popup({ "Error", "Nie", "Nie" });
		return;
	}
		
	if (folder_to_close.is_untitled()) {
		augs::remove_directory(folder_to_close.current_path);
	}

	signi.folders.erase(signi.folders.begin() + i);

	if (signi.folders.empty()) {
		signi.current_index = -1;
		pause();
	}
	else {
		signi.current_index = std::min(signi.current_index, static_cast<folder_index>(signi.folders.size() - 1));
	}
}

void editor_setup::close_folder() {
	if (anything_opened()) {
		close_folder(signi.current_index);
	}
}


editor_command_input editor_setup::make_command_input() {
	return { destructor_input.lua, folder(), selector };
}

void editor_setup::select_all_entities() {
	const auto& cosm = work().world;

	auto& selections = view().selected_entities;
	selections.reserve(cosm.get_entities_count());

	cosmic::for_each_entity(cosm, [&](const auto handle) {
		selections.emplace(handle.get_id());
	});
}

bool editor_setup::handle_input_before_imgui(
	const augs::event::state& common_input_state,
	const augs::event::change e,

	augs::window& window,
	sol::state& lua
) {
	using namespace augs::event;
	using namespace keys;

	if (settings.autosave.on_lost_focus && e.msg == message::deactivate) {
		force_autosave_now();
	}

	if (e.was_any_key_pressed()) {
		const auto k = e.data.key.key;
		
		/* Media buttons work regardless of pause */

		switch (k) {
			case key::PLAY_PAUSE_TRACK: play_pause(); return true;
			case key::PREV_TRACK: prev(); return true;
			case key::NEXT_TRACK: next(); return true;
			case key::STOP_TRACK: stop(); return true;
			default: break;
		}

		if (player.paused) {
			const bool has_ctrl{ common_input_state[key::LCTRL] };
			const bool has_shift{ common_input_state[key::LSHIFT] };

			if (has_ctrl) {
				if (has_shift) {
					switch (k) {
						case key::E: reveal_in_explorer(window); return true;
						case key::TAB: prev_tab(); return true;
						case key::F5: fill_with_minimal_scene(lua); return true;
						default: break;
					}
				}

				switch (k) {
					case key::S: save(lua, window); return true;
					case key::O: open(window); return true;
					case key::COMMA: go_to_all(); return true;
					case key::N: new_tab(); return true;
					case key::W: close_folder(); return true;
					case key::TAB: next_tab(); return true;
					default: break;
				}
			}

			if (has_shift) {
				switch (k) {
					case key::F5: fill_with_test_scene(lua); return true;
					default: break;
				}
			}

			switch (k) {
				case key::F12: save_as(window); return true;
				case key::ENTER: return confirm_modal_popup();
				default: break;
			}
		}
	}

	return false;
}


bool editor_setup::handle_input_before_game(
	const augs::event::state& common_input_state,
	const augs::event::change e,

	augs::window& window,
	sol::state& lua
) {
	using namespace augs::event;
	using namespace augs::event::keys;

	const auto mouse_pos = vec2i(ImGui::GetIO().MousePos);
	const auto screen_size = vec2i(ImGui::GetIO().DisplaySize);

	if (!anything_opened()) {
		return false;
	}

	const auto current_cone = *get_current_camera();
	const auto world_cursor_pos = current_cone.to_world_space(screen_size, mouse_pos);

	const bool has_ctrl{ common_input_state[key::LCTRL] };
	const bool has_shift{ common_input_state[key::LSHIFT] };

	if (is_editing_mode()) {
		if (editor_detail::handle_camera_input(
			settings.camera,
			current_cone,
			common_input_state,
			e,
			world_cursor_pos,
			screen_size,
			view().panned_camera
		)) {
			return true;
		}

		if (e.msg == message::mousemotion) {
			selector.do_mousemotion(
				work().world,
				world_cursor_pos,
				common_input_state[key::LMOUSE]
			);

			return true;
		}

		if (e.was_pressed(key::SLASH)) {
			go_to_entity();
			return true;
		}

		{
			auto& selections = view().selected_entities;

			if (e.was_pressed(key::LMOUSE)) {
				selector.do_left_press(has_ctrl, world_cursor_pos, selections);
				return true;
			}
			if (e.was_released(key::LMOUSE)) {
				selector.do_left_release(has_ctrl, selections);
			}
		}

		if (e.was_any_key_pressed()) {
			const auto k = e.data.key.key;

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
					case key::F: all_entities_gui.open(); return true;
					case key::H: history_gui.open(); return true;
					case key::C: copy(); return true;
					case key::X: cut(); return true;
					case key::V: paste(); return true;
					default: break;
				}
			}

			switch (k) {
				case key::C: 
					if (view().selected_entities.size() == 1) { 
						set_locally_viewed(*view().selected_entities.begin()); 
						return true;
					}
				case key::I: play(); return true;
				case key::DEL: del(); return true;
				default: break;
			}
		}
	}

	return false;
}