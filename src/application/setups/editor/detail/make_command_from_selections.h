#pragma once
#include "game/detail/describers.h"

template <class T, class F, class P>
T make_command_from_selections(
	const editor_setup& setup,
	F for_each_selected,
   	const std::string& preffix,
	const std::string& suffix,
   	P inclusion_predicate
) {
	T command;

	std::string last_name;

	for_each_selected(
		[&](const auto selected) {
			if (inclusion_predicate(selected)) {
				command.push_entry(selected);
				last_name = setup.get_name(selected);
			}
		}
	);

	if (command.size() == 1) {
		command.built_description = preffix + last_name;
	}
#if 0
	// TODO: pass the proper ALL count but this will be a pain in the ass and hard to interpret anyway
	// low prio tbh

	else if (command.size() == setup.get_node_count()) {
		command.built_description = preffix + "all " + suffix;
	}
#endif
	else {
		command.built_description = preffix + typesafe_sprintf("%x %x", command.size(), suffix);
	}

	return command;
}

template <class T, class F>
T make_command_from_selections(
	const editor_setup& setup,
	F for_each_selected,
	const std::string& preffix,
	const std::string& suffix
) {
	return make_command_from_selections<T>(setup, for_each_selected, preffix, suffix, [](const auto&) { return true; });
}

template <class T, class... Args>
auto editor_setup::make_command_from_selected_entities(const std::string& preffix, Args&&... args) const {
	return ::make_command_from_selections<T>(
		*this,
		[&](auto callback) {
			for_each_inspected_entity(
				[&](const auto id) {
					if (const auto handle = scene.world[id]) {
						callback(handle);
					}
				}
			);	
		},
		preffix,
		"nodes",
		std::forward<Args>(args)...
	);
}

template <class T, class... Args>
auto editor_setup::make_command_from_selected_nodes(const std::string& preffix, Args&&... args) const {
	return ::make_command_from_selections<T>(
		*this,
		[&](auto callback) {
			gui.inspector.for_each_inspected<editor_node_id>(callback);
		},
		preffix,
		"nodes",
		std::forward<Args>(args)...
	);
}

template <class T, class Node, class... Args>
auto editor_setup::make_command_from_selected_typed_nodes(const std::string& preffix, Args&&... args) const {
	return ::make_command_from_selections<T>(
		*this,
		[&](auto callback) {
			gui.inspector.for_each_inspected<editor_node_id>(
				[&](const editor_node_id& node_id) {
					callback(editor_typed_node_id<Node>::from_generic(node_id));
				}
			);
		},
		preffix,
		"nodes",
		std::forward<Args>(args)...
	);
}

template <class T, class Resource, class... Args>
auto editor_setup::make_command_from_selected_typed_resources(const std::string& preffix, const bool include_resources_from_selected_nodes, Args&&... args) const {
	return ::make_command_from_selections<T>(
		*this,
		[&](auto callback) {
			if (include_resources_from_selected_nodes) {
				thread_local std::unordered_set<editor_typed_resource_id<Resource>> seen;
				seen.clear();

				gui.inspector.for_each_inspected<editor_node_id>(
					[&](const editor_node_id& inspected_id) {
						on_node(inspected_id, [&](const auto& typed_node, const auto id) {
							(void)id;

							if (const auto resource = find_resource(typed_node.resource_id)) {
								if (!typed_node.resource_id.is_official) {
									const auto id = editor_typed_resource_id<Resource>::from_generic(editor_resource_id(typed_node.resource_id));

									if (!found_in(seen, id)) {
										seen.emplace(id);
										callback(id);
									}
								}
							}
						});
					}
				);
			}

			gui.inspector.for_each_inspected<editor_resource_id>(
				[&](const editor_resource_id& resource_id) {
					if (!resource_id.is_official) {
						callback(editor_typed_resource_id<Resource>::from_generic(resource_id));
					}
				}
			);
		},
		preffix,
		"resources",
		std::forward<Args>(args)...
	);
}

template <class T, class... Args>
auto editor_setup::make_command_from_selected_layers(const std::string& preffix, Args&&... args) const {
	return ::make_command_from_selections<T>(
		*this,
		[&](auto callback) {
			gui.inspector.for_each_inspected<editor_layer_id>(callback);
		},
		preffix,
		"layers",
		std::forward<Args>(args)...
	);
}
