#pragma once
#include "game/detail/sentience/gore/blood_splatter.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/logic_step.h"
#include "game/cosmos/create_entity.hpp"
#include "game/cosmos/data_living_one_step.h"
#include "game/organization/all_entity_types.h"
#include "game/components/decal_component.h"
#include "game/components/sprite_component.h"
#include "game/detail/view_input/particle_effect_input.h"

static constexpr unsigned BLOOD_SPLATTER_NUM_VARIANTS = 3;
/* Baseline distance for blood burst velocity scaling */
static constexpr real32 BLOOD_BURST_BASELINE_DISTANCE = 50.f;

inline void spawn_blood_splatters(
	allocate_new_entity_access access,
	const logic_step step,
	const entity_id subject,
	const vec2 position,
	const vec2 impact_direction,
	const real32 damage_amount,
	const blood_splatter_params& params
) {
	auto& cosm = step.get_cosmos();
	const auto& common_assets = cosm.get_common_assets();

	/* Calculate number of splatters: 1 per damage_per_splatter damage, at least 1 */
	const auto num_full_splatters = static_cast<int>(damage_amount / params.damage_per_splatter);
	const auto remainder = damage_amount - (num_full_splatters * params.damage_per_splatter);
	const auto remainder_size = remainder / params.damage_per_splatter;

	/* Total splatters: full ones + 1 partial if there's a remainder */
	const int num_splatters = num_full_splatters + (remainder > 0.f ? 1 : 0);

	if (num_splatters <= 0) {
		return;
	}

	/* Get available splatter flavours */
	std::array<typed_entity_flavour_id<decal_decoration>, BLOOD_SPLATTER_NUM_VARIANTS> splatter_flavours = {
		common_assets.blood_splatter_1,
		common_assets.blood_splatter_2,
		common_assets.blood_splatter_3
	};

	auto rng = cosm.get_rng_for(subject);

	/* Check if any splatter flavour is available */
	bool any_set = false;
	for (const auto& f : splatter_flavours) {
		if (f.is_set()) {
			any_set = true;
			break;
		}
	}

	if (!any_set) {
		return;
	}

	const auto impact_degrees = impact_direction.is_nonzero() ? impact_direction.degrees() : 0.f;

	/* Calculate max distance based on damage: scales from base to max at full damage */
	const auto damage_ratio = std::min(1.f, damage_amount / params.distance_damage_scale);
	const auto max_distance = params.max_distance_base + 
		(params.max_distance_at_full_damage - params.max_distance_base) * damage_ratio;

	const auto half_spread = params.angle_spread / 2.f;

	const bool has_burst_effect = common_assets.blood_burst_particles.id.is_set();

	for (int i = 0; i < num_splatters; ++i) {
		/* Determine size multiplier */
		real32 size_mult = 1.f;
		if (i == num_splatters - 1 && remainder > 0.f) {
			/* Last splatter gets partial size based on remainder */
			size_mult = std::max(params.min_size, remainder_size);
		}

		/* Choose a random splatter flavour using bounded random */
		const auto splatter_idx = static_cast<std::size_t>(rng.randval(0, static_cast<int>(BLOOD_SPLATTER_NUM_VARIANTS) - 1));
		auto flavour = splatter_flavours[splatter_idx];

		size_mult *= rng.randval(0.5f, 1.0f);

		/* Skip if this flavour is not set, try others */
		if (!flavour.is_set()) {
			for (const auto& f : splatter_flavours) {
				if (f.is_set()) {
					flavour = f;
					break;
				}
			}
		}

		if (!flavour.is_set()) {
			continue;
		}

		/* Randomize position with distance scaled by damage (slightly randomized) */
		const auto angle_offset = rng.randval(-half_spread, half_spread);
		const auto distance = rng.randval(params.min_distance, max_distance);
		const auto rotation = rng.randval(0.f, 360.f);

		const auto offset_direction = vec2::from_degrees(impact_degrees + angle_offset);
		const auto splatter_pos = position + offset_direction * distance;

		/* Spawn blood burst particles for this splatter: from impact towards splatter position */
		if (has_burst_effect) {
			auto burst_effect = common_assets.blood_burst_particles;
			
			/* Scale velocities based on distance to splatter */
			const auto distance_scale = distance / BLOOD_BURST_BASELINE_DISTANCE;
			burst_effect.modifier.scale_velocities = distance_scale / 2.0f;
			burst_effect.modifier.scale_amounts = std::min(15.0f, damage_amount / 5.f);
			burst_effect.modifier.scale_lifetimes = std::min(3.0f, damage_amount / 20.f);
			
			/* Direction from impact point towards the splatter position */
			const auto burst_degrees = offset_direction.degrees();
			
			burst_effect.start(
				step,
				particle_effect_start_input::orbit_absolute(cosm[subject], transformr(position, burst_degrees)),
				always_predictable_v
			);
		}

		cosmic::specific_create_entity(access, cosm, flavour, [&](auto splatter_entity, auto& agg) {
			splatter_entity.set_logic_transform(transformr(splatter_pos, rotation));

			/* Set spawned_by to track whose blood this is */
			if (auto* decal_state = agg.template find<components::decal>()) {
				decal_state->spawned_by = subject;
			}

			/* Apply size multiplier through overridden_geo if needed */
			if (size_mult < 1.f) {
				if (auto overridden_geo = agg.template find<components::overridden_geo>()) {
					const auto original_size = splatter_entity.template get<invariants::sprite>().size;
					overridden_geo->size = vec2i(vec2(original_size) * size_mult);
				}
			}
		});
	}
}

inline void spawn_blood_splatters_omnidirectional(
	allocate_new_entity_access access,
	const logic_step step,
	const entity_id subject,
	const vec2 position,
	const real32 damage_amount
) {
	/* Use the same logic as normal splatters but with 360° spread and different params */
	const auto direction = vec2::from_degrees(0.f); /* Starting direction doesn't matter with 360° spread */
	
	blood_splatter_params omni_params;
	omni_params.damage_per_splatter = 15.f;
	omni_params.angle_spread = 360.f;
	omni_params.min_distance = damage_amount/4;
	omni_params.max_distance_base = damage_amount/3;
	omni_params.max_distance_at_full_damage = omni_params.max_distance_base;
	
	::spawn_blood_splatters(access, step, subject, position, direction, damage_amount, omni_params);
}
