#pragma once

template <class T>
struct recurse_to_find_resource_ids {
	static constexpr bool value = 
		!is_one_of_v<T, editor_node_pools, editor_resource_pools, editor_layers>
		&& !augs::has_custom_to_json_value_v<T>;
};

template <class O, class F>
void on_each_resource_id_in_object(O& object, F callback) {
	auto handle = [&](auto& field) {
		using Field = remove_cref<decltype(field)>;

		if constexpr(is_editor_typed_resource_id_v<Field>) {
			callback(field);
		}
	};

	augs::on_each_object_in_object<recurse_to_find_resource_ids>(object, handle);
}

template <class P, class F>
void on_each_resource_id_in_project(P& project, F callback) {
	auto traverse = [&](auto& object) {
		on_each_resource_id_in_object(object, callback);
	};

	traverse(project);

	project.resources.pools.for_each(
		[&](auto& resource) {
			traverse(resource.editable);
		}
	);

	project.nodes.pools.for_each(
		[&](auto& node) {
			callback(node.resource_id);
			traverse(node.editable);
		}
	);

	for (auto& layer : project.layers.pool) {
		traverse(layer.editable);
	}
}

