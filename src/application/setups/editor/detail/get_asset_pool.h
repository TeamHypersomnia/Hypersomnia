#pragma once
#include "augs/templates/identity_templates.h"
#include "game/assets/ids/asset_ids.h"

template <class I, class T>
decltype(auto) get_logicals_pool(T&& t) {
	if constexpr(std::is_same_v<I, assets::recoil_player_id>) {
		return (t.recoils);
	}
	else if constexpr(std::is_same_v<I, assets::physical_material_id>) {
		return (t.physical_materials);
	}
	else {
		return always_false<I>();
	}
}

template <class I, class T>
decltype(auto) get_viewable_pool(T&& t) {
	if constexpr(std::is_same_v<I, assets::image_id>) {
		return (t.image_definitions);
	}
	else if constexpr(std::is_same_v<I, assets::sound_id>) {
		return (t.sounds);
	}
	else if constexpr(std::is_same_v<I, assets::particle_effect_id>) {
		return (t.particle_effects);
	}
	else {
		return always_false<I>();
	}
}

template <class T, class V, class L>
auto& get_asset_pool(V&& viewables, L&& logicals) {
	if constexpr(!std::is_same_v<decltype(get_viewable_pool<T>(viewables)), always_false<T>>) {
		return get_viewable_pool<T>(viewables);
	}
	else {
		return get_logicals_pool<T>(logicals);
	}
}

template <class T, class E>
auto& get_asset_pool(E&& cmd_in) {
	return get_asset_pool<T>(cmd_in.folder.work->viewables, cmd_in.folder.work->world.get_logical_assets());
}
