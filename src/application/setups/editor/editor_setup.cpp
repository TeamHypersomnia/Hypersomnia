#include "game/cosmos/logic_step.h"
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

editor_setup::editor_setup(const augs::path_type& project_path) : paths(project_path) {
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

	if (in.e.msg == message::ldoubleclick) {
		handle_doubleclick_in_layers_gui = true;
	}

	if (in.e.msg == message::activate) {
		on_window_activate();
	}

	if (in.e.msg == message::deactivate) {
		force_autosave_now();
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

		if (has_ctrl) {
			if (has_shift) {
				switch (k) {
					case key::Z: redo(); return true;
					default: break;
				}
			}

			switch (k) {
				case key::Z: undo(); return true;
				//case key::C: copy(); return true;
				//case key::X: cut(); return true;
				//case key::V: paste(); return true;

				//case key::R: mover.rotate_selection_once_by(make_mover_input(), -90); return true;
				case key::F: gui.filesystem.open(); return true;
				case key::L: gui.layers.open(); return true;
				case key::H: gui.history.open(); return true;
				case key::I: gui.inspector.open(); return true;
				default: break;
			}
		}

		auto clamp_units = [&]() { view.grid.clamp_units(8, settings.grid.render.get_maximum_unit()); };

		if (!has_shift && !has_ctrl) {
			switch (k) {
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

	return false;
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
		files.fill_thumbnail_entries(paths.project_folder, new_subjects);
		files.root.for_each_file_recursive(
			[&](const auto& file_node) {
				on_resource(
					file_node.associated_resource,
					[&]<typename T>(T& typed_resource, const auto resource_id) {
						(void)resource_id;

						if constexpr(std::is_same_v<T, editor_sprite_resource>) {
							typed_resource.thumbnail_id = file_node.file_thumbnail_id;
						}
					}
				);
			}
		);

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
	gui.filesystem.clear_pointers();
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
	handle_pool(sound_pool, editor_filesystem_node_type::SOUND);

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

void editor_setup::inspect(inspected_variant new_inspected) {
	gui.inspector.inspect(new_inspected, wants_multiple_selection());
}

void editor_setup::inspect_only(inspected_variant new_inspected) {
	gui.inspector.inspect(new_inspected, false);
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

void editor_setup::seek_to_revision(editor_history::index_type revision_index) {
	history.seek_to_revision(revision_index, make_command_input());

	gui.filesystem.clear_drag_drop();
	rebuild_scene();
}

void editor_setup::undo_quiet() {
	history.undo(make_command_input());
}

void editor_setup::undo() {
	gui.history.scroll_to_current_once = true;
	undo_quiet();

	gui.filesystem.clear_drag_drop();
	rebuild_scene();
}

void editor_setup::redo() {
	gui.history.scroll_to_current_once = true;
	history.redo(make_command_input());

	gui.filesystem.clear_drag_drop();
	rebuild_scene();
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

editor_layer_id editor_setup::find_parent_layer(const editor_node_id node_id) const {
	for (const auto layer_id : project.layers.order) {
		if (const auto layer = find_layer(layer_id)) {
			if (found_in(layer->hierarchy.nodes, node_id)) {
				return layer_id;
			}
		}
	}

	return editor_layer_id();
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

void editor_setup::rebuild_scene() {
	scene.clear();

	const auto mutable_access = cosmos_common_significant_access();
	auto& common = scene.world.get_common_significant(mutable_access);
	(void)common;

	/* Create resources */

	auto resource_pool_handler = [&]<typename P>(const P& pool, const bool is_official) {
		using resource_type = typename P::value_type;

		auto& viewables = scene.viewables;

		for (const auto& resource : pool) {
			if constexpr(std::is_same_v<editor_sprite_resource, resource_type>) {
				auto create_in = [&]<typename entity_type>(entity_type) {
					auto& flavour_pool = scene.world.get_flavours<static_decoration>(mutable_access);
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
				create_in(static_decoration());
			}
			else if constexpr(std::is_same_v<editor_sound_resource, resource_type>) {

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

		if (!layer->visible) {
			continue;
		}

		auto node_handler = [&]<typename node_type>(node_type& typed_node, const auto id) {
			(void)id;

			if (!typed_node.visible) {
				return;
			}

			const auto resource = find_resource(typed_node.resource_id);

			if (resource == nullptr) {
				return;
			}

			auto entity_pre_constructor = [&]<typename H>(const H& handle, auto& agg) {
				// using entity_type = typename H::used_entity_type;

				if (auto sorting_order = agg.template find<components::sorting_order>()) {
					sorting_order->order = total_order;
				}

				handle.set_logic_transform(typed_node.get_transform());
			};

			auto entity_post_constructor = [&](auto&&...) {

			};

			if constexpr(std::is_same_v<editor_sprite_node, node_type>) {

				cosmic::create_entity(
					scene.world,
					resource->scene_flavour_id,
					entity_pre_constructor,
					entity_post_constructor
				);
			}
			else {
				//static_assert(always_false_v<P>, "Non-exhaustive if constexpr");
			}
		};

		for (const auto node_id : reverse(layer->hierarchy.nodes)) {
			on_node(node_id, node_handler);
		}
	}
}

augs::path_type editor_setup::get_unofficial_content_dir() const {
	return paths.project_folder;
}

camera_eye editor_setup::get_camera_eye() const {
	return view.panned_camera;
}

void editor_setup::draw_custom_gui(const draw_setup_gui_input& in) { 
	auto cone = in.cone;

	auto& eye = cone.eye;
	eye.transform.pos.discard_fract();

	auto triangles = in.get_drawer();
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
}

void editor_setup::apply(const config_lua_table& cfg) {
	settings = cfg.editor;
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
