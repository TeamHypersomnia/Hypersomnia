#pragma once

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
			node_color = object.editable.colorize;
		}
		else if constexpr(std::is_same_v<T, editor_light_node>) {
			node_color = object.editable.colorize;
		}
		else if constexpr(std::is_same_v<T, editor_particles_node>) {
			node_color = object.editable.colorize;
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
	}
	else if constexpr(std::is_same_v<T, editor_sprite_resource>) {
		return object.editable.color;
	}
	else if constexpr(std::is_same_v<T, editor_light_resource>) {
		return object.editable.color;
	}
	else if constexpr(std::is_same_v<T, editor_particles_resource>) {
		return object.editable.colorize;
	}

	return white;
}

template <class T>
editor_icon_info editor_setup::get_icon_for(
	const T& object, 
	const editor_icon_info_in in
) const {
	if constexpr(is_node_type_v<T>) {
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
	else if constexpr(std::is_same_v<T, editor_sprite_resource>) {
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
		return { in.necessary_images[assets::necessary_image_id::DETACHABLE_MAGAZINE_SLOT_ICON], augs::imgui_atlas_type::GAME };
	}
	else if constexpr(std::is_same_v<T, editor_particles_resource>) {
		if (object.editable.wandering.is_enabled) {
			return { in.necessary_images[assets::necessary_image_id::EDITOR_ICON_SMOKE_EFFECT], augs::imgui_atlas_type::GAME };
		}

		return { in.necessary_images[assets::necessary_image_id::EDITOR_ICON_PARTICLE_SOURCE], augs::imgui_atlas_type::GAME };
	}

	return { in.necessary_images[assets::necessary_image_id::DEFUSING_INDICATOR], augs::imgui_atlas_type::GAME };
}

