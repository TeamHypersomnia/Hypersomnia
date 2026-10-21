#pragma once
#include <unordered_map>
#include "game/assets/ids/asset_ids.h"
#include "augs/math/vec2.h"
#include "game/components/shape_polygon_component.h"
#include "game/assets/image_offsets.h"
#include "augs/templates/container_templates.h"

struct image_definition_view;

struct image_cache {
	image_cache() = default;

	explicit image_cache(
		const image_definition_view&
	);

	vec2u original_image_size;

	/*
		The custom collider from the image's meta, empty if the body is a box of the image's size.
		Lets the flavours compute the mass of a body before any cosmos exists.
	*/

	image_shape_type non_standard_shape;

	vec2u get_original_size() const {
		return original_image_size;
	}
};

class loaded_image_caches_map {
	using map_type = std::unordered_map<assets::image_id, image_cache>;
	map_type caches;

public:
	const image_cache* find(const assets::image_id id) const {
		return mapped_or_nullptr(caches, id);
	}

	decltype(auto) at(const assets::image_id id) {
		return caches.at(id);
	}

	decltype(auto) at(const assets::image_id id) const {
		return caches.at(id);
	}

	auto size() const {
		return caches.size();
	}

	void clear() {
		caches.clear();
	}

	void erase(const assets::image_id id) {
		caches.erase(id);
	}

	template <class... Args>
	auto try_emplace(const assets::image_id id, Args&&... args) {
		caches.try_emplace(id, std::forward<Args>(args)...);
	}
};

