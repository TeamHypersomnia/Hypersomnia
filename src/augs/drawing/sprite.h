#pragma once
#include <compare>
#include "augs/pad_bytes.h"

#include "augs/templates/maybe.h"
#include "augs/drawing/make_sprite_points.h"
#include "augs/math/transform.h"
#include "augs/math/rects.h"

#include "augs/templates/traits/container_traits.h"

#include "augs/graphics/rgba.h"

struct intensity_vibration_input {
	// GEN INTROSPECTOR struct intensity_vibration_input
	real32 change_per_sec = 100.f;
	rgba_channel lower = 150;
	rgba_channel upper = 255;
	pad_bytes<2> pad;
	// END GEN INTROSPECTOR

	bool operator==(const intensity_vibration_input&) const = default;
};

struct sprite_drawing_input;

using sprite_size_type = vec2i;

namespace augs {
	enum class sprite_special_effect /* : unsigned char */ {
		// GEN INTROSPECTOR enum class augs::sprite_special_effect
		NONE = 0,
		COLOR_WAVE,
		CONTINUOUS_ROTATION
		// END GEN INTROSPECTOR
	};

	template <class id_type>
	struct sprite {
		static constexpr bool reinfer_when_tweaking = true;
		using size_type = sprite_size_type;

		using drawing_input = sprite_drawing_input;

		static std::string get_custom_type_name() {
			return "sprite";
		}

		sprite(
			const id_type image_id = {},
			const size_type size = size_type(),
			const rgba color = white
		) :
			image_id(image_id),
			size(size),
			color(color),
			neon_color(color)
		{}

		template <class M>
		sprite(
			const id_type image_id,
			const M& manager,
			const rgba color = white
		) {
			set(image_id, manager, color);
		}

		template <class M>
		void set(
			const id_type image_id,
			const M& manager,
			const rgba color = white
		) {
			this->image_id = image_id;
			this->size = manager.at(image_id).get_original_size();
			this->color = color;
			this->neon_color = color;
		}

		template <class T>
		void set(
			const id_type image_id,
			const basic_vec2<T> size,
			const rgba color = white
		) {
			this->image_id = image_id;
			this->size = size;
			this->color = color;
			this->neon_color = color;
		}
		
		void set_color(const rgba col) {
			color = col;
		}

		// GEN INTROSPECTOR struct augs::sprite class id_type
		id_type image_id;
		size_type size;
		rgba color = white;
		rgba neon_color = white;
		sprite_special_effect effect = sprite_special_effect::NONE;
		real32 effect_speed_multiplier = 1.f;

		augs::maybe<intensity_vibration_input> neon_intensity_vibration;
		bool vibrate_diffuse_as_well = false;
		bool tile_excess_size = false;
		pad_bytes<2> pad;
		// END GEN INTROSPECTOR

		bool has_color_wave() const {
			return effect == sprite_special_effect::COLOR_WAVE;
		}

		bool operator==(const sprite<id_type>&) const = default;

		size_type get_size() const {
			return size;
		}

		ltrb get_aabb(const transformr where) const {
			return augs::calc_sprite_aabb(where, size);
		}
	};
}