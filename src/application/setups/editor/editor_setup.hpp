#pragma once
#include "application/setups/editor/editor_setup.h"
#include "application/setups/editor/resources/editor_typed_resource_id.h"

template <class T>
decltype(auto) editor_setup::post_new_command(T&& command) {
	auto rebuild_after = augs::scope_guard([this]() { 
		rebuild_scene(); 
	
		if constexpr(std::is_base_of_v<allocating_command<editor_node_pool_id>, T>) {
			/*
				This is already called by inspect_only (which is called during redo of the command),
				however the inspect_only is called before the scene is rebuilt,
				causing the result selector state to contain invalid entity_ids.

				This is why we have to manually call it after the command completes.
			*/

			inspected_to_entity_selector_state();
		}
	});

	gui.history.scroll_to_latest_once = true;
	return history.execute_new(std::forward<T>(command), make_command_input(true));
}

template <class T>
decltype(auto) editor_setup::rewrite_last_command(T&& command) {
	auto rebuild_after = augs::scope_guard([this]() { rebuild_scene(); });

	history.undo(make_command_input(true));
	return history.execute_new(std::forward<T>(command), make_command_input(true));
}

template <class S, class F>
decltype(auto) editor_setup::on_resource_impl(S& self, const editor_resource_id& id, F&& callback) {
	if (id.is_official) {
		return self.official_resources.dispatch_on(id, std::forward<F>(callback));
	}

	return self.project.resources.dispatch_on(id, std::forward<F>(callback));
}

template <class S, class T>
decltype(auto) editor_setup::find_resource_impl(S& self, const editor_typed_resource_id<T>& id) {
	if (id.is_official) {
		return self.official_resources.find_typed(id);
	}

	return self.project.resources.find_typed(id);
}

template <class T>
decltype(auto) editor_setup::find_node(const editor_typed_node_id<T>& id) {
	return project.nodes.find_typed(id);
}

template <class T>
decltype(auto) editor_setup::find_node(const editor_typed_node_id<T>& id) const{
	return project.nodes.find_typed(id);
}

template <class T>
decltype(auto) editor_setup::find_resource(const editor_typed_resource_id<T>& id) {
	return find_resource_impl(*this, id);
}

template <class T>
decltype(auto) editor_setup::find_resource(const editor_typed_resource_id<T>& id) const {
	return find_resource_impl(*this, id);
}

template <class F>
decltype(auto) editor_setup::on_resource(const editor_resource_id& id, F&& callback) {
	return on_resource_impl(*this, id, std::forward<F>(callback));
}

template <class F>
decltype(auto) editor_setup::on_resource(const editor_resource_id& id, F&& callback) const {
	return on_resource_impl(*this, id, std::forward<F>(callback));
}

template <class F>
decltype(auto) editor_setup::on_node(const editor_node_id& id, F&& callback) {
	return project.nodes.dispatch_on(id, std::forward<F>(callback));
}

template <class F>
decltype(auto) editor_setup::on_node(const editor_node_id& id, F&& callback) const {
	return project.nodes.dispatch_on(id, std::forward<F>(callback));
}