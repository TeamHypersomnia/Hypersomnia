#include "augs/string/string_templates.h"
#include "augs/templates/algorithm_templates.h"
#include "augs/misc/imgui/imgui_utils.h"
#include "augs/filesystem/directory.h"
#include "augs/templates/thread_templates.h"
#include "augs/templates/algorithm_templates.h"
#include "augs/window_framework/window.h"
#include "augs/window_framework/platform_utils.h"
#include "augs/window_framework/shell.h"

#include "game/detail/visible_entities.h"
#include "view/viewables/images_in_atlas_map.h"

#include "application/config_lua_table.h"
#include "application/setups/editor/editor_setup.h"
#include "application/setups/editor/editor_paths.h"
#include "application/setups/editor/editor_camera.h"

#include "application/setups/editor/gui/editor_tab_gui.h"

#include "augs/readwrite/byte_file.h"
#include "augs/readwrite/lua_file.h"

std::optional<ltrb> editor_setup::find_screen_space_rect_selection(
	const vec2i screen_size,
	const vec2i mouse_pos
) const {
	if (const auto cam = find_current_camera()) {
		return selector.find_screen_space_rect_selection(*cam, screen_size, mouse_pos);
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

std::optional<camera_cone> editor_setup::find_current_camera() const {
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

	augs::save_as_text(LOG_FILES_DIR "/last_editor_message.txt", augs::date_time().get_readable() + '\n' + logged);
}

void editor_setup::set_locally_viewed(const entity_id id) {
	work().local_test_subject = id;
	//view().panned_camera = std::nullopt;
}

editor_setup::editor_setup(
	sol::state& lua
) : 
	recent(lua),
	destructor_input{ lua }
{
	augs::create_directories(get_untitled_dir());

	open_last_folders(lua);
	load_gui_state();
}

editor_setup::editor_setup(
	sol::state& lua, 
	const augs::path_type& intercosm_path
) : 
	recent(lua),
	destructor_input{ lua }
{
	augs::create_directories(get_untitled_dir());

	open_last_folders(lua);
	open_folder_in_new_tab({ lua, intercosm_path });
	load_gui_state();
}

void editor_setup::load_gui_state() {
	try {
		augs::load_from_bytes(*this, get_editor_gui_state_path());
	}
	catch (const augs::file_open_error) {
		// We don't care if it does not exist
	}
}

void editor_setup::save_gui_state() {
	augs::save_as_bytes(*this, get_editor_gui_state_path());
}

editor_setup::~editor_setup() {
	save_gui_state();
	force_autosave_now();
}

void editor_setup::force_autosave_now() const {
	autosave.save(destructor_input.lua, signi);
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

			if (const auto warning = f.load_folder_maybe_autosave()) {
				set_popup(*warning);
			}
			/* f.history.mark_as_just_saved(); */
		}
	);
}

void editor_setup::save_current_folder() {
	try {
		folder().save_folder();
		folder().history.mark_as_just_saved();
	}
	catch (std::runtime_error what) {
		set_popup({ "Error", "Failed to save the project.\nSome serious problem has occured.", what.what() });
	}
}

void editor_setup::save_current_folder_to(const path_operation op) {
	folder().set_folder_path(op.lua, op.path, recent);
	save_current_folder();
}

void editor_setup::fill_with_minimal_scene() {
	if (anything_opened()) {
		clear_id_caches();
		folder().history.execute_new(fill_with_test_scene_command(true), make_command_input());
	}
}

void editor_setup::fill_with_test_scene() {
	if (anything_opened()) {
		clear_id_caches();
		folder().history.execute_new(fill_with_test_scene_command(false), make_command_input());
	}
}

void editor_setup::perform_custom_imgui(
	sol::state& lua,
	augs::window& owner,
	const bool in_direct_gameplay,
	const images_in_atlas_map& game_atlas
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

	const bool has_ctrl = ImGui::GetIO().KeyCtrl;

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

					if (auto menu = scoped_menu("Recent projects", !recent.empty())) {
						/*	
							IMPORTANT! recent.paths can be altered in the loop by loading a intercosm,
							thus we need to copy its contents.
						*/

						const auto recent_paths = recent.paths;

						for (const auto& target_path : recent_paths) {
							const auto str = augs::to_display(target_path);

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
						save(owner);
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
					{
						const bool enable_undo = anything_opened() && !folder().history.is_revision_oldest();
						const bool enable_redo = anything_opened() && !folder().history.is_revision_newest();

						if (item_if_tabs_and(enable_redo, "Undo", "CTRL+Z")) { undo(); }
						if (item_if_tabs_and(enable_undo, "Redo", "CTRL+SHIFT+Z")) { redo(); }
					}

					ImGui::Separator();
					if (item_if_tabs("Cut", "CTRL+X")) {}
					if (item_if_tabs("Copy", "CTRL+C")) {}
					if (item_if_tabs("Paste", "CTRL+V")) {}
					ImGui::Separator();

#if BUILD_TEST_SCENES
					if (item_if_tabs("Fill with test scene", "SHIFT+F5")) {
						fill_with_test_scene();
					}

					if (item_if_tabs("Fill with minimal scene", "CTRL+SHIFT+F5")) {
						fill_with_minimal_scene();
					}
#else
					if (item_if_tabs_and(false, "Fill with test scene", "SHIFT+F5")) {}
					if (item_if_tabs_and(false, "Fill with minimal scene", "CTRL+SHIFT+F5")) {}
#endif
				}
				if (auto menu = scoped_menu("View")) {
					auto do_window_entry = [&](auto& win) {
						if (item_if_tabs(win.get_title().c_str())) {
							win.open();
						}
					};

					do_window_entry(history_gui);

					ImGui::Separator();

					do_window_entry(summary_gui);
					do_window_entry(coordinates_gui);

					if (item_if_tabs("Player")) {
						player.show = true;
					}

					do_window_entry(selection_groups_gui);

					ImGui::Separator();
					ImGui::MenuItem("(State)", NULL, false, false);

					do_window_entry(common_state_gui);
					do_window_entry(fae_gui);
					do_window_entry(selected_fae_gui);

					ImGui::Separator();
					ImGui::MenuItem("(Assets)", NULL, false, false);

					do_window_entry(images_gui);
					do_window_entry(sounds_gui);
					do_window_entry(particle_effects_gui);
					do_window_entry(plain_animations_gui);
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

		common_state_gui.perform(settings, make_command_input());

		{
			const auto output = fae_gui.perform(make_fae_gui_input(), view().selected_entities);

			if (const auto id = output.instantiate_id) {
				instantiate_flavour_command cmd;
				cmd.instantiated_id = *id;
				cmd.where.pos = get_world_cursor_pos();

				const auto& executed = post_editor_command(make_command_input(), std::move(cmd));
				const auto created_id = executed.get_created_id();
				view().selected_entities = { created_id };
				mover.start_moving_selection(make_mover_input());
			}
		}

		selection_groups_gui.perform(has_ctrl, make_command_input());

		summary_gui.perform(*this);

		images_gui.perform(settings.property_editor, game_atlas, make_command_input());
		sounds_gui.perform(settings.property_editor, game_atlas, make_command_input());

		particle_effects_gui.perform(settings.property_editor, make_command_input());
		plain_animations_gui.perform(settings.property_editor, make_command_input());

		const auto all_selected = [&]() -> decltype(get_all_selected_entities()) {
			auto selections = get_all_selected_entities();

			if (const auto held = selector.get_held(); held.is_set() && work().world[held]) {
				selections.emplace(held);

				if (!view().ignore_groups) {
					view().selection_groups.for_each_sibling(held, [&](const auto id){ selections.emplace(id); });
				}
			}

			if (const auto matching = get_matching_go_to_entity()) {
				selections.emplace(matching);
			}

			return selections;
		}();

		{
			const auto in = make_fae_gui_input();
			const auto output = selected_fae_gui.perform(in, all_selected);

			const auto& cosm = work().world;
			output.filter.perform(cosm, view().selected_entities);
		}

		coordinates_gui.perform(*this, screen_size, mouse_pos, all_selected);

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

		const auto go_to_dialog_pos = vec2 { static_cast<float>(screen_size.x / 2), menu_bar_size.y * 2 + 1 };

		if (const auto confirmation = 
			go_to_entity_gui.perform(settings.go_to, work().world, go_to_dialog_pos)
		) {
			::standard_confirm_go_to(*confirmation, has_ctrl, view());
		}

		if (const auto rot = mover.current_mover_rot_delta(make_mover_input())) {
			text_tooltip("%x*", *rot);
		}
		else if (const auto pos = mover.current_mover_pos_delta(make_mover_input())) {
			text_tooltip("x: %x\ny: %x", pos->x, pos->y);
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
		return setup_escape_result::JUST_FETCH;
	}
	else if (!player.paused) {
		player.paused = true;
		return setup_escape_result::SWITCH_TO_GAME_GUI;
	}
	else if (mover.escape()) {
		return setup_escape_result::JUST_FETCH;
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

void editor_setup::save(const augs::window& owner) {
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

std::unordered_set<entity_id> editor_setup::get_all_selected_entities() const {
	std::unordered_set<entity_id> all;

	for_each_selected_entity(
		[&](const auto e) {
			all.insert(e);
		}
	);

	return all;
}

void editor_setup::cut_selection() {
	delete_selection();
}

void editor_setup::delete_selection() {
	if (anything_opened()) {
		auto command = make_command_from_selections<delete_entities_command>("Deleted ");

		if (!command.empty()) {
			folder().history.execute_new(std::move(command), make_command_input());
			clear_id_caches();
		}
	}
}

void editor_setup::mirror_selection(const vec2i direction) {
	if (anything_opened()) {
		finish_rectangular_selection();

		const bool only_duplicating = direction.is_zero();

		auto command = make_command_from_selections<duplicate_entities_command>(only_duplicating ? "Duplicated " : "Mirrored ");

		if (!command.empty()) {
			command.mirror_direction = direction;
			folder().history.execute_new(std::move(command), make_command_input());
		}

		if (only_duplicating) {
			mover.start_moving_selection(make_mover_input());
			make_last_command_a_child();
		}
	}
}

void editor_setup::duplicate_selection() {
	mirror_selection(vec2i(0, 0));
}

void editor_setup::group_selection() {
	if (anything_opened()) {
		auto command = make_command_from_selections<change_grouping_command>("Grouped ");

		if (!command.empty()) {
			command.all_to_new_group = true;
			folder().history.execute_new(std::move(command), make_command_input());
		}
	}
}

void editor_setup::ungroup_selection() {
	if (anything_opened()) {
		auto command = make_command_from_selections<change_grouping_command>("Ungrouped ");

		if (!command.empty()) {
			folder().history.execute_new(std::move(command), make_command_input());
		}
	}
}

void editor_setup::make_last_command_a_child() {
	if (anything_opened()) {
		auto set_has_parent = [](auto& command) { 
			command.common.has_parent = true; 
		};

		std::visit(set_has_parent, folder().history.last_command());
	}
}

void editor_setup::center_view_at_selection() {
	if (anything_opened()) {
		if (const auto aabb = find_selection_aabb()) {
			view().center_at(aabb->get_center());
		}
	}
}

void editor_setup::go_to_all() {

}

void editor_setup::go_to_entity() {
	go_to_entity_gui.open();
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
		t.current_path = get_first_free_untitled_path("Project%x");
		augs::create_directories(t.current_path);
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

	base::refresh();
}

void editor_setup::close_folder() {
	if (anything_opened()) {
		close_folder(signi.current_index);
	}
}


editor_command_input editor_setup::make_command_input() {
	return { destructor_input.lua, folder(), selector, fae_gui, mover };
}

grouped_selector_op_input editor_setup::make_grouped_selector_op_input() const {
	return { view().selected_entities, view().selection_groups, view().ignore_groups };
}

editor_fae_gui_input editor_setup::make_fae_gui_input() {
	return { settings.property_editor, make_command_input() };
}

entity_mover_input editor_setup::make_mover_input() {
	return { *this };
}

void editor_setup::select_all_entities(const bool has_ctrl) {
	if (anything_opened()) {
		selector.select_all(work().world, view().rect_select_mode, has_ctrl, view().selected_entities);
	}
}

bool editor_setup::handle_input_before_imgui(
	const augs::event::state& common_input_state,
	const augs::event::change e,

	augs::window& window
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

		const bool has_alt{ common_input_state[key::LALT] };

		if (has_alt) {
			switch (k) {
				case key::A: fae_gui.open(); return true;
				case key::H: history_gui.open(); return true;
				case key::S: selected_fae_gui.open(); return true;
				case key::C: common_state_gui.open(); return true;
				case key::G: selection_groups_gui.open(); return true;
				case key::P: player.show = true; return true;
				case key::U: summary_gui.open(); return true;
				case key::O: coordinates_gui.open(); return true;
				case key::I: images_gui.open(); return true;
				case key::N: sounds_gui.open(); return true;
				case key::R: particle_effects_gui.open(); return true;
				case key::M: plain_animations_gui.open(); return true;
				default: break;
			}
		}

		if (player.paused) {
			const bool has_ctrl{ common_input_state[key::LCTRL] };
			const bool has_shift{ common_input_state[key::LSHIFT] };

			if (has_ctrl) {
				if (has_shift) {
					switch (k) {
						case key::E: reveal_in_explorer(window); return true;
						case key::TAB: prev_tab(); return true;
						case key::F5: fill_with_minimal_scene(); return true;
						default: break;
					}
				}

				switch (k) {
					case key::S: save(window); return true;
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
					case key::F5: fill_with_test_scene(); return true;
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
	const necessary_images_in_atlas_map& sizes_for_icons,

	const augs::event::state& common_input_state,
	const augs::event::change e,

	augs::window&
) {
	using namespace augs::event;
	using namespace augs::event::keys;

	const auto maybe_cone = find_current_camera();

	if (!maybe_cone || !anything_opened()) {
		return false;
	}

	const auto current_cone = *maybe_cone;
	const auto world_cursor_pos = get_world_cursor_pos(current_cone);

	const bool has_ctrl{ common_input_state[key::LCTRL] };
	const bool has_shift{ common_input_state[key::LSHIFT] };

	if (is_editing_mode()) {
		auto& cosm = work().world;
		const auto screen_size = vec2i(ImGui::GetIO().DisplaySize);

		if (e.was_any_key_pressed()) {
			const auto k = e.data.key.key;

			if (has_ctrl) {
				switch(k) {
					case key::LEFT: mirror_selection(vec2i(-1, 0)); return true;
					case key::RIGHT: mirror_selection(vec2i(1, 0)); return true;
					case key::UP: mirror_selection(vec2i(0, -1)); return true;
					case key::DOWN: mirror_selection(vec2i(0, 1)); return true;
					default: break;
				}
			}
		}

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
			if (mover.do_mousemotion(make_mover_input(), world_cursor_pos)) {
				return true;
			}

			selector.do_mousemotion(
				sizes_for_icons,
				cosm,
				view().rect_select_mode,
				world_cursor_pos,
				current_cone,
				common_input_state[key::LMOUSE]
			);

			return true;
		}

		if (e.was_pressed(key::SLASH)) {
			go_to_entity();
			return true;
		}

		if (e.was_pressed(key::LMOUSE)) {
			if (mover.do_left_press(make_mover_input())) {
				return true;	
			}
		}

		{
			auto& selections = view().selected_entities;

			if (e.was_pressed(key::LMOUSE)) {
				selector.do_left_press(cosm, has_ctrl, world_cursor_pos, selections);
				return true;
			}
			else if (e.was_released(key::LMOUSE)) {
				selections = selector.do_left_release(has_ctrl, make_grouped_selector_op_input());
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
					case key::A: select_all_entities(has_ctrl); return true;
					case key::_0: view().reset_zoom(); return true;
					case key::Z: undo(); return true;
					case key::C: copy(); return true;
					case key::X: cut(); return true;
					case key::V: paste(); return true;

					case key::G: group_selection(); return true;
					case key::U: ungroup_selection(); return true;
					case key::R: mover.rotate_selection_once_by(make_mover_input(), 90); return true;
					default: break;
				}
			}

			if (has_shift) {
				switch (k) {
					case key::R: mover.rotate_selection_once_by(make_mover_input(), -90); return true;
					default: break;
				}
			}

			auto clamp_units = [&]() { view().grid.clamp_units(8, settings.grid.render.get_maximum_unit()); };

			switch (k) {
				case key::O: 
					if (view().selected_entities.size() == 1) { 
						set_locally_viewed(*view().selected_entities.begin()); 
					}
					return true;
				case key::A: view().toggle_ignore_groups(); return true;
				case key::Z: center_view_at_selection(); if (has_shift) { view().reset_zoom(); } return true;
				case key::I: play(); return true;
				case key::F: view().toggle_flavour_rect_selection(); return true;
				case key::G: view().toggle_grid(); return true;
				case key::S: view().toggle_snapping(); return true;
				case key::OPEN_SQUARE_BRACKET: view().grid.decrease_grid_size(); clamp_units(); return true;
				case key::CLOSE_SQUARE_BRACKET: view().grid.increase_grid_size(); clamp_units(); return true;
				case key::C: duplicate_selection(); return true;
				case key::D: cut_selection(); return true;
				case key::DEL: delete_selection(); return true;
				case key::T: mover.start_moving_selection(make_mover_input()); return true;
				case key::R: mover.start_rotating_selection(make_mover_input()); return true;
				default: break;
			}

			// history.seek_to_revision(has_shift ? history.get_commands().size() - 1 : 0, make_command_input()); 
		}
	}

	return false;
}

vec2 editor_setup::get_world_cursor_pos() const {
	return get_world_cursor_pos(find_current_camera().value());
}

vec2 editor_setup::get_world_cursor_pos(const camera_cone cone) const {
	const auto mouse_pos = vec2i(ImGui::GetIO().MousePos);
	const auto screen_size = vec2i(ImGui::GetIO().DisplaySize);

	return cone.to_world_space(screen_size, mouse_pos);
}

const editor_view* editor_setup::find_view() const {
	if (anything_opened()) {
		return std::addressof(view());
	}

	return nullptr;
}

std::optional<ltrb> editor_setup::find_selection_aabb() const {
	if (anything_opened() && player.paused) {
		return selector.find_selection_aabb(work().world, make_grouped_selector_op_input());
	}

	return std::nullopt;
}

std::optional<rgba> editor_setup::find_highlight_color_of(const entity_id id) const {
	if (anything_opened() && player.paused) {
		return selector.find_highlight_color_of(
			settings.entity_selector, id, make_grouped_selector_op_input()
		);
	}

	return std::nullopt;
}

augs::path_type editor_setup::get_unofficial_content_dir() const {
	if (anything_opened()) {
		return folder().current_path;
	}

	return {};
}
