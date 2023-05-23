#pragma once
#include "application/setups/editor/editor_setup.h"
#include "application/setups/editor/resources/editor_typed_resource_id.h"
#include "application/setups/editor/project/editor_project.hpp"

template <class T>
constexpr bool skip_scene_rebuild_v = is_one_of_v<T,
	inspect_command
>;

/*
	Renames and transformations should be safe.
	Others, not so much, they might edit some resource property.
*/

template <class T>
constexpr bool skip_missing_resources_check_v = is_one_of_v<T,
	inspect_command,

	rename_node_command,
	rename_layer_command,

	reorder_nodes_command,
	reorder_layers_command,

	move_nodes_command,
	resize_nodes_command,
	flip_nodes_command,
	toggle_nodes_active_command,
	toggle_layers_active_command
>;

template <class T>
const T& editor_setup::post_new_command(T&& command) {
	gui.history.scroll_to_latest_once = true;
	const T& result = history.execute_new(std::forward<T>(command), make_command_input(true));

	if constexpr(!skip_scene_rebuild_v<T>) {
		rebuild_arena();
	}

	if constexpr(!skip_missing_resources_check_v<T>) {
		on_resource_references_changed();
	}

	if constexpr(std::is_base_of_v<allocating_command<editor_node_pool_id>, T>) {
		/*
			This is already called by inspect_only (which is called during redo of the command),
			however the inspect_only is called before the scene is rebuilt,
			causing the result selector state to contain invalid entity_ids.

			This is why we have to manually call it after the command completes.
		*/

		inspected_to_entity_selector_state();
	}

	return result;
}

template <class T>
const T& editor_setup::rewrite_last_command(T&& command) {
	history.undo(make_command_input(true));
	const T& result = history.execute_new(std::forward<T>(command), make_command_input(true));

	rebuild_arena(); 

	if constexpr(!skip_missing_resources_check_v<remove_cref<T>>) {
		on_resource_references_changed();
	}

	return result;
}

template <class R, class F>
void editor_setup::for_each_resource(F&& callback, bool for_official) const {
	const auto& pools = for_official ? get_official_resources() : project.resources;

	pools.template get_pool_for<R>().for_each_id_and_object(
		[&](const auto& raw_id, const auto& object) {
			const auto typed_id = editor_typed_resource_id<R>::from_raw(raw_id, for_official);

			callback(typed_id, object);
		}
	);
}

template <class N, class F>
void editor_setup::for_each_node(F&& callback) const {
	project.nodes.template get_pool_for<N>().for_each_id_and_object(
		[&](const auto& raw_id, const auto& object) {
			const auto typed_id = editor_typed_node_id<N>::from_raw(raw_id);

			callback(typed_id, object);
		}
	);
}

template <class T>
decltype(auto) editor_setup::find_node(const editor_typed_node_id<T>& id) {
	return project.find_node(id);
}

template <class T>
decltype(auto) editor_setup::find_node(const editor_typed_node_id<T>& id) const {
	return project.find_node(id);
}

template <class T>
decltype(auto) editor_setup::find_resource(const editor_typed_resource_id<T>& id) {
	return project.find_resource(get_mut_official_resources(), id);
}

template <class T>
decltype(auto) editor_setup::find_resource(const editor_typed_resource_id<T>& id) const {
	return project.find_resource(get_official_resources(), id);
}

template <class F>
decltype(auto) editor_setup::on_resource(const editor_resource_id& id, F&& callback) {
	return project.on_resource(get_mut_official_resources(), id, std::forward<F>(callback));
}

template <class F>
decltype(auto) editor_setup::on_resource(const editor_resource_id& id, F&& callback) const {
	return project.on_resource(get_official_resources(), id, std::forward<F>(callback));
}

template <class F>
decltype(auto) editor_setup::on_node(const editor_node_id& id, F&& callback) {
	return project.on_node(id, std::forward<F>(callback));
}

template <class F>
decltype(auto) editor_setup::on_node(const editor_node_id& id, F&& callback) const {
	return project.on_node(id, std::forward<F>(callback));
}