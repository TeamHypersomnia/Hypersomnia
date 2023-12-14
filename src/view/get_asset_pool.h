#pragma once
#include "game/assets/animation.h"
#include "game/assets/animation_templates.h"
#include "game/assets/get_logicals_pool.h"

#include "game/cosmos/cosmos_common_significant_access.h"

struct image_definition;
struct sound_definition;
struct particle_effect;

template <class I, class T>
decltype(auto) get_viewable_pool(T&& t) {
	if constexpr(is_one_of_v<I, assets::image_id, image_definition>) {
		return (t.image_definitions);
	}
	else if constexpr(is_one_of_v<I, assets::sound_id, sound_definition>) {
		return (t.sounds);
	}
	else if constexpr(is_one_of_v<I, assets::particle_effect_id, particle_effect>) {
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
	return get_asset_pool<T>(cmd_in.folder.commanded->work.viewables, cmd_in.folder.commanded->work.world.get_logical_assets());
}

template <class T, class E>
auto& access_asset_pool(E&& cmd_in, cosmos_common_significant_access access) {
	return get_asset_pool<T>(cmd_in.folder.commanded->work.viewables, cmd_in.folder.commanded->work.world.get_logical_assets(access));
}

template <class T>
decltype(auto) get_displayed_name(const T& object) {
	if constexpr(std::is_same_v<T, sound_definition>) {
		return cut_trailing_number_and_spaces(object.get_loadable_path().get_prettified_filename());
	}
	else {
		return object.get_loadable_path().get_prettified_filename();
	}
}

template <class T, class D>
decltype(auto) get_displayed_name(const T& object, const D& image_defs) {
	if constexpr(has_frames_v<T>) {
		const auto image_id = object.frames[0].image_id;
		return cut_trailing_number_and_spaces(::get_displayed_name(image_defs[image_id])) + " " + object.name_suffix;
	}
	else {
		return object.get_name();
	}
}
