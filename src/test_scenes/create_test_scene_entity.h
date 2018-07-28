#pragma once
#include "test_scenes/test_scene_flavours.h"
#include "game/cosmos/create_entity.hpp"

template <class T>
auto transform_setter(const T& where) {
	return [where](const auto handle) {
		handle.set_logic_transform(where);		
	};
}

template <class C, class E, class F>
auto create_test_scene_entity(C& cosm, const E enum_flavour, F&& callback) {
	return cosmic::specific_create_entity(cosm, to_entity_flavour_id(enum_flavour), std::forward<F>(callback));
}

template <class C, class E>
auto create_test_scene_entity(C& cosm, const E enum_flavour, const vec2 pos) {
	return cosmic::specific_create_entity(cosm, to_entity_flavour_id(enum_flavour), transform_setter(pos));
}

template <class C, class E>
auto create_test_scene_entity(C& cosm, const E enum_flavour, const transformr where) {
	return cosmic::specific_create_entity(cosm, to_entity_flavour_id(enum_flavour), transform_setter(where));
}

template <class C, class E>
auto create_test_scene_entity(C& cosm, const E enum_flavour) {
	return cosmic::specific_create_entity(cosm, to_entity_flavour_id(enum_flavour), [](const auto) {});
}

