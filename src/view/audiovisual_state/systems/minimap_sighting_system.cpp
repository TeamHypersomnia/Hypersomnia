#include <vector>
#include "augs/math/rects.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"
#include "game/cosmos/logic_step.h"
#include "game/cosmos/data_living_one_step.h"
#include "game/cosmos/for_each_entity.h"
#include "game/components/sentience_component.h"
#include "game/components/gun_component.h"
#include "game/components/hand_fuse_component.h"
#include "game/detail/entity_handle_mixins/inventory_mixin.hpp"
#include "game/messages/gunshot_message.h"
#include "game/messages/health_event.h"
#include "augs/templates/container_templates.h"
#include "augs/log.h"
#include "game/inferred_caches/physics_world_cache.h"
#include "game/enums/filters.h"
#include "view/audiovisual_state/systems/minimap_sighting_system.h"

void minimap_sighting_system::clear() {
	enemy_records.clear();
	recent_deaths.clear();
	bomb_pulse = {};
	teammate_rotation_counter = 0;
	prev_bomb_state = 0;
}

void minimap_sighting_system::record_deaths(const const_logic_step step) {
	const auto& cosm = step.get_cosmos();
	const auto now = cosm.get_total_seconds_passed();

	/* Prune old entries, and ones from the future after a clock rewind. */
	erase_if(
		recent_deaths,
		[&](const death_record& rec) {
			return rec.when > now || now - rec.when > 15.0;
		}
	);

	for (const auto& event : step.get_queue<messages::health_event>()) {
		const bool knocked_out =
			event.special_result == messages::health_event::result_type::DEATH ||
			event.special_result == messages::health_event::result_type::LOSS_OF_CONSCIOUSNESS
		;

		if (!knocked_out) {
			continue;
		}

		const auto subject = cosm[event.subject];

		if (subject.dead()) {
			continue;
		}

		const auto transform = subject.find_logic_transform();

		if (!transform.has_value()) {
			continue;
		}

		recent_deaths.push_back({
			transform->pos,
			subject.get_official_faction(),
			now
		});

		/*
			Death interrupts any active pulse and dot of that entity -
			the skull takes over from here.
		*/
		enemy_records.erase(event.subject);
	}
}

void minimap_sighting_system::advance(
	const const_logic_step step,
	const entity_id viewed_character,
	const vec2 sight_range,
	const float fov_angle
) {
	const bool consider_fov_angle = fov_angle > 0.0f && fov_angle < 360.0f;
	const auto& cosm = step.get_cosmos();
	const auto viewed = cosm[viewed_character];

	if (viewed.dead()) {
		return;
	}

	if (!viewed.has<components::sentience>()) {
		return;
	}

	const auto& physics = cosm.get_solvable_inferred().physics;
	const auto si = cosm.get_si();
	const auto now = cosm.get_total_seconds_passed();
	const auto viewer_faction = viewed.get_official_faction();
	const auto los_filter = predefined_queries::line_of_sight();

	thread_local std::vector<entity_id> teammates;
	thread_local std::vector<entity_id> enemies;

	teammates.clear();
	enemies.clear();

	cosm.for_each_having<components::sentience>(
		[&](const auto& typed_handle) {
			const auto& sentience = typed_handle.template get<components::sentience>();

			if (!sentience.is_conscious()) {
				return;
			}

			if (typed_handle.get_official_faction() == viewer_faction) {
				if (entity_id(typed_handle.get_id()) != viewed_character) {
					teammates.push_back(typed_handle.get_id());
				}
			}
			else {
				enemies.push_back(typed_handle.get_id());
			}
		}
	);

	/*
		Drop records of enemies that died, no longer exist, or are no
		longer enemies (faction switches). Records with timestamps from
		the future mean the cosmos clock went backwards (round restart
		or rewind) - those are stale as well.
	*/
	for (auto it = enemy_records.begin(); it != enemy_records.end();) {
		const auto& rec = it->second;
		const auto handle = cosm[it->first];

		const bool from_the_future =
			rec.last_seen_at > now ||
			rec.heard_at > now ||
			rec.appeared_at > now
		;

		const bool still_relevant =
			!from_the_future &&
			handle.alive() &&
			handle.template find<components::sentience>() != nullptr &&
			handle.template get<components::sentience>().is_conscious() &&
			handle.get_official_faction() != viewer_faction
		;

		if (still_relevant) {
			++it;
		}
		else {
			it = enemy_records.erase(it);
		}
	}

	if (enemies.empty()) {
		return;
	}

	/*
		The sighting budget: the viewed character checks every step,
		plus one teammate chosen round-robin.
	*/

	thread_local std::vector<entity_id> observers;
	observers.clear();
	observers.push_back(viewed_character);

	if (!teammates.empty()) {
		observers.push_back(teammates[teammate_rotation_counter % teammates.size()]);
		++teammate_rotation_counter;
	}

	for (const auto& enemy_id : enemies) {
		const auto enemy = cosm[enemy_id];
		const auto enemy_transform = enemy.find_logic_transform();

		if (!enemy_transform.has_value()) {
			continue;
		}

		const auto enemy_pos = enemy_transform->pos;

		thread_local std::vector<entity_id> effective_observers;
		effective_observers = observers;

		/*
			Whoever saw this enemy last keeps tracking them every step,
			so the sight loss is detected immediately.
		*/

		if (const auto found = enemy_records.find(enemy_id); found != enemy_records.end()) {
			const auto& rec = found->second;

			if (rec.last_seen_by.is_set() && is_seen_now(rec, now)) {
				if (!found_in(effective_observers, rec.last_seen_by)) {
					const auto tracker = cosm[rec.last_seen_by];

					const bool valid_tracker =
						tracker.alive() &&
						tracker.template find<components::sentience>() != nullptr &&
						tracker.template get<components::sentience>().is_conscious() &&
						tracker.get_official_faction() == viewer_faction
					;

					if (valid_tracker) {
						effective_observers.push_back(rec.last_seen_by);
					}
				}
			}
		}

		bool seen = false;
		auto seen_by = entity_id();

		for (const auto& observer_id : effective_observers) {
			const auto observer = cosm[observer_id];
			const auto observer_transform = observer.find_logic_transform();

			if (!observer_transform.has_value()) {
				continue;
			}

			const auto observer_pos = observer_transform->pos;

			/*
				The enemy must be within the observer's field of view cone,
				just like in is_reasonably_in_view. The angle check goes
				first as it is the cheapest.
			*/

			if (consider_fov_angle) {
				const auto look_dir = vec2::from_degrees(observer_transform->rotation);
				const auto enemy_dir = enemy_pos - observer_pos;

				if (look_dir.degrees_between(enemy_dir) > fov_angle / 2) {
					continue;
				}
			}

			if (!ltrb::center_and_size(observer_pos, sight_range).hover(enemy_pos)) {
				continue;
			}

			const auto raycast = physics.ray_cast_px(
				si,
				observer_pos,
				enemy_pos,
				los_filter,
				observer_id
			);

			if (!raycast.hit) {
				seen = true;
				seen_by = observer_id;
				break;
			}
		}

		if (seen) {
			auto& rec = enemy_records[enemy_id];

			if (now - rec.last_seen_at > reappear_pulse_threshold_secs) {
				rec.appeared_at = now;
			}

			rec.last_seen_pos = enemy_pos;
			rec.last_seen_at = now;
			rec.last_seen_by = seen_by;
		}
	}

	/*
		Gunshots: an enemy shot heard by the viewed character or by any
		teammate reveals the muzzle position for a while.
		Hearing follows the same range formula as the AI:
		80% of the muzzle shot sound's max distance.
	*/

	for (const auto& shot : step.get_queue<messages::gunshot_message>()) {
		const auto shooter = cosm[shot.capability];

		if (shooter.dead()) {
			continue;
		}

		if (shooter.get_official_faction() == viewer_faction) {
			continue;
		}

		const auto muzzle_pos = shot.muzzle_transform.pos;

		auto hearing_dist = 0.0f;

		{
			const auto gun_handle = cosm[shot.subject];

			if (gun_handle.alive()) {
				gun_handle.dispatch_on_having_all<invariants::gun>([&](const auto& typed_gun) {
					const auto& gun_def = typed_gun.template get<invariants::gun>();
					hearing_dist = gun_def.muzzle_shot_sound.modifier.max_distance * 0.8f;
				});
			}
		}

		if (hearing_dist <= 0.0f) {
			continue;
		}

		auto anyone_heard = [&]() {
			auto is_within = [&](const entity_id& listener_id) {
				const auto listener = cosm[listener_id];
				const auto listener_transform = listener.find_logic_transform();

				if (!listener_transform.has_value()) {
					return false;
				}

				const auto dist_sq = (listener_transform->pos - muzzle_pos).length_sq();
				return dist_sq <= hearing_dist * hearing_dist;
			};

			if (is_within(viewed_character)) {
				return true;
			}

			for (const auto& teammate_id : teammates) {
				if (is_within(teammate_id)) {
					return true;
				}
			}

			return false;
		};

		if (anyone_heard()) {
			auto& rec = enemy_records[shot.capability];
			rec.heard_at = now;
			rec.heard_pos = muzzle_pos;
		}
	}

	/*
		A pulse when the bomb lands on the ground or gets planted.
	*/

	{
		if (bomb_pulse.when > now) {
			bomb_pulse = {};
		}

		/* 0 - unknown, 1 - carried, 2 - dropped, 3 - planted */
		auto bomb_state = 0;
		auto bomb_pos = vec2();
		auto bomb_id = entity_id();

		cosm.for_each_having<components::hand_fuse>(
			[&](const auto& typed_handle) {
				const auto& fuse_def = typed_handle.template get<invariants::hand_fuse>();

				if (!fuse_def.is_like_plantable_bomb()) {
					return;
				}

				const auto transform = typed_handle.find_logic_transform();

				if (!transform.has_value()) {
					return;
				}

				const auto& fuse = typed_handle.template get<components::hand_fuse>();

				if (fuse.armed()) {
					bomb_state = 3;
				}
				else if (typed_handle.get_owning_transfer_capability().alive()) {
					bomb_state = 1;
				}
				else {
					bomb_state = 2;
				}

				bomb_pos = transform->pos;
				bomb_id = typed_handle.get_id();
			}
		);

		const bool became_dropped = bomb_state == 2 && prev_bomb_state != 2;
		const bool became_planted = bomb_state == 3 && prev_bomb_state != 3;

		if ((became_dropped || became_planted) && prev_bomb_state != 0) {
			bomb_pulse = { bomb_pos, now, bomb_id };
		}

		prev_bomb_state = bomb_state;
	}
}
