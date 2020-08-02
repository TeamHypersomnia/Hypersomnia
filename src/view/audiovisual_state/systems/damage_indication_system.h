#pragma once
#include <optional>

#include "augs/math/vec2.h"
#include "augs/misc/timing/delta.h"

#include "augs/graphics/rgba.h"
#include "augs/math/camera_cone.h"

#include "game/messages/health_event.h"
#include "view/view_container_sizes.h"
#include "augs/misc/constant_size_vector.h"
#include "augs/misc/constant_size_string.h"
#include "view/gui_fonts.h"

class images_in_atlas_map;
class interpolation_system;

namespace augs {
	struct baked_font;
	struct drawer;
}

struct damage_indication_settings;

class damage_indication_system {
public:
	struct damage_event {
		enum class event_type {
			HEALTH,
			SHIELD,
			CRITICAL
		};

		struct input {
			event_type type = event_type::HEALTH;
			real32 amount = 0.0f;
			vec2 pos;

		};

		input in;

		real32 displayed_amount = 0.0f;
		double time_of_occurence_seconds = 0.0;
		uint32_t offset_slot = 0;
		mutable std::optional<vec2> first_pos;

		bool is_set() const {
			return in.amount != 0.0f;
		}
	};

private:
	struct damage_streak {
		std::vector<damage_event> events;

		double when_last_event_disappeared = 0.0;

		real32 total = 0.0f;
		real32 displayed_total = 0.0f;

		uint32_t current_event_slot_offset = 0;
		uint32_t total_damage_events = 0;

		mutable std::optional<vec2> last_visible_pos;
	};

	double global_time_seconds = 0.0;

	struct active_white_highlight {
		// We keep the target to reset the white highlight when e.g. the shield is destroyed
		messages::health_event::target_type target = messages::health_event::target_type::HEALTH;

		float original_ratio = 0.0f;
		double time_of_occurence_seconds = 0.0;
	};

	std::unordered_map<entity_id, damage_streak> streaks;
	std::unordered_map<entity_id, active_white_highlight> active_white_highlights;

public:

	struct white_highlight {
		float original_ratio; 
		float passed_mult; 
	};

	std::optional<white_highlight> find_white_highlight(entity_id, const damage_indication_settings& settings) const;

	void reserve_caches_for_entities(const size_t) const {}
	void clear();

	void add(
		const entity_id subject,
		const damage_event::input
	);

	void add_white_highlight(
		const entity_id subject,
		const messages::health_event::target_type target,
		float original_ratio
	);

	void advance(const damage_indication_settings& settings, const augs::delta dt);

	void draw_indicators(
		const std::function<bool(const_entity_handle)> is_reasonably_in_view,
		const const_entity_handle& viewed_character,
		const interpolation_system& interp,
		const damage_indication_settings& settings,
		const images_in_atlas_map& game_images,
		const all_loaded_gui_fonts& fonts,
		const augs::drawer output,
		const camera_cone
	) const;
};