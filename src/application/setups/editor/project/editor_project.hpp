#pragma once
#include <cstddef>
#include <unordered_set>
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
T* editor_project::find_node(const editor_node_id& generic_id) {
	if (generic_id.type_id.is<T>()) {
		return find_node(editor_typed_node_id<T>::from_generic(generic_id));
	}

	return nullptr;
}

template <class T>
const T* editor_project::find_node(const editor_node_id& generic_id) const {
	if (generic_id.type_id.is<T>()) {
		return find_node(editor_typed_node_id<T>::from_generic(generic_id));
	}

	return nullptr;
}

template <class T>
T* editor_project::find_node(const editor_typed_node_id<T>& id) {
	return nodes.find_typed(id);
}

template <class T>
const T* editor_project::find_node(const editor_typed_node_id<T>& id) const {
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
T* editor_project::find_resource(editor_resource_pools& officials, const editor_typed_resource_id<T>& id) {
	return find_resource_impl(*this, officials, id);
}

template <class T>
const T* editor_project::find_resource(const editor_resource_pools& officials, const editor_typed_resource_id<T>& id) const {
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

inline editor_layer* editor_project::find_layer(const std::string& name) {
	for (auto& layer : layers.pool) {
		if (layer.unique_name == name) {
			return std::addressof(layer);
		}
	}

	return nullptr;
}

inline editor_layer_id editor_project::find_layer_id(const std::string& name) const {
	editor_layer_id found;

	layers.pool.for_each_id_and_object(
		[&](const auto& id, const auto& object) {
			if (object.unique_name == name) {
				found = id;
			}
		}
	);

	return found;
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

inline bool editor_project::rescan_resources_to_track(const O& officials, const editor_official_resource_map& officials_map) const {
	const bool any_references_to_pathed = recount_references(officials, false);
	const bool any_changes_to_pathed = mark_changed_resources(officials_map);

	return any_references_to_pathed || any_changes_to_pathed;
}

inline name_to_node_map_type editor_project::make_name_to_node_map() const {
	std::unordered_map<std::string, editor_node_id> result;

	nodes.for_each(
		[&](const auto& node_pool) {
			auto register_node = [&]<typename T>(const auto id, const T& object) {
				result[object.get_display_name()] = editor_typed_node_id<T> { id }.operator editor_node_id();
			};

			node_pool.for_each_id_and_object(register_node);
		}
	);

	return result;
}

inline std::optional<parent_layer_info> editor_project::convert_to_parent_layer_info(const editor_layer_id layer_id) const {
	const auto& order = layers.order;

	for (std::size_t l = 0; l < order.size(); ++l) {
		if (layer_id == order[l]) {
			if (const auto layer = find_layer(layer_id)) {
				return parent_layer_info { layer_id, layer, l, 0, false };
			}
		}
	}

	const auto& scratchpad = layers.scratchpad;

	for (std::size_t l = 0; l < scratchpad.size(); ++l) {
		if (layer_id == scratchpad[l]) {
			if (const auto layer = find_layer(layer_id)) {
				return parent_layer_info { layer_id, layer, order.size() + l, 0, true };
			}
		}
	}

	return std::nullopt;
}

inline bool editor_project::is_scratchpad_layer(const editor_layer_id id) const {
	return found_in(layers.scratchpad, id);
}

inline bool editor_project::is_in_scratchpad(const editor_node_id id) const {
	const auto parent = find_parent_layer(id);
	return parent.has_value() && parent->in_scratchpad;
}

inline std::unordered_set<editor_node_id> editor_project::make_scratchpad_node_set() const {
	std::unordered_set<editor_node_id> result;

	for (const auto layer_id : layers.scratchpad) {
		if (const auto layer = find_layer(layer_id)) {
			result.insert(layer->hierarchy.nodes.begin(), layer->hierarchy.nodes.end());
		}
	}

	return result;
}

inline std::optional<parent_layer_info> editor_project::find_parent_layer(const editor_node_id node_id) const {
	if (!node_id.is_set()) {
		return std::nullopt;
	}

	/*
		The scratchpad's layers too - the editor's commands work on their nodes like on any other.
	*/

	auto search = [&](const std::vector<editor_layer_id>& list, const std::size_t first_index, const bool in_scratchpad) -> std::optional<parent_layer_info> {
		for (std::size_t l = 0; l < list.size(); ++l) {
			const auto layer_id = list[l];

			if (const auto layer = find_layer(layer_id)) {
				const auto& nodes = layer->hierarchy.nodes;

				for (std::size_t i = 0; i < nodes.size(); ++i) {
					if (nodes[i] == node_id) {
						return parent_layer_info { layer_id, layer, first_index + l, i, in_scratchpad };
					}
				}
			}
		}

		return std::nullopt;
	};

	if (const auto found = search(layers.order, 0, false)) {
		return found;
	}

	return search(layers.scratchpad, layers.order.size(), true);
}

inline void editor_project::clear_cached_scene_node_data() {
	nodes.for_each(
		[&](auto& pool) {
			for (auto& node : pool) {
				node.scene_entity_id.unset();
			}
		}
	);
}
