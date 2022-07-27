#pragma once
#include "game/detail/describers.h"

template <class T, class F, class C, class P>
T make_command_from_selections(
	F for_each_selected,
	C& cosm,
   	const std::string& preffix,
   	P inclusion_predicate
) {
	T command;

	std::string last_name;

	for_each_selected(
		[&](const auto e) {
			const auto handle = cosm[e];

			if (inclusion_predicate(handle)) {
				command.push_entry(handle);
				last_name = handle.get_name();
			}
		}
	);

	if (command.size() == cosm.get_entities_count()) {
		command.built_description = preffix + "all entities";
	}
	else if (command.size() == 1) {
		command.built_description = preffix + last_name;
	}
	else {
		command.built_description = preffix + typesafe_sprintf("%x entities", command.size());
	}

	return command;
}

template <class T, class C, class F>
T make_command_from_selections(
	F for_each_selected,
	C& cosm,
	const std::string& preffix
) {
	return make_command_from_selections<T>(for_each_selected, cosm, preffix, [](const auto&) { return true; });
}

template <class T, class... Args>
auto editor_setup::make_command_from_selections(Args&&... args) const {
	return ::make_command_from_selections<T>(
		[&](auto callback) {
			for_each_inspected_entity(callback);	
		},
		scene.world,
		std::forward<Args>(args)...
	);
}
