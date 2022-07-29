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
#include "augs/templates/traits/has_size.h"
#include "augs/templates/traits/has_flip.h"
#include "application/setups/editor/detail/make_command_from_selections.h"

editor_setup::editor_setup(const augs::path_type& project_path) : paths(project_path) {
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
}

void editor_setup::create_official() {
	::create_official_resources(official_resources);
	::create_official_filesystem_from(official_resources, official_files_root);
}

void editor_setup::open_default_windows() {
	gui.inspector.open();
	gui.layers.open();
	gui.filesystem.open();
	gui.history.open();
}

bool editor_setup::handle_input_before_imgui(
	handle_input_before_imgui_input in
) {
	using namespace augs::event;

	if (in.e.was_pressed(keys::key::LMOUSE)) {
		if (mover.do_left_press(make_mover_input())) {
			return true;	
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
				case key::A: select_all_entities(has_ctrl); return true;
				case key::Z: undo(); return true;
				//case key::C: copy(); return true;
				//case key::X: cut(); return true;
				//case key::V: paste(); return true;

				case key::R: mover.rotate_selection_once_by(make_mover_input(), 90); return true;
				case key::E: mover.start_resizing_selection(make_mover_input(), true); return true;
				case key::F: gui.filesystem.open(); return true;
				case key::L: gui.layers.open(); return true;
				case key::H: gui.history.open(); return true;
				case key::I: gui.inspector.open(); return true;
				default: break;
			}
		}

		auto clamp_units = [&]() { view.grid.clamp_units(8, settings.grid.render.get_maximum_unit()); };

		if (has_shift && !has_ctrl) {
			switch (k) {
				case key::R: mover.rotate_selection_once_by(make_mover_input(), -90); return true;
				case key::H: mover.flip_selection(make_mover_input(), flip_flags::make_horizontally()); return true;
				case key::V: mover.flip_selection(make_mover_input(), flip_flags::make_vertically()); return true;
				default: break;
			}
		}

		if (!has_shift && !has_ctrl) {
			switch (k) {
				case key::D: cut_selection(); return true;
				case key::DEL: delete_selection(); return true;

				case key::T: start_moving_selection(); return true;
				case key::E: mover.start_resizing_selection(make_mover_input(), false); return true;
				case key::R: mover.start_rotating_selection(make_mover_input()); return true;
				case key::W: mover.reset_rotation(make_mover_input()); return true;
				case key::F: center_view_at_selection(); return true;
				case key::G: view.toggle_grid(); return true;
				case key::S: view.toggle_snapping(); return true;
				case key::OPEN_SQUARE_BRACKET: view.grid.increase_grid_size(); clamp_units(); return true;
				case key::CLOSE_SQUARE_BRACKET: view.grid.decrease_grid_size(); clamp_units(); return true;
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

		if (cached_selected_comparison_after != cached_selected_comparison) {
			inspect(cached_selected_comparison_after);
		}
	};

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
}

std::optional<ad_hoc_atlas_subjects> editor_setup::get_new_ad_hoc_images() {
	if (rebuild_ad_hoc_atlas) {
		rebuild_ad_hoc_atlas = false;

		ad_hoc_atlas_subjects new_subjects;
		const auto next_id = files.root.fill_thumbnail_entries(paths.project_folder, new_subjects);
		official_files_root.fill_thumbnail_entries(augs::path_type(OFFICIAL_CONTENT_DIR), new_subjects, next_id);

		auto cache_thumbnail_id_in_resource = [&](const auto& file_node) {
			on_resource(
				file_node.associated_resource,
				[&]<typename T>(T& typed_resource, const auto resource_id) {
					(void)resource_id;

					if constexpr(std::is_same_v<T, editor_sprite_resource>) {
						typed_resource.thumbnail_id = file_node.file_thumbnail_id;
					}
				}
			);
		};

		files.root.for_each_file_recursive(cache_thumbnail_id_in_resource);
		official_files_root.for_each_file_recursive(cache_thumbnail_id_in_resource);

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

augs::path_type editor_setup::resolve(const augs::path_type& path_in_project) const {
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
			const auto full_path = resolve(path_in_project);

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

				if (moved_resource && !augs::exists(resolve(moved_resource->external_file.path_in_project))) {
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
						}
						catch (...) {
							new_resource.editable.size.set(32, 32);
						}
					}

					file.associated_resource.set<resource_type>(new_id, false);
				}
			}
		};

		files.root.for_each_file_recursive(add_if_new);

		for (auto& entry : pool) {
			auto& r = entry.external_file;

			if (!augs::exists(resolve(r.path_in_project))) {
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

void editor_setup::inspect(const current_selections_type& selections) {
	gui.inspector.all_inspected.clear();

	for (const auto entity : selections) {
		gui.inspector.all_inspected.emplace_back(to_node_id(entity));
	}
}

void editor_setup::inspect(const std::vector<entity_id>& selections) {
	gui.inspector.all_inspected.clear();

	for (const auto entity : selections) {
		gui.inspector.all_inspected.emplace_back(to_node_id(entity));
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

void editor_setup::inspect_add(inspected_variant new_inspected, const bool update_selector) {
	gui.inspector.inspect(new_inspected, true);

	if (update_selector) {
		inspected_to_entity_selector_state();
	}
}

void editor_setup::inspect(inspected_variant new_inspected) {
	gui.inspector.inspect(new_inspected, wants_multiple_selection());

	inspected_to_entity_selector_state();
}

void editor_setup::inspect_only(inspected_variant new_inspected) {
	gui.inspector.inspect(new_inspected, false);

	inspected_to_entity_selector_state();
}

bool editor_setup::is_inspected(inspected_variant inspected) const {
	return found_in(gui.inspector.all_inspected, inspected);
}

std::vector<inspected_variant> editor_setup::get_all_inspected() const {
	return gui.inspector.all_inspected;
}

editor_command_input editor_setup::make_command_input() {
	return { *this };
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

		const auto prev_inspected = get_all_inspected();

		gui.history.scroll_to_current_once = true;
		history.undo(make_command_input());
		LOG("UNDO");

		gui.filesystem.clear_drag_drop();
		rebuild_scene();

		if (prev_inspected != get_all_inspected()) {
			inspected_to_entity_selector_state();
		}
	} while(repeat);
}

void editor_setup::redo() {
	do {
		const auto prev_inspected = get_all_inspected();

		gui.history.scroll_to_current_once = true;
		history.redo(make_command_input());
		LOG("REDO");

		gui.filesystem.clear_drag_drop();
		rebuild_scene();

		if (prev_inspected != get_all_inspected()) {
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
	const auto name_map = make_name_to_node_map();

	auto is_node_name_free = [&](const auto& name) {
		return !found_in(name_map, name);
	};

	return augs::first_free_string(
		new_name + "%x",
		" (%x)",
		is_node_name_free
	);
}

std::string editor_setup::get_free_layer_name(const std::string& name_pattern) {
	return augs::first_free_string(
		name_pattern, 
		" %x", 
		[this](const auto& candidate){ return nullptr == find_layer(candidate); }
	);
}


void editor_setup::create_new_layer(const std::string& name_pattern) {
	create_layer_command cmd;
	cmd.chosen_name = get_free_layer_name(name_pattern);

	post_new_command(cmd);
}

bool editor_setup::wants_multiple_selection() const {
	return ImGui::GetIO().KeyCtrl;
}

std::optional<std::pair<editor_layer_id, std::size_t>> editor_setup::find_parent_layer(const editor_node_id node_id) const {
	if (!node_id.is_set()) {
		return std::nullopt;
	}

	for (const auto layer_id : project.layers.order) {
		if (const auto layer = find_layer(layer_id)) {
			const auto& nodes = layer->hierarchy.nodes;

			for (std::size_t i = 0; i < nodes.size(); ++i) {
				if (nodes[i] == node_id) {
					return std::pair<editor_layer_id, std::size_t> { layer_id, i };
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
	return editor_entity_selector::calc_hovered_entity(
		scene.world,
		sizes_for_icons,
		get_camera_eye().zoom,
		at ? *at : get_world_cursor_pos(),
		render_layer_filter::all()
	);
}

editor_node_id editor_setup::get_hovered_node(const necessary_images_in_atlas_map& sizes_for_icons, const std::optional<vec2> at) const {
	return to_node_id(get_hovered_entity(sizes_for_icons, at));
}

void editor_setup::scroll_once_to(editor_node_id id) {
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

void editor_setup::rebuild_scene() {
	scene.clear();

	for (auto& s : scene_entity_to_node) {
		s.clear();
	}

	const auto mutable_access = cosmos_common_significant_access();
	auto& common = scene.world.get_common_significant(mutable_access);
	common.light.ambient_color = rgba(180, 180, 180, 255);
	(void)common;

	/* Create resources */

	auto resource_pool_handler = [&]<typename P>(const P& pool, const bool is_official) {
		using resource_type = typename P::value_type;

		auto& viewables = scene.viewables;

		for (const auto& resource : pool) {
			if constexpr(std::is_same_v<editor_sprite_resource, resource_type>) {
				auto create_as = [&]<typename entity_type>(entity_type) {
					auto& flavour_pool = scene.world.get_flavours<entity_type>(mutable_access);
					auto& definitions = viewables.image_definitions;
					
					const auto [new_image_id, new_definition] = definitions.allocate();

					{
						/* 
							Note that this is later resolved with get_unofficial_content_dir.
							get_unofficial_content_dir could really be depreciated,
							it existed because an intercosm always had relative paths since it was saved.
							Now we won't need it because an intercosm will always exist in memory only.
							Even if we cache things in some .cache folder per-project,
							I think we'd be better off just caching binary representation of the project data instead of intercosms.
						*/

						new_definition.source_image.path = resource.external_file.path_in_project;
						new_definition.source_image.is_official = is_official;

						auto& meta = new_definition.meta;
						(void)meta;
					}

					const auto [new_raw_flavour_id, new_flavour] = flavour_pool.allocate();
					const auto new_flavour_id = typed_entity_flavour_id<entity_type>(new_raw_flavour_id);

					const auto& editable = resource.editable;

					{
						auto& render = new_flavour.template get<invariants::render>();
						render.layer = render_layer::GROUND;
					}

					{
						auto& sprite = new_flavour.template get<invariants::sprite>();
						sprite.set(new_image_id, editable.size, editable.color);
					}

					/* Cache for quick and easy mapping */
					resource.scene_flavour_id = new_flavour_id;
				};

				// for now the only supported one
				create_as(static_decoration());
			}
			else if constexpr(std::is_same_v<editor_sound_resource, resource_type>) {
				auto create_as = [&]<typename entity_type>(entity_type) {
					auto& flavour_pool = scene.world.get_flavours<entity_type>(mutable_access);
					auto& definitions = viewables.sounds;
					
					const auto [new_sound_id, new_definition] = definitions.allocate();

					{
						new_definition.source_sound.path = resource.external_file.path_in_project;
						new_definition.source_sound.is_official = is_official;

						auto& meta = new_definition.meta;
						(void)meta;
					}

					const auto [new_raw_flavour_id, new_flavour] = flavour_pool.allocate();
					const auto new_flavour_id = typed_entity_flavour_id<entity_type>(new_raw_flavour_id);

					const auto& editable = resource.editable;

					{
						auto& sound = new_flavour.template get<invariants::continuous_sound>();
						sound.effect.id = new_sound_id;
						sound.effect.modifier = static_cast<sound_effect_modifier>(editable);
					}

					/* Cache for quick and easy mapping */
					resource.scene_flavour_id = new_flavour_id;
				};

				create_as(sound_decoration());
			}
			else if constexpr(std::is_same_v<editor_light_resource, resource_type>) {

			}
			else {
				//static_assert(always_false_v<P>, "Non-exhaustive if constexpr");
			}
		}
	};

	project.resources .for_each([&](const auto& pool) { resource_pool_handler(pool, false); } );
	official_resources.for_each([&](const auto& pool) { resource_pool_handler(pool, true); } );

	/* Create nodes */

	auto total_order = sorting_order_type(0);

	for (const auto layer_id : reverse(project.layers.order)) {
		auto layer = find_layer(layer_id);
		ensure(layer != nullptr);

		auto node_handler = [&]<typename node_type>(const node_type& typed_node, const auto node_id) {
			typed_node.scene_entity_id.unset();

			if (!typed_node.visible || !layer->visible) {
				return;
			}

			const auto resource = find_resource(typed_node.resource_id);

			if (resource == nullptr) {
				return;
			}

			auto setup_entity_from_node = [&]<typename H>(const H& handle, auto& agg) {
				// using entity_type = typename H::used_entity_type;

				if (auto sorting_order = agg.template find<components::sorting_order>()) {
					sorting_order->order = total_order;
				}

				if (auto sprite = agg.template find<components::sprite>()) {
					const auto alpha_mult = layer->editable.alpha_mult;

					if (alpha_mult != 1.0f) {
						sprite->colorize.mult_alpha(alpha_mult);
					}
				}

				using Node = decltype(typed_node.editable);
				auto& node = typed_node.editable;

				if (auto geo = agg.template find<components::overridden_geo>()) {
					if constexpr(has_size_v<Node>) {
						if (bool(node.size)) {
							geo->size.emplace(node.size.value());
						}
					}

					if constexpr(has_flip_v<Node>) {
						geo->flip.horizontally = node.flip_horizontally;
						geo->flip.vertically = node.flip_vertically;
					}
				}

				handle.set_logic_transform(typed_node.get_transform());
			};

			auto entity_post_constructor = [&](auto&&...) {

			};

			if constexpr(
				std::is_same_v<editor_sprite_node, node_type> ||
				std::is_same_v<editor_sound_node, node_type>
			) {
				const auto new_id = cosmic::create_entity(
					scene.world,
					resource->scene_flavour_id,
					setup_entity_from_node,
					entity_post_constructor
				).get_id();

				const auto mapping_index = new_id.type_id.get_index();
				auto& mapping = scene_entity_to_node[mapping_index];

				ensure_eq(mapping.size(), std::size_t(new_id.raw.indirection_index));
				ensure_eq(decltype(new_id.raw.version)(1), new_id.raw.version);

				mapping.emplace_back(node_id.operator editor_node_id());
				typed_node.scene_entity_id = new_id;
			}
			else {
				//static_assert(always_false_v<P>, "Non-exhaustive if constexpr");
			}
		};

		for (const auto node_id : reverse(layer->hierarchy.nodes)) {
			on_node(node_id, node_handler);
		}
	}

	/* 
		We need to do this because ids stored in selection state
		might point to other entities after toggling visibility.
	*/

	inspected_to_entity_selector_state();
}

augs::path_type editor_setup::get_unofficial_content_dir() const {
	return paths.project_folder;
}

camera_eye editor_setup::get_camera_eye() const {
	return view.panned_camera;
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

				const auto& callout_name = typed_handle.get_name();

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

			::draw_area_indicator(typed_handle, lines, screen_space, eye.zoom, drawn_indicator_type::EDITOR, color);
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

void editor_setup::select_all_entities(const bool has_ctrl) {
	selector.select_all(
		scene.world,
		view.rect_select_mode,
		has_ctrl,
		entity_selector_state,
		render_layer_filter::all()
	);

	inspect_from_selector_state();
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

void editor_setup::start_moving_selection() {
	mover.start_moving_selection(make_mover_input());
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
	auto command = make_command_from_selected_nodes<delete_nodes_command>("Deleted ");

	if (!command.empty()) {
		post_new_command(std::move(command));
	}
}

template struct edit_resource_command<editor_sprite_resource>;
template struct edit_resource_command<editor_sound_resource>;
template struct edit_resource_command<editor_light_resource>;

template struct edit_node_command<editor_sprite_node>;
template struct edit_node_command<editor_sound_node>;
template struct edit_node_command<editor_light_node>;

template struct create_node_command<editor_sprite_node>;
template struct create_node_command<editor_sound_node>;
template struct create_node_command<editor_light_node>;
