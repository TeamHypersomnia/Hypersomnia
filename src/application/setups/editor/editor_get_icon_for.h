#pragma once
#include "view/viewables/images_in_atlas_map.h"
#include "application/setups/editor/has_thumbnail_id.h"
#include "view/rendering_scripts/for_each_iconed_entity.h"

struct editor_icon_info {
	augs::atlas_entry icon;
	augs::imgui_atlas_type atlas;
};

struct editor_icon_info_in {
	const necessary_images_in_atlas_map& necessary_images;
	const ad_hoc_in_atlas_map& ad_hoc_atlas;

	template <class T>
	editor_icon_info_in(const T& in) : necessary_images(in.necessary_images), ad_hoc_atlas(in.ad_hoc_atlas) {};
};

template <class T, class = void>
struct has_resource_id : std::false_type {};

template <class T>
struct has_resource_id<T, decltype(std::declval<T&>().resource_id, void())> : std::true_type {};

template <class T>
constexpr bool is_node_type_v = has_resource_id<T>::value;

template <class T>
rgba editor_setup::get_icon_color_for(
	const T& object
) const {
	if constexpr(is_node_type_v<T>) {
		auto node_color = white;

		if constexpr(std::is_same_v<T, editor_sprite_node>) {
			node_color = object.editable.color;
		}
		else if constexpr(std::is_same_v<T, editor_light_node>) {
			node_color = object.editable.color;
		}
		else if constexpr(std::is_same_v<T, editor_particles_node>) {
			node_color = object.editable.color;
		}
		else if constexpr(std::is_same_v<T, editor_wandering_pixels_node>) {
			node_color = object.editable.color;
		}
		else if constexpr(is_one_of_v<T, editor_point_marker_node, editor_area_marker_node>) {
			const auto* maybe_resource = find_resource(object.resource_id);

			if (maybe_resource != nullptr) {
				marker_icon icon;

				auto get_faction_color = [&](const auto faction) {
					return faction_view.colors[faction].standard;
				};

				if constexpr(std::is_same_v<T, editor_point_marker_node>) {
					icon = marker_icon::from_point(maybe_resource->editable, object.editable, get_faction_color);
				}
				else if constexpr(std::is_same_v<T, editor_area_marker_node>) {
					icon = marker_icon::from_area(maybe_resource->editable, object.editable, get_faction_color);

					if (::is_portal_based(maybe_resource->editable.type)) {
						icon.col = object.editable.as_portal.get_icon_color();
					}
				}

				return icon.col;
			}
		}

		const auto* maybe_resource = find_resource(object.resource_id);

		if (maybe_resource != nullptr) {
			return node_color * get_icon_color_for(*maybe_resource);
		}

		return node_color;
	}
	else if constexpr(std::is_same_v<T, editor_resource_id>) {
		auto icon_getter = [&](const auto& typed_resource, const auto) { 
			return get_icon_color_for(typed_resource);
		};

		if (const auto result = on_resource(object, icon_getter)) {
			return *result;
		}
		else {
			return rgba(0, 0, 0, 0);
		}
	}
	else if constexpr(std::is_same_v<T, editor_node_id>) {
		auto icon_getter = [&](const auto& typed_resource, const auto) { 
			return get_icon_color_for(typed_resource);
		};

		if (const auto result = on_node(object, icon_getter)) {
			return *result;
		}
		else {
			return rgba(0, 0, 0, 0);
		}
	}
	else if constexpr(std::is_same_v<T, editor_sprite_resource>) {
		return object.editable.color;
	}
	else if constexpr(std::is_same_v<T, editor_light_resource>) {
		return white;
	}
	else if constexpr(std::is_same_v<T, editor_particles_resource>) {
		return object.editable.color;
	}
	else if constexpr(std::is_same_v<T, editor_material_resource>) {
		return white;
	}
	else if constexpr(std::is_same_v<T, editor_prefab_resource>) {
		return white;
	}
	else if constexpr(std::is_same_v<T, editor_game_mode_resource>) {
		return white;
	}
	else {
		const auto sprite = std::visit(
			[&](const auto& typed) -> const invariants::sprite* {
				if (!typed.is_set()) {
					return nullptr;
				}

				return scene.world.get_flavour(typed).template find<invariants::sprite>();
			},

			object.scene_flavour_id
		);

		if (sprite) {
			return sprite->color;
		}
	}

	return white;
}

template <class T>
editor_icon_info editor_setup::get_icon_for(
	const T& object, 
	const editor_icon_info_in in
) const {
	if constexpr(is_one_of_v<T, editor_point_marker_node, editor_area_marker_node>) {
		const auto* maybe_resource = find_resource(object.resource_id);

		if (maybe_resource != nullptr) {
			marker_icon icon;

			auto get_faction_color = [&](const auto faction) {
				return faction_view.colors[faction].standard;
			};

			if constexpr(std::is_same_v<T, editor_point_marker_node>) {
				icon = marker_icon::from_point(maybe_resource->editable, object.editable, get_faction_color);
			}
			else if constexpr(std::is_same_v<T, editor_area_marker_node>) {
				icon = marker_icon::from_area(maybe_resource->editable, object.editable, get_faction_color);
			}

			return { in.necessary_images[icon.id], augs::imgui_atlas_type::GAME };
		}
	}
	else if constexpr(is_node_type_v<T>) {
		const auto* maybe_resource = find_resource(object.resource_id);

		if (maybe_resource != nullptr) {
			return get_icon_for(*maybe_resource, in);
		}
	}
	else if constexpr(std::is_same_v<T, editor_resource_id>) {
		auto icon_getter = [&](const auto& typed_resource, const auto) { 
			return get_icon_for(typed_resource, in);
		};

		if (const auto result = on_resource(object, icon_getter)) {
			return *result;
		}
	}
	else if constexpr(std::is_same_v<T, editor_node_id>) {
		auto icon_getter = [&](const auto& typed_node, const auto) { 
			return get_icon_for(typed_node, in);
		};

		if (const auto result = on_node(object, icon_getter)) {
			return *result;
		}
	}
	else if constexpr(has_thumbnail_id_v<T>) {
		if (auto ad_hoc = mapped_or_nullptr(in.ad_hoc_atlas, object.thumbnail_id)) {
			return { *ad_hoc, augs::imgui_atlas_type::AD_HOC };
		}

		return { in.necessary_images[assets::necessary_image_id::EDITOR_ICON_FILE], augs::imgui_atlas_type::GAME };
	}
	else if constexpr(std::is_same_v<T, editor_light_resource>) {
		return { in.necessary_images[assets::necessary_image_id::EDITOR_ICON_LIGHT], augs::imgui_atlas_type::GAME };
	}
	else if constexpr(std::is_same_v<T, editor_sound_resource>) {
		return { in.necessary_images[assets::necessary_image_id::EDITOR_ICON_SOUND], augs::imgui_atlas_type::GAME };
	}
	else if constexpr(std::is_same_v<T, editor_material_resource>) {
		return { in.necessary_images[assets::necessary_image_id::EDITOR_ICON_MATERIAL], augs::imgui_atlas_type::GAME };
	}
	else if constexpr(is_one_of_v<T, editor_prefab_resource>) {
		return { in.necessary_images[assets::necessary_image_id::SPELL_BORDER], augs::imgui_atlas_type::GAME };
	}
	else if constexpr(is_one_of_v<T, editor_game_mode_resource>) {
		if (object.type.template is<editor_bomb_defusal_mode>()) {
			return { in.necessary_images[assets::necessary_image_id::BOMB_INDICATOR], augs::imgui_atlas_type::GAME };
		}

		if (object.type.template is<editor_gun_game_mode>()) {
			return { in.necessary_images[assets::necessary_image_id::DEATH_INDICATOR], augs::imgui_atlas_type::GAME };
		}

		if (object.type.template is<editor_quick_test_mode>()) {
			return { in.necessary_images[assets::necessary_image_id::EDITOR_TOOL_PLAYTEST], augs::imgui_atlas_type::GAME };
		}

		return { in.necessary_images[assets::necessary_image_id::EDITOR_TOOL_HOST_SERVER], augs::imgui_atlas_type::GAME };
	}
	else if constexpr(is_one_of_v<T, editor_point_marker_resource, editor_area_marker_resource>) {
		marker_icon icon;

		if constexpr(std::is_same_v<T, editor_point_marker_resource>) {
			icon = marker_icon::from_point(object.editable, decltype(editor_point_marker_node::editable)(), [](auto&&...) { return white; });
		}
		else if constexpr(std::is_same_v<T, editor_area_marker_resource>) {
			icon = marker_icon::from_area(object.editable, decltype(editor_area_marker_node::editable)(), [](auto&&...) { return white; });
		}

		return { in.necessary_images[icon.id], augs::imgui_atlas_type::GAME };
	}
	else if constexpr(std::is_same_v<T, editor_particles_resource>) {
		if (object.editable.wandering.is_enabled) {
			return { in.necessary_images[assets::necessary_image_id::EDITOR_ICON_SMOKE_EFFECT], augs::imgui_atlas_type::GAME };
		}

		return { in.necessary_images[assets::necessary_image_id::EDITOR_ICON_PARTICLE_SOURCE], augs::imgui_atlas_type::GAME };
	}
	else if constexpr(std::is_same_v<T, editor_wandering_pixels_resource>) {
		return { in.necessary_images[assets::necessary_image_id::EDITOR_ICON_WANDERING_PIXELS], augs::imgui_atlas_type::GAME };
	}

	return { {}, augs::imgui_atlas_type::GAME };
}

