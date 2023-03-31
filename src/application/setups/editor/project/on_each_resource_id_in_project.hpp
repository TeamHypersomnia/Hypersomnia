#pragma once

template <class T>
struct recurse_to_find_ids {
	static constexpr bool value = !augs::json_ignore_v<T> && !augs::has_custom_to_json_value_v<T>;
};

template <class P, class F>
void on_each_resource_id_in_project(P& project, F callback) {
	auto handle = [&](auto& object) {
		using Field = remove_cref<decltype(object)>;

		if constexpr(is_editor_typed_resource_id_v<Field>) {
			callback(object);
		}
	};

	auto traverse_object = [&](auto& object) {
		augs::on_each_object_in_object<recurse_to_find_ids>(object, handle);
	};

	traverse_object(project);

	project.resources.pools.for_each(
		[&](auto& resource) {
			traverse_object(resource.editable);
		}
	);

	project.nodes.pools.for_each(
		[&](auto& node) {
			callback(node.resource_id);
			traverse_object(node.editable);
		}
	);

	for (auto& layer : project.layers.pool) {
		traverse_object(layer.editable);
	}
}

