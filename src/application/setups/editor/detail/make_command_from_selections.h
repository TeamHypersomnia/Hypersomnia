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
	else if (command.size() == setup.get_node_count()) {
		command.built_description = preffix + "all " + suffix;
	}
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
