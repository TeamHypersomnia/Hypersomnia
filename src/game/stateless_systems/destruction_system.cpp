#include "destruction_system.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_id.h"
#include "game/cosmos/entity_handle.h"
#include "game/cosmos/logic_step.h"
#include "game/cosmos/data_living_one_step.h"
#include "game/cosmos/just_create_entity.h"
#include "game/cosmos/cosmos_global_solvable.h"

#include "game/messages/collision_message.h"
#include "game/messages/damage_message.h"
#include "game/messages/pure_color_highlight_message.h"

#include "game/components/fixtures_component.h"
#include "game/components/shape_circle_component.h"
#include "game/components/destructible_component.h"
#include "game/components/rigid_body_component.h"
#include "game/invariants/destructible.h"

#include "game/detail/physics/physics_scripts.h"
#include "game/detail/physics/calc_physical_material.hpp"
#include "game/detail/view_input/sound_effect_input.h"
#include "game/detail/view_input/particle_effect_input.h"
#include "game/detail/spaaawn.h"
#include "augs/templates/container_templates.h"
#include "augs/misc/randomization.h"

void destruction_system::generate_damages_from_forceful_collisions(const logic_step step) const {
	auto& cosm = step.get_cosmos();
	const auto& events = step.get_queue<messages::collision_message>();

	for (const auto& it : events) {
		if (it.type != messages::collision_message::event_type::PRE_SOLVE || it.one_is_sensor) {
			continue;
		}
		
		const auto subject = cosm[it.subject];
		const auto collider = cosm[it.collider];

		if (subject.dead() || collider.dead()) {
			continue;
		}

		const auto* fixtures = subject.find<invariants::fixtures>();
		if (!fixtures) {
			continue;
		}

		const auto& data_indices = it.indices.subject;

		if (data_indices.is_set() && fixtures->is_destructible()) {
			messages::damage_message damage_msg;
			damage_msg.indices = it.indices;

			damage_msg.origin = damage_origin(cosm[it.collider]);
			damage_msg.subject = it.subject;
			damage_msg.impact_velocity = it.collider_impact_velocity;
			damage_msg.normal = damage_msg.impact_velocity;
			damage_msg.point_of_impact = it.point;

			step.post_message(damage_msg);
		}
	}
}

void destruction_system::generate_damages_for_pending_destructions(const logic_step step) const {
	auto& cosm = step.get_cosmos();
	auto& global = cosm.get_global_solvable();
	const auto delta = step.get_delta();

	erase_if(global.pending_destructions, [&](pending_destruction& pd) {
		pd.delay_ms -= delta.in_milliseconds();

		if (pd.delay_ms <= 0.0f) {
			const auto target = cosm[pd.target];

			if (target.dead()) {
				return true;
			}

			const auto* dest_inv = target.find<invariants::destructible>();
			const auto* dest_comp = target.find<components::destructible>();
			if (!dest_inv || !dest_comp) {
				return true;
			}

			/* Only generate damage if health is still negative (hasn't been destroyed by another event) */
			if (dest_comp->health < 0.0f) {
				messages::damage_message damage_msg;
				damage_msg.subject = pd.target;
				damage_msg.damage.base = 1.0f; /* Formality - we detect negative health in apply_damages */
				damage_msg.impact_velocity = pd.impact_velocity;
				damage_msg.point_of_impact = target.get_logic_transform().pos; /* Center of entity */

				step.post_message(damage_msg);
			}

			return true;
		}

		return false;
	});
}

void destruction_system::apply_damages_and_split_fixtures(const logic_step step) const {
	auto& cosm = step.get_cosmos();
	auto& global = cosm.get_global_solvable();
	auto& rng = step.step_rng;
	const auto& damages = step.get_queue<messages::damage_message>();

	for (const auto& d : damages) {
		if (d.processed) {
			continue;
		}

		const auto subject = cosm[d.subject];
		
		if (subject.dead()) {
			continue;
		}

		/* Check if this entity has a destructible invariant and component */
		const auto* dest_inv = subject.find<invariants::destructible>();
		const auto* dest_comp_ptr = subject.find<components::destructible>();
		
		if (!dest_inv || !dest_comp_ptr) {
			continue;
		}

		const auto& dest_inv_ref = *dest_inv;
		
		if (subject.get_logical_size().area() < dest_inv_ref.disable_below_area) {
			continue;
		}

		/* Get mutable reference for modifications */
		auto& dest = subject.get<components::destructible>();
		
		const auto damage_amount = d.damage.base;
		const auto health_before_damage = dest.health;
		
		/* Apply damage to health */
		dest.health -= damage_amount;

		/* 
		 * Calculate highlight intensity based on damage / remaining health ratio.
		 * 100% highlight when health is negative (pending destruction).
		 */
		real32 highlight_ratio = 1.0f;
		if (health_before_damage > 0.0f) {
			highlight_ratio = std::min(1.0f, damage_amount / health_before_damage);
		}

		/* Linearly interpolate from 10% (at 0% ratio) to 100% (at 50%+ ratio) */
		const auto highlight_alpha = std::min(1.0f, 0.1f + highlight_ratio * 1.8f);
		
		/* 
		 * Post a pure_color_highlight message for visual feedback.
		 * Don't post health_event since we don't want damage numbers for non-sentient objects.
		 */
		messages::pure_color_highlight h;
		h.subject = d.subject;
		h.input.starting_alpha_ratio = highlight_alpha;
		h.input.maximum_duration_seconds = 0.15f;
		h.input.color = white;
		
		step.post_message(h);

		/* Check if the entity should be destroyed/split */
		if (dest.health <= 0.0f) {
			const auto& texture_rect = dest.texture_rect;
			const auto texture_area = texture_rect.w * texture_rect.h;

			/* If area is too small (essentially 0), just delete */
			if (texture_area <= 0.0f) {
				step.queue_deletion_of(subject, "Destructible area too small");
				continue;
			}

			/* 
			 * Get the base sprite size (BEFORE texture_rect scaling).
			 * get_logical_size() returns the size WITH texture_rect applied,
			 * so we need to reverse that to get the original base size.
			 */
			const auto scaled_size = subject.get_logical_size();
			const auto base_sprite_size = vec2(
				texture_rect.w > 0.0f ? scaled_size.x / texture_rect.w : scaled_size.x,
				texture_rect.h > 0.0f ? scaled_size.y / texture_rect.h : scaled_size.y
			);

			/* Now calculate actual chunk dimensions */
			const auto actual_width = base_sprite_size.x * texture_rect.w;
			const auto actual_height = base_sprite_size.y * texture_rect.h;
			const auto actual_area = actual_width * actual_height;

			/* 
			 * If the actual area is already below the minimum destructible threshold (100px²),
			 * don't split further - just disable destructibility and let the LYING_ITEM filter
			 * handle it in calc_filters. This prevents infinite splitting loops.
			 */
			constexpr real32 min_destructible_area = 100.0f;
			if (actual_area < min_destructible_area) {
				/* Reset health to positive to stop further destruction attempts */
				dest.health = dest_inv_ref.max_health * texture_area;
				continue;
			}

			/* 
			 * Spawn money on FIRST split only (when texture_rect area is 1.0).
			 * This is the original entity being split for the first time.
			 */
			const bool is_first_split = (texture_area >= 0.99f);
			if (is_first_split && dest_inv_ref.money_spawned_max > 0) {
				const auto money_amount = rng.randval(dest_inv_ref.money_spawned_min, dest_inv_ref.money_spawned_max);
				if (money_amount > 0) {
					/* Import spawn_money function - spawn coins at the split location */
					const auto spawn_pos = transformr(d.point_of_impact, 0.0f);
					spawn_money(step, spawn_pos, money_amount, subject);
				}
			}

			/* 
			 * Splitting logic:
			 * 1. If edges are roughly equal, find the closest edge
			 * 2. Otherwise split along the LONGER edges
			 * 3. Find closest point on either of the two longer edges
			 * 4. Limit split ratio to at most 10% deviation from center
			 */

			const auto transform = subject.get_logic_transform();
			const auto entity_pos = transform.pos;
			
			/* Calculate local point of impact relative to entity center */
			auto local_impact = d.point_of_impact - entity_pos;
			local_impact.rotate(-transform.rotation);

			const auto half_w = actual_width / 2.0f;
			const auto half_h = actual_height / 2.0f;

			/* 
			 * Check if edges are roughly equal (within epsilon of 10%).
			 * If so, find the closest edge instead of always choosing the longer one.
			 */
			constexpr real32 edge_epsilon = 0.1f;
			const real32 edge_ratio = std::min(actual_width, actual_height) / std::max(actual_width, actual_height);
			const bool edges_roughly_equal = edge_ratio >= (1.0f - edge_epsilon);

			bool is_horizontal_split;
			
			if (edges_roughly_equal) {
				/* Edges are roughly equal - find the closest edge */
				const real32 dist_left = local_impact.x + half_w;
				const real32 dist_right = half_w - local_impact.x;
				const real32 dist_top = local_impact.y + half_h;
				const real32 dist_bottom = half_h - local_impact.y;

				const real32 min_h_dist = std::min(dist_left, dist_right);
				const real32 min_v_dist = std::min(dist_top, dist_bottom);

				/* If closest to left/right edge, do horizontal split (left-right pieces) */
				is_horizontal_split = (min_h_dist < min_v_dist);
			}
			else {
				/* Split along the longer dimension */
				is_horizontal_split = (actual_width >= actual_height);
			}
			
			/* Calculate split position along the edge (0-1 in texture space) */
			real32 split_ratio;
			if (is_horizontal_split) {
				/* Split creates left-right pieces - project impact onto the width */
				split_ratio = (local_impact.x + half_w) / actual_width;
			}
			else {
				/* Split creates top-bottom pieces - project impact onto the height */
				split_ratio = (local_impact.y + half_h) / actual_height;
			}

			/* 
			 * Limit split position to at most 10% deviation from center (0.5).
			 * This means split_ratio must be in range [0.4, 0.6].
			 */
			constexpr real32 max_deviation_from_center = 0.1f;
			const real32 min_ratio = 0.5f - max_deviation_from_center;
			const real32 max_ratio = 0.5f + max_deviation_from_center;
			split_ratio = std::clamp(split_ratio, min_ratio, max_ratio);

			/* Calculate new texture_rect for both pieces */
			xywh original_rect = dest.texture_rect;
			xywh new_rect = original_rect;

			/* 
			 * Calculate the root max_health (the invariant's max_health is the root value).
			 * The current chunk's health should be scaled by its area.
			 */
			const real32 root_max_health = dest_inv_ref.max_health;
			
			/* Calculate excessive damage to distribute to both splits */
			const real32 excess_damage = -dest.health; /* health is negative, so -health gives excess */

			if (is_horizontal_split) {
				/* Left piece = original, Right piece = new */
				const real32 split_x = original_rect.x + original_rect.w * split_ratio;

				/* Larger piece stays original */
				if (split_ratio >= 0.5f) {
					/* Left piece is larger or equal - keep as original */
					original_rect.w = original_rect.w * split_ratio;

					new_rect.x = split_x;
					new_rect.w = dest.texture_rect.w * (1.0f - split_ratio);
				}
				else {
					/* Right piece is larger - swap them */
					new_rect.w = original_rect.w * split_ratio;
					
					original_rect.x = split_x;
					original_rect.w = dest.texture_rect.w * (1.0f - split_ratio);
				}
			}
			else {
				/* Top piece = original, Bottom piece = new */
				const real32 split_y = original_rect.y + original_rect.h * split_ratio;

				if (split_ratio >= 0.5f) {
					/* Top piece is larger or equal - keep as original */
					original_rect.h = original_rect.h * split_ratio;

					new_rect.y = split_y;
					new_rect.h = dest.texture_rect.h * (1.0f - split_ratio);
				}
				else {
					/* Bottom piece is larger - swap them */
					new_rect.h = original_rect.h * split_ratio;
					
					original_rect.y = split_y;
					original_rect.h = dest.texture_rect.h * (1.0f - split_ratio);
				}
			}

			/* Calculate new health values based on area */
			const real32 original_new_area = original_rect.w * original_rect.h;
			const real32 new_piece_area = new_rect.w * new_rect.h;

			const real32 original_new_max_health = root_max_health * original_new_area;
			const real32 new_piece_max_health = root_max_health * new_piece_area;

			/* 
			 * Apply excess damage to both splits.
			 * Set health = max_health - excess_damage for each split.
			 * Negative health means the split will be destroyed in a future tick.
			 */
			const real32 original_new_health = original_new_max_health - excess_damage;
			const real32 new_piece_health = new_piece_max_health - excess_damage;

			/* Update original entity - only health and texture_rect since max_health is in invariant */
			dest.texture_rect = original_rect;
			dest.health = original_new_health;

			/* Calculate position shifts needed for both entities */
			vec2 original_center_shift;
			vec2 new_center_shift;

			if (is_horizontal_split) {
				const real32 original_center_x = (original_rect.x + original_rect.w / 2.0f - 0.5f) * base_sprite_size.x;
				const real32 new_center_x = (new_rect.x + new_rect.w / 2.0f - 0.5f) * base_sprite_size.x;

				original_center_shift = vec2(original_center_x, 0);
				new_center_shift = vec2(new_center_x, 0);
			}
			else {
				const real32 original_center_y = (original_rect.y + original_rect.h / 2.0f - 0.5f) * base_sprite_size.y;
				const real32 new_center_y = (new_rect.y + new_rect.h / 2.0f - 0.5f) * base_sprite_size.y;

				original_center_shift = vec2(0, original_center_y);
				new_center_shift = vec2(0, new_center_y);
			}

			/* Rotate shifts to world space */
			original_center_shift.rotate(transform.rotation);
			new_center_shift.rotate(transform.rotation);

			/* Calculate final positions */
			const auto original_new_pos = entity_pos + original_center_shift;
			const auto new_entity_pos = entity_pos + new_center_shift;
			const auto rotation = transform.rotation;

			/* Reposition original entity now */
			subject.get<components::rigid_body>().set_transform(transformr(original_new_pos, rotation));
			subject.infer_rigid_body();
			subject.infer_colliders_from_scratch();

			/* Calculate impulse parameters for later application */
			const auto split_direction = is_horizontal_split ? vec2(1, 0) : vec2(0, 1);
			const auto fallback_dir = vec2(split_direction).rotate(rotation);
			const auto impact_dir = d.impact_velocity.is_nonzero() ? vec2(d.impact_velocity).normalize() : fallback_dir;
			
			constexpr real32 damage_to_impulse_scale = 0.5f;
			const auto impulse_magnitude = damage_amount * damage_to_impulse_scale;

			/* Apply impulse to original piece */
			if (auto rigid = subject.find<components::rigid_body>()) {
				rigid.apply_impulse(-impact_dir * impulse_magnitude * (1.0f - split_ratio));
			}

			/* 
			 * Spawn destruction effects (sound and particles).
			 * Pitch and scale are based on chunk area:
			 * - area = 1.0 → pitch = 1.0, scale = 1.0
			 * - area = 0.5 → pitch = 1.2, scale = 0.8  
			 * - area = 0.0 → pitch = 1.4, scale = 0.6
			 */
			{
				const auto& logicals = cosm.get_logical_assets();
				const auto material_id = calc_physical_material(subject);
				
				if (const auto* mat = logicals.find(material_id)) {
					/* Calculate pitch and scale modifiers based on area */
					const real32 pitch_mult = 1.0f + 0.4f * (1.0f - texture_area);
					const real32 scale_mult = 0.6f + 0.4f * texture_area;

					const auto effect_transform = transformr(d.point_of_impact, (-d.impact_velocity).degrees());

					/* Spawn destruction sound */
					auto destruction_sound = mat->standard_destruction_sound;
					if (!destruction_sound.id.is_set()) {
						/* Fall back to standard damage sound if destruction sound not set */
						destruction_sound = mat->standard_damage_sound;
					}
					if (destruction_sound.id.is_set()) {
						destruction_sound.modifier.pitch *= pitch_mult;
						destruction_sound.start(
							step,
							sound_effect_start_input::fire_and_forget(effect_transform).set_listener(subject),
							always_predictable_v
						);
					}

					/* Spawn destruction particles */
					auto destruction_particles = mat->standard_destruction_particles;
					if (!destruction_particles.id.is_set()) {
						/* Fall back to standard damage particles if destruction particles not set */
						destruction_particles = mat->standard_damage_particles;
					}
					if (destruction_particles.id.is_set()) {
						destruction_particles.modifier.scale_amounts *= scale_mult;
						destruction_particles.start(
							step,
							particle_effect_start_input::orbit_absolute(subject, effect_transform),
							always_predictable_v
						);
					}
				}
			}

			/* Queue pending destruction for original if health is negative */
			const auto subject_id = subject.get_id();
			if (original_new_health < 0.0f) {
				const real32 local_excess = -original_new_health;
				const real32 ratio = std::max(0.01f, local_excess / original_new_max_health);
				real32 delay = 200.0f / ratio;
				delay *= rng.randval(0.5f, 1.0f);

				pending_destruction pd;
				pd.target = subject_id;
				pd.delay_ms = 0.0f;
				pd.impact_velocity = d.impact_velocity;

				global.pending_destructions.push_back(pd);
			}

			/* 
			 * Queue the clone operation. The lambda captures all needed values by copy.
			 * This is the safe approach - the clone will be created during flush_create_entity_requests,
			 * after all immediate operations are done, preventing pool reallocation issues.
			 */
			const auto impact_velocity_copy = d.impact_velocity;

			queue_clone_entity(
				step,
				subject_id,
				[=](entity_handle new_entity, logic_step step_inner) mutable {
					if (new_entity.dead()) {
						return;
					}

					auto& cosm_inner = step_inner.get_cosmos();
					auto& global_inner = cosm_inner.get_global_solvable();

					/* Set up the new chunk's destructible component - only health and texture_rect */
					auto* new_dest = new_entity.find<components::destructible>();
					if (new_dest) {
						new_dest->texture_rect = new_rect;
						new_dest->health = new_piece_health;
					}

					/* Position the new entity */
					if (auto rigid = new_entity.find<components::rigid_body>()) {
						rigid.set_transform(transformr(new_entity_pos, rotation));
					}

					/* Reinfer physics */
					new_entity.infer_rigid_body();
					new_entity.infer_colliders_from_scratch();

					/* Apply impulse to new piece (pushed in direction of impact) */
					if (auto rigid = new_entity.find<components::rigid_body>()) {
						rigid.apply_impulse(impact_dir * impulse_magnitude * split_ratio);
					}

					/* Queue pending destruction if health is negative */
					if (new_piece_health < 0.0f) {
						const real32 local_excess = -new_piece_health;
						const real32 ratio = std::max(0.01f, local_excess / new_piece_max_health);
						real32 delay = 200.0f / ratio;
						/* Use a fixed factor instead of rng inside lambda */
						delay *= 0.75f;

						pending_destruction pd;
						pd.target = new_entity.get_id();
						pd.delay_ms = 0.0f;
						pd.impact_velocity = impact_velocity_copy;

						global_inner.pending_destructions.push_back(pd);
					}
				}
			);
		}
	}
}
