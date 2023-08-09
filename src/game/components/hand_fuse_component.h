#pragma once
#include "augs/graphics/rgba.h"
#include "augs/misc/timing/stepped_timing.h"
#include "game/detail/view_input/sound_effect_input.h"
#include "augs/math/physics_structs.h"
#include "game/detail/view_input/particle_effect_input.h"
#include "game/enums/slot_function.h"
#include "game/enums/faction_type.h"

namespace components {
	struct hand_fuse {
		enum arming_source_type {
			NONE,

			THROW_INTENT,
			SHOOT_INTENT,
			SHOOT_SECONDARY_INTENT
		};

		// GEN INTROSPECTOR struct components::hand_fuse
		real32 fuse_delay_ms = 800.f;
		augs::stepped_timestamp when_armed;

		real32 amount_defused = -1.f;
		augs::stepped_timestamp when_started_arming;

		augs::stepped_timestamp when_last_beep;

		bool arming_requested = false;
		uint8_t arming_source = arming_source_type::NONE;
		pad_bytes<2> pad;

		signi_entity_id character_now_defusing;
		slot_function slot_when_armed = slot_function::INVALID;
		// END GEN INTROSPECTOR

		bool armed() const;
		bool defused() const;
	};
}

using arming_source_type = components::hand_fuse::arming_source_type;

namespace invariants {
	struct hand_fuse {
		// GEN INTROSPECTOR struct invariants::hand_fuse
		bool can_only_arm_at_bombsites = false;
		bool always_release_when_armed = false;
		bool must_stand_still_to_arm = false;
		bool override_release_impulse = false;

		real32 arming_duration_ms = -1.f;
		real32 defusing_duration_ms = -1.f;

		impulse_mults additional_release_impulse;
		impulse_mults additional_secondary_release_impulse;

		sound_effect_input beep_sound;
		real32 beep_time_mult = 0.05f;
		rgba beep_color = rgba(0, 0, 0, 0);

		sound_effect_input started_arming_sound;
		sound_effect_input started_defusing_sound;
		sound_effect_input armed_sound;
		std::array<sound_effect_input, 2> defused_sound;
		particle_effect_input defused_particles;
		sound_effect_input release_sound;

		assets::plain_animation_id armed_animation_id;
		assets::image_id defused_image_id;
		assets::image_id released_image_id;
		assets::physical_material_id released_physical_material;

		real32 circle_shape_radius_when_released = 9.f;

		per_actual_faction<sound_effect_input> radio_sounds_on_throw;
		// END GEN INTROSPECTOR

		bool has_delayed_arming() const;
		bool defusing_enabled() const;
		void set_bomb_vars(const float arm_ms, const float defuse_ms);
		bool is_like_plantable_bomb() const;
	};
}
