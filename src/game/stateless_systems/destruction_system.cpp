#include "destruction_system.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_id.h"
#include "game/cosmos/entity_handle.h"
#include "game/cosmos/logic_step.h"
#include "game/cosmos/data_living_one_step.h"
#include "game/cosmos/just_create_entity.h"

#include "game/messages/collision_message.h"
#include "game/messages/damage_message.h"
#include "game/messages/health_event.h"

#include "game/components/fixtures_component.h"
#include "game/components/shape_circle_component.h"
#include "game/components/destructible_component.h"
#include "game/components/rigid_body_component.h"

#include "game/detail/physics/physics_scripts.h"

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

		const auto& fixtures = subject.get<invariants::fixtures>();

		const auto& data_indices = it.indices.subject;

		if (data_indices.is_set() && fixtures.is_destructible()) {
			//LOG("Destructible fixture was hit.");
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

void destruction_system::apply_damages_and_split_fixtures(const logic_step step) const {
	auto& cosm = step.get_cosmos();
	const auto& damages = step.get_queue<messages::damage_message>();

	for (const auto& d : damages) {
		if (d.processed) {
			continue;
		}

		const auto subject = cosm[d.subject];
		
		if (subject.dead()) {
			continue;
		}

		/* Check if this entity has a destructible component */
		const auto* destructible = subject.find<components::destructible>();
		
		if (!destructible) {
			continue;
		}

		/* Skip if destructibility is disabled (max_health == -1) */
		if (!destructible->is_enabled()) {
			continue;
		}

		/* Get mutable reference for modifications */
		auto& dest = subject.get<components::destructible>();
		
		const auto damage_amount = d.damage.base;
		
		/* Apply damage to health */
		dest.health -= damage_amount;

		/* Calculate highlight intensity based on damage ratio */
		const auto damage_ratio = std::min(1.0f, damage_amount / dest.max_health);
		/* Linearly interpolate from 10% (at 0% damage) to 100% (at 50%+ damage) */
		const auto highlight_alpha = static_cast<rgba_channel>(255 * std::min(1.0f, 0.1f + damage_ratio * 1.8f));
		
		/* Post a health event for visual feedback (white highlight) */
		messages::health_event h;
		h.subject = d.subject;
		h.point_of_impact = d.point_of_impact;
		h.impact_velocity = d.impact_velocity;
		h.origin = d.origin;
		h.target = messages::health_event::target_type::HEALTH;
		h.damage.effective = damage_amount;
		
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

			/* Get sprite size for area calculation */
			const auto sprite_size = subject.get_logical_size();
			const auto actual_width = sprite_size.x * texture_rect.w;
			const auto actual_height = sprite_size.y * texture_rect.h;
			const auto actual_area = actual_width * actual_height;

			/* If area is less than 100 pixels, just delete */
			if (actual_area < 100.0f) {
				/* TODO: Spawn destruction effects here */
				step.queue_deletion_of(subject, "Destructible too small to split");
				continue;
			}

			/* 
			 * Splitting logic:
			 * 1. Find closest edge to point of impact
			 * 2. Create a split line 
			 * 3. Create smaller chunk as new entity
			 * 4. Adjust original entity
			 * 5. Reinfer physics
			 */

			const auto transform = subject.get_logic_transform();
			const auto entity_pos = transform.pos;
			
			/* Calculate local point of impact relative to entity center */
			auto local_impact = d.point_of_impact - entity_pos;
			local_impact.rotate(-transform.rotation);

			const auto half_w = actual_width / 2.0f;
			const auto half_h = actual_height / 2.0f;

			/* Find which edge is closest to the impact point */
			const real32 dist_left = local_impact.x + half_w;
			const real32 dist_right = half_w - local_impact.x;
			const real32 dist_top = local_impact.y + half_h;
			const real32 dist_bottom = half_h - local_impact.y;

			enum class edge_type { LEFT, RIGHT, TOP, BOTTOM };
			edge_type closest_edge = edge_type::LEFT;
			real32 min_dist = dist_left;

			if (dist_right < min_dist) { min_dist = dist_right; closest_edge = edge_type::RIGHT; }
			if (dist_top < min_dist) { min_dist = dist_top; closest_edge = edge_type::TOP; }
			if (dist_bottom < min_dist) { min_dist = dist_bottom; closest_edge = edge_type::BOTTOM; }

			/* Calculate split position along the edge (0-1 in texture space) */
			real32 split_ratio = 0.5f;
			bool is_horizontal_split = (closest_edge == edge_type::TOP || closest_edge == edge_type::BOTTOM);

			if (is_horizontal_split) {
				/* Split vertically (left-right) */
				split_ratio = (local_impact.x + half_w) / actual_width;
			} else {
				/* Split horizontally (top-bottom) */
				split_ratio = (local_impact.y + half_h) / actual_height;
			}

			/* Limit split position: 5px from corners minimum, but if edge < 10px, split in half */
			const real32 edge_length = is_horizontal_split ? actual_width : actual_height;
			const real32 min_offset_px = 5.0f;

			if (edge_length <= 10.0f) {
				split_ratio = 0.5f;
			} else {
				const real32 min_ratio = min_offset_px / edge_length;
				const real32 max_ratio = 1.0f - min_ratio;
				split_ratio = std::clamp(split_ratio, min_ratio, max_ratio);
			}

			/* Calculate new texture_rect for both pieces */
			xywh original_rect = dest.texture_rect;
			xywh new_rect = original_rect;

			/* Calculate the original max_health based on root max_health */
			const real32 root_max_health = dest.max_health / texture_area;

			if (is_horizontal_split) {
				/* Left piece = original, Right piece = new */
				const real32 split_x = original_rect.x + original_rect.w * split_ratio;

				/* Larger piece stays original */
				if (split_ratio >= 0.5f) {
					/* Left piece is larger or equal - keep as original */
					original_rect.w = original_rect.w * split_ratio;

					new_rect.x = split_x;
					new_rect.w = dest.texture_rect.w * (1.0f - split_ratio);
				} else {
					/* Right piece is larger - swap them */
					new_rect.w = original_rect.w * split_ratio;
					
					original_rect.x = split_x;
					original_rect.w = dest.texture_rect.w * (1.0f - split_ratio);
				}
			} else {
				/* Top piece = original, Bottom piece = new */
				const real32 split_y = original_rect.y + original_rect.h * split_ratio;

				if (split_ratio >= 0.5f) {
					/* Top piece is larger or equal - keep as original */
					original_rect.h = original_rect.h * split_ratio;

					new_rect.y = split_y;
					new_rect.h = dest.texture_rect.h * (1.0f - split_ratio);
				} else {
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

			/* Update original entity */
			dest.texture_rect = original_rect;
			dest.max_health = original_new_max_health;
			dest.health = original_new_max_health; /* Reset health since it just split */

			/* Clone the entity to create the new chunk */
			auto new_entity = just_clone_entity(allocate_new_entity_access(), subject);

			if (new_entity.alive()) {
				auto& new_dest = new_entity.get<components::destructible>();
				new_dest.texture_rect = new_rect;
				new_dest.max_health = new_piece_max_health;
				new_dest.health = new_piece_max_health;

				/* Calculate new positions for both entities */
				/* The center of each piece shifts based on how it was split */
				vec2 original_center_shift;
				vec2 new_center_shift;

				if (is_horizontal_split) {
					const real32 original_center_x = (original_rect.x + original_rect.w / 2.0f - 0.5f) * sprite_size.x;
					const real32 new_center_x = (new_rect.x + new_rect.w / 2.0f - 0.5f) * sprite_size.x;

					original_center_shift = vec2(original_center_x, 0);
					new_center_shift = vec2(new_center_x, 0);
				} else {
					const real32 original_center_y = (original_rect.y + original_rect.h / 2.0f - 0.5f) * sprite_size.y;
					const real32 new_center_y = (new_rect.y + new_rect.h / 2.0f - 0.5f) * sprite_size.y;

					original_center_shift = vec2(0, original_center_y);
					new_center_shift = vec2(0, new_center_y);
				}

				/* Rotate shifts to world space */
				original_center_shift.rotate(transform.rotation);
				new_center_shift.rotate(transform.rotation);

				/* Set new transforms */
				const auto original_new_pos = entity_pos + original_center_shift;
				const auto new_entity_pos = entity_pos + new_center_shift;

				subject.get<components::rigid_body>().set_transform(transformr(original_new_pos, transform.rotation));
				new_entity.get<components::rigid_body>().set_transform(transformr(new_entity_pos, transform.rotation));

				/* Reinfer physics for both entities */
				subject.infer_colliders_from_scratch();
				new_entity.infer_colliders_from_scratch();

				/* Apply separating impulse proportional to damage */
				const auto impact_dir = d.impact_velocity.is_nonzero() ? d.impact_velocity.normalize() : vec2(1, 0);
				const auto impulse_magnitude = damage_amount * 0.5f; /* Scale factor for impulse */

				/* Original piece gets pushed in the opposite direction of impact */
				if (auto rigid = subject.find<components::rigid_body>()) {
					rigid.apply_impulse(-impact_dir * impulse_magnitude * (1.0f - split_ratio));
				}

				/* New piece gets pushed in the direction of impact */
				if (auto rigid = new_entity.find<components::rigid_body>()) {
					rigid.apply_impulse(impact_dir * impulse_magnitude * split_ratio);
				}
			}
		}
	}
}