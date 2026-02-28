#include "destruction_system.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_id.h"
#include "game/cosmos/entity_handle.h"
#include "game/cosmos/logic_step.h"
#include "game/cosmos/data_living_one_step.h"
#include "game/cosmos/just_create_entity.h"

#include "game/messages/collision_message.h"
#include "game/messages/damage_message.h"
#include "game/messages/pure_color_highlight_message.h"

#include "game/components/fixtures_component.h"
#include "game/components/shape_circle_component.h"
#include "game/components/destructible_component.h"
#include "game/components/rigid_body_component.h"
#include "game/components/sprite_component.h"

#include "game/detail/physics/physics_scripts.h"
#include "game/detail/physics/calc_physical_material.hpp"
#include "game/detail/view_input/sound_effect_input.h"
#include "game/detail/view_input/particle_effect_input.h"
#include "game/detail/spawn_collectibles.hpp"
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

/*
 * Simplified destruction system:
 * - Destroy the subject once and split into 4 pieces immediately
 * - First split along the impact point (clamped to not be too close to edges)
 * - Then split each half along the other axis perfectly in half
 * - All splits become REMNANT (never destructible again)
 * - Splits are darker: disable_neon_map = disable_special_effects = true, colorize brightness * 0.7
 * - All 4 dynamic remnants fly toward the impact direction
 */
void destruction_system::apply_damages_and_split_fixtures(const logic_step step) const {
	auto& cosm = step.get_cosmos();
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

		/* Skip if this is already a remnant (texture_rect != 0,0,1,1) - never accept damage */
		if (dest_comp_ptr->is_remnant()) {
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
		 * 100% highlight when health is negative.
		 */
		real32 highlight_ratio = 1.0f;
		if (health_before_damage > 0.0f) {
			highlight_ratio = std::min(1.0f, damage_amount / health_before_damage);
		}

		const auto highlight_alpha = std::min(1.0f, 0.1f + highlight_ratio * 1.8f);
		
		/* Post a pure_color_highlight message for visual feedback */
		messages::pure_color_highlight h;
		h.subject = d.subject;
		h.input.starting_alpha_ratio = highlight_alpha;
		h.input.maximum_duration_seconds = 0.15f;
		h.input.color = white;
		
		step.post_message(h);

		/* Check if the entity should be destroyed/split */
		if (dest.health <= 0.0f) {
			/* 
			 * Get the base sprite size (the original size before any texture_rect scaling).
			 * Since this is the first destruction (texture_rect == 0,0,1,1), get_logical_size() gives original size.
			 */
			const auto base_sprite_size = subject.get_logical_size();
			const auto transform = subject.get_logic_transform();
			const auto entity_pos = transform.pos;
			const auto rotation = transform.rotation;

			/* Calculate local point of impact relative to entity center */
			auto local_impact = d.point_of_impact - entity_pos;
			local_impact.rotate(-rotation);

			const auto half_w = base_sprite_size.x / 2.0f;
			const auto half_h = base_sprite_size.y / 2.0f;

			/* 
			 * Determine split axis based on longer edge.
			 * If roughly equal, use the one closest to the impact point.
			 */
			constexpr real32 edge_epsilon = 0.1f;
			const real32 edge_ratio = std::min(base_sprite_size.x, base_sprite_size.y) / 
			                          std::max(base_sprite_size.x, base_sprite_size.y);
			const bool edges_roughly_equal = edge_ratio >= (1.0f - edge_epsilon);

			bool is_horizontal_split;
			
			if (edges_roughly_equal) {
				const real32 dist_left = local_impact.x + half_w;
				const real32 dist_right = half_w - local_impact.x;
				const real32 dist_top = local_impact.y + half_h;
				const real32 dist_bottom = half_h - local_impact.y;

				const real32 min_h_dist = std::min(dist_left, dist_right);
				const real32 min_v_dist = std::min(dist_top, dist_bottom);

				is_horizontal_split = (min_h_dist < min_v_dist);
			}
			else {
				is_horizontal_split = (base_sprite_size.x >= base_sprite_size.y);
			}
			
			/* Calculate split position along the primary axis (0-1 space) */
			real32 split_ratio;
			if (is_horizontal_split) {
				split_ratio = (local_impact.x + half_w) / base_sprite_size.x;
			}
			else {
				split_ratio = (local_impact.y + half_h) / base_sprite_size.y;
			}

			/* Clamp split ratio to at most 10% deviation from center */
			constexpr real32 max_deviation_from_center = 0.1f;
			split_ratio = std::clamp(split_ratio, 0.5f - max_deviation_from_center, 0.5f + max_deviation_from_center);

			/* 
			 * Create 4 pieces:
			 * - First split creates 2 halves along primary axis (at split_ratio)
			 * - Each half is then split in half along the secondary axis
			 * 
			 * Naming: A1, A2 (first half, split), B1, B2 (second half, split)
			 */
			
			struct ChunkInfo {
				xywh texture_rect;
				vec2 center_offset; // Offset from original entity center
			};

			std::array<ChunkInfo, 4> chunks;

			if (is_horizontal_split) {
				/* Primary split is horizontal (left-right), secondary is vertical (top-bottom) */
				const real32 left_w = split_ratio;
				const real32 right_w = 1.0f - split_ratio;

				/* A1: top-left */
				chunks[0].texture_rect = xywh(0.0f, 0.0f, left_w, 0.5f);
				/* A2: bottom-left */
				chunks[1].texture_rect = xywh(0.0f, 0.5f, left_w, 0.5f);
				/* B1: top-right */
				chunks[2].texture_rect = xywh(split_ratio, 0.0f, right_w, 0.5f);
				/* B2: bottom-right */
				chunks[3].texture_rect = xywh(split_ratio, 0.5f, right_w, 0.5f);

				/* Calculate center offsets */
				const real32 left_center_x = (left_w / 2.0f - 0.5f) * base_sprite_size.x;
				const real32 right_center_x = (split_ratio + right_w / 2.0f - 0.5f) * base_sprite_size.x;
				const real32 top_center_y = -0.25f * base_sprite_size.y;
				const real32 bottom_center_y = 0.25f * base_sprite_size.y;

				chunks[0].center_offset = vec2(left_center_x, top_center_y);
				chunks[1].center_offset = vec2(left_center_x, bottom_center_y);
				chunks[2].center_offset = vec2(right_center_x, top_center_y);
				chunks[3].center_offset = vec2(right_center_x, bottom_center_y);
			}
			else {
				/* Primary split is vertical (top-bottom), secondary is horizontal (left-right) */
				const real32 top_h = split_ratio;
				const real32 bottom_h = 1.0f - split_ratio;

				/* A1: top-left */
				chunks[0].texture_rect = xywh(0.0f, 0.0f, 0.5f, top_h);
				/* A2: top-right */
				chunks[1].texture_rect = xywh(0.5f, 0.0f, 0.5f, top_h);
				/* B1: bottom-left */
				chunks[2].texture_rect = xywh(0.0f, split_ratio, 0.5f, bottom_h);
				/* B2: bottom-right */
				chunks[3].texture_rect = xywh(0.5f, split_ratio, 0.5f, bottom_h);

				/* Calculate center offsets */
				const real32 left_center_x = -0.25f * base_sprite_size.x;
				const real32 right_center_x = 0.25f * base_sprite_size.x;
				const real32 top_center_y = (top_h / 2.0f - 0.5f) * base_sprite_size.y;
				const real32 bottom_center_y = (split_ratio + bottom_h / 2.0f - 0.5f) * base_sprite_size.y;

				chunks[0].center_offset = vec2(left_center_x, top_center_y);
				chunks[1].center_offset = vec2(right_center_x, top_center_y);
				chunks[2].center_offset = vec2(left_center_x, bottom_center_y);
				chunks[3].center_offset = vec2(right_center_x, bottom_center_y);
			}

			/* 
			 * Spawn money on destruction.
			 */
			const auto& common_assets = cosm.get_common_assets();

			if (dest_inv->money_spawned_max > 0 && !common_assets.default_coin_flavours.empty()) {
				const auto money_amount = rng.randval(dest_inv->money_spawned_min, dest_inv->money_spawned_max);
				if (money_amount > 0) {
					spawn_coins_queued(money_amount, d.point_of_impact, step, common_assets.default_coin_flavours);
				}
			}

			/* 
			 * Spawn destruction effects (sound and particles).
			 */
			{
				const auto& logicals = cosm.get_logical_assets();
				const auto material_id = calc_physical_material(subject);
				
				if (const auto* mat = logicals.find(material_id)) {
					const auto effect_transform = transformr(d.point_of_impact, (-d.impact_velocity).degrees());

					/* Spawn destruction sound */
					auto destruction_sound = mat->standard_destruction_sound;
					if (!destruction_sound.id.is_set()) {
						destruction_sound = mat->standard_damage_sound;
					}
					
					if (destruction_sound.id.is_set()) {
						destruction_sound.start(
							step,
							sound_effect_start_input::fire_and_forget(effect_transform).set_listener(subject),
							always_predictable_v
						);
					}

					/* Spawn destruction particles */
					auto destruction_particles = mat->standard_destruction_particles;
					if (!destruction_particles.id.is_set()) {
						destruction_particles = mat->standard_damage_particles;
					}
					if (destruction_particles.id.is_set()) {
						destruction_particles.start(
							step,
							particle_effect_start_input::orbit_absolute(subject, effect_transform),
							always_predictable_v
						);
					}
				}
			}

			/* Calculate impulse parameters */
			const auto fallback_dir = vec2(1, 0).rotate(rotation);
			const auto impact_dir = d.impact_velocity.is_nonzero() ? vec2(d.impact_velocity).normalize() : fallback_dir;
			
			constexpr real32 damage_to_impulse_scale = 0.5f;
			const auto impulse_magnitude = damage_amount * damage_to_impulse_scale;

			/* 
			 * Queue 4 clone operations for the chunks.
			 * We'll use the first chunk to replace the original entity,
			 * and clone for the other 3.
			 */
			const auto subject_id = subject.get_id();

			/* Process first chunk - reuse original entity */
			{
				const auto& chunk = chunks[0];
				
				/* Update the original entity to become the first chunk */
				dest.texture_rect = chunk.texture_rect;
				dest.health = dest_inv->max_health; /* Not relevant anymore, but set it */

				/* Make sprite darker and disable effects */
				if (auto* sprite = subject.find<components::sprite>()) {
					sprite->disable_neon_map = true;
					sprite->disable_special_effects = true;
					sprite->colorize.r = static_cast<rgba_channel>(sprite->colorize.r * 0.7f);
					sprite->colorize.g = static_cast<rgba_channel>(sprite->colorize.g * 0.7f);
					sprite->colorize.b = static_cast<rgba_channel>(sprite->colorize.b * 0.7f);
				}

				/* Calculate new position */
				auto offset = chunk.center_offset;
				offset.rotate(rotation);
				const auto new_pos = entity_pos + offset;

				/* Reposition and reinfer */
				subject.get<components::rigid_body>().set_transform(transformr(new_pos, rotation));
				subject.infer_rigid_body();
				subject.infer_colliders_from_scratch();

				/* Apply impulse */
				if (auto rigid = subject.find<components::rigid_body>()) {
					/* Chunks fly generally in impact direction with some spread */
					const auto chunk_dir = (chunk.center_offset.is_nonzero() ? 
						vec2(chunk.center_offset).normalize().rotate(rotation) : 
						impact_dir);
					rigid.apply_impulse((impact_dir * 0.7f + chunk_dir * 0.3f) * impulse_magnitude * 0.25f);
				}
			}

			/* Clone and setup chunks 1, 2, 3 */
			for (std::size_t i = 1; i < 4; ++i) {
				const auto& chunk = chunks[i];
				const auto chunk_texture_rect = chunk.texture_rect;
				const auto chunk_center_offset = chunk.center_offset;

				queue_clone_entity(
					step,
					subject_id,
					[=](entity_handle new_entity, logic_step step_inner) mutable {
						if (new_entity.dead()) {
							return;
						}

						/* Set up destructible component */
						if (auto* new_dest = new_entity.find<components::destructible>()) {
							new_dest->texture_rect = chunk_texture_rect;
						}

						/* Make sprite darker and disable effects */
						if (auto* sprite = new_entity.find<components::sprite>()) {
							sprite->disable_neon_map = true;
							sprite->disable_special_effects = true;
							sprite->colorize.r = static_cast<rgba_channel>(sprite->colorize.r * 0.7f);
							sprite->colorize.g = static_cast<rgba_channel>(sprite->colorize.g * 0.7f);
							sprite->colorize.b = static_cast<rgba_channel>(sprite->colorize.b * 0.7f);
						}

						/* Calculate new position */
						auto offset = chunk_center_offset;
						offset.rotate(rotation);
						const auto new_pos = entity_pos + offset;

						/* Position the new entity */
						if (auto rigid = new_entity.find<components::rigid_body>()) {
							rigid.set_transform(transformr(new_pos, rotation));
						}

						/* Reinfer physics */
						new_entity.infer_rigid_body();
						new_entity.infer_colliders_from_scratch();

						/* Apply impulse - chunks fly in impact direction with some spread */
						if (auto rigid = new_entity.find<components::rigid_body>()) {
							const auto chunk_dir = (chunk_center_offset.is_nonzero() ? 
								vec2(chunk_center_offset).normalize().rotate(rotation) : 
								impact_dir);
							rigid.apply_impulse((impact_dir * 0.7f + chunk_dir * 0.3f) * impulse_magnitude * 0.25f);
						}
					}
				);
			}
		}
	}
}
