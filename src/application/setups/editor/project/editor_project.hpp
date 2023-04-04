#pragma once
#include "application/setups/editor/project/editor_project.h"

template <class S, class Officials, class F>
decltype(auto) editor_project::on_resource_impl(S& self, Officials& officials, const editor_resource_id& id, F&& callback) {
	if (id.is_official) {
		return officials.dispatch_on(id, std::forward<F>(callback));
	}

	return self.resources.dispatch_on(id, std::forward<F>(callback));
}

template <class S, class Officials, class T>
decltype(auto) editor_project::find_resource_impl(S& self, Officials& officials, const editor_typed_resource_id<T>& id) {
	if (id.is_official) {
		return officials.find_typed(id);
	}

	return self.resources.find_typed(id);
}

template <class T>
decltype(auto) editor_project::find_node(const editor_typed_node_id<T>& id) {
	return nodes.find_typed(id);
}

template <class T>
decltype(auto) editor_project::find_node(const editor_typed_node_id<T>& id) const {
	return nodes.find_typed(id);
}

template <class F>
decltype(auto) editor_project::on_node(const editor_node_id& id, F&& callback) {
	return nodes.dispatch_on(id, std::forward<F>(callback));
}

template <class F>
decltype(auto) editor_project::on_node(const editor_node_id& id, F&& callback) const {
	return nodes.dispatch_on(id, std::forward<F>(callback));
}

template <class T>
decltype(auto) editor_project::find_resource(editor_resource_pools& officials, const editor_typed_resource_id<T>& id) {
	return find_resource_impl(*this, officials, id);
}

template <class T>
decltype(auto) editor_project::find_resource(const editor_resource_pools& officials, const editor_typed_resource_id<T>& id) const {
	return find_resource_impl(*this, officials, id);
}

template <class F>
decltype(auto) editor_project::on_resource(editor_resource_pools& officials, const editor_resource_id& id, F&& callback) {
	return on_resource_impl(*this, officials, id, std::forward<F>(callback));
}

template <class F>
decltype(auto) editor_project::on_resource(const editor_resource_pools& officials, const editor_resource_id& id, F&& callback) const {
	return on_resource_impl(*this, officials, id, std::forward<F>(callback));
}

inline auto editor_project::make_find_resource_lambda(const editor_resource_pools& officials) const {
	return [&](auto&&... args) -> decltype(auto) { 
		return find_resource(officials, std::forward<decltype(args)>(args)... ); 
	};
}

inline auto editor_project::make_on_resource_lambda(const editor_resource_pools& officials) const {
	return [&](auto&&... args) -> decltype(auto) { 
		return on_resource(officials, std::forward<decltype(args)>(args)... ); 
	};
}

inline editor_layer* editor_project::find_layer(const editor_layer_id& id) {
	return layers.pool.find(id);
}

inline const editor_layer* editor_project::find_layer(const editor_layer_id& id) const {
	return layers.pool.find(id);
}
template <class R, class F>
void editor_project::for_each_resource(F&& callback) const {
	resources.template get_pool_for<R>().for_each_id_and_object(
		[&](const auto& raw_id, const auto& object) {
			const auto typed_id = editor_typed_resource_id<R>::from_raw(raw_id, false);

			callback(typed_id, object);
		}
	);
}

inline bool editor_project::rescan_pathed_resources_to_track(const O& officials, const editor_official_resource_map& officials_map) const {
	const bool any_references = recount_references(officials, false);
	const bool any_changes = mark_changed_resources(officials_map);

	return any_references || any_changes;
}
