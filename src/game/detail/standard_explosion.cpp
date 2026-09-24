#include <map>
#include "augs/misc/randomization.h"
#include "game/detail/physics/physics_queries.h"
#include "game/detail/standard_explosion.h"
#include "game/assets/ids/asset_ids.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"
#include "game/detail/entity_handle_mixins/inventory_mixin.hpp"
#include "game/messages/exploding_ring_effect.h"
#include "game/messages/damage_message.h"
#include "game/stateless_systems/visibility_system.h"
#include "game/stateless_systems/sound_existence_system.h"
#include "game/enums/filters.h"
#include "game/components/sentience_component.h"
#include "game/cosmos/logic_step.h"
#include "game/cosmos/data_living_one_step.h"
#include "game/debug_drawing_settings.h"
#include "game/messages/thunder_effect.h"
#include "game/cosmos/data_living_one_step.h"
#include "game/detail/organisms/startle_nearbly_organisms.h"
#include "game/detail/physics/shape_overlapping.hpp"
#include "game/detail/damage_origin.hpp"
#include "game/detail/movement/dash_logic.h"
#include "game/detail/sentience/sentience_logic.h"
#include "game/detail/decals/spawn_decals.hpp"

static bool triangle_degenerate(const std::array<vec2, 3>& v) {
	constexpr auto eps_triangle_degenerate = 0.5f;

	if (v[0].compare(v[1], eps_triangle_degenerate)) {
		return true;
	}

	if (v[0].compare(v[2], eps_triangle_degenerate)) {
		return true;
	}

	if (v[1].compare(v[2], eps_triangle_degenerate)) {
		return true;
	}

	return false;
}

void standard_explosion_input::instantiate(
	const logic_step step,
	const transformr explosion_location,
	const damage_cause cause,
	const predictability_info predictability
) const {
	const auto subject_if_any = cause.entity;

	if (create_thunders_effect) {
		for (int t = 0; t < 4; ++t) {
			static randomization rng;
			auto msg = messages::thunder_effect(predictability);
			auto& th = msg.payload;

			th.delay_between_branches_ms = {10.f, 25.f};
			th.max_branch_lifetime_ms = {40.f, 65.f};
			th.branch_length = {10.f, 120.f};

			th.max_all_spawned_branches = 40 + (t+1)*10;
			th.max_branch_children = 2;

			th.first_branch_root = explosion_location;
			th.first_branch_root.pos += rng.random_point_in_circle(70.f);
			th.first_branch_root.rotation += t * 360/4;
			th.branch_angle_spread = 40.f;

			th.color = t % 2 ? cyan : turquoise;

			step.post_message(msg);
		}
	}

	{
		sound.start(
			step,
			sound_effect_start_input::fire_and_forget(explosion_location).set_listener(subject_if_any),
			predictability
		);
	}

	auto& cosm = step.get_cosmos();

	const auto si = cosm.get_si();
	const auto now = cosm.get_timestamp();

	const auto subject = cosm[subject_if_any];
	const auto subject_alive = subject.alive();

	std::optional<faction_type> sender_faction;

	if (subject_alive) {
		sender_faction = subject.get_official_faction();

		if (subject.template has<components::sentience>()) {
			::mark_caused_danger(subject, sound.modifier, explosion_location);
			sender_faction = subject.get_official_faction();
		}
		else if (const auto sender = subject.template find<components::sender>()) {
			if (const auto sender_capability = cosm[sender->capability_of_sender]) {
				::mark_caused_danger(sender_capability, sound.modifier, explosion_location);
				sender_faction = sender_capability.get_official_faction();
			}
		}
	}

	if (sender_faction == faction_type::SPECTATOR) {
		sender_faction = std::nullopt;
	}

	if (subject_alive) {
		if (const auto sentience = subject.find<components::sentience>()) {
			subject_shake.apply(now, subject.template get<invariants::sentience>(), *sentience);

			impulse_input in;
			in.angular = damage.base / 45.f;

			subject.apply_crosshair_recoil(in);
		}

		if (subject_impulse > 0.f) {
			if (const auto movement_def = subject.find<invariants::movement>()) {
				const auto dash_effect_mult = ::perform_dash(
					subject,
					vec2(subject.get_effective_velocity()).normalize(),

					subject_impulse,
					subject_inert_ms,

					dash_flags()
				);

				::perform_dash_effects(
					step,
					subject,
					dash_effect_mult,
					predictability
				);
			}
		}
	}

	const auto explosion_pos = explosion_location.pos;

	{
		const bool leaves_decal =
			this->type == adverse_element_type::FORCE ||
			this->type == adverse_element_type::FLASH
		;

		if (leaves_decal) {
			/*
				Flashes deal negligible damage, so they get a fixed-size decal instead.
			*/
			const auto size_mult =
				this->type == adverse_element_type::FLASH ?
				FLASH_EXPLOSION_DECAL_SIZE_MULT :
				damage.base / EXPLOSION_DECAL_BASELINE_DAMAGE
			;

			::spawn_explosion_decal(
				step,
				explosion_pos,
				size_mult,
				cause.entity
			);
		}
	}

	if (this->type != adverse_element_type::PED) {
		startle_nearby_organisms(cosm, explosion_pos, effective_radius * 1.8f, 60.f, startle_type::IMMEDIATE);
	}

	const bool causes_chain_reaction = 
		this->type == adverse_element_type::FORCE ||
		this->type == adverse_element_type::INTERFERENCE
	;

	messages::visibility_information_request request;
	request.eye_transform = explosion_location;
	request.filter = predefined_queries::pathfinding();
	request.queried_rect = vec2::square(effective_radius * 2);
	request.subject = subject_if_any;

	auto& response = thread_local_visibility_response();
	visibility_system().calc_visibility(cosm, request, response);

	if (response.empty()) {
		return;
	}

	const auto& physics = cosm.get_solvable_inferred().physics;

	std::unordered_set<unversioned_entity_id> affected_entities_of_bodies;

	/*
		Marks left by the blast on the surfaces it hits.
		Only surfaces whose material defines explosion_decals get them, e.g. glass.

		Where the blast touches a surface:
		every visibility triangle ends exactly on the wall it hit,
		so the triangle's far edge is a stretch of that wall exposed to the blast.

		Where the marks go:
		each stretch is cut into 8px pieces, and each piece gets a weight -
		high close to the blast, zero outside its radius.
		The marks are then spread so that each one covers an equal share of the total weight.
		This puts most of them near the point closest to the blast.

		How many marks:
		the more total weight a surface gets, the more marks, up to MAX_EXPLOSION_DECALS_PER_SURFACE.

		Unlike bullet marks, they do not stack in depth - a blast has no trajectory to march along.
	*/
	const bool leaves_surface_decals = this->type == adverse_element_type::FORCE;

	struct hit_surface_piece {
		const b2Fixture* fixture = nullptr;
		vec2 center;
		real32 weight = 0.f;
	};

	struct hit_surface_pieces_of_victim {
		std::vector<hit_surface_piece> pieces;
		real32 total_weight = 0.f;
	};

	/*
		An ordered map, not unordered - we iterate it to spawn the decals,
		and the order in which they are spawned must be deterministic.
	*/
	std::map<entity_id, hit_surface_pieces_of_victim> hit_surface_pieces;
	auto decal_rng = cosm.get_rng_for(subject_if_any);

	/*
		Checks if the triangle's far edge really lies on this fixture.
		A triangle can also overlap a fixture without ending on it, e.g.:
		- the triangle ends at the edge of the queried rect, with no wall there,
		- the fixture does not block visibility, so the triangle passes through it and ends on another wall behind.
	*/
	auto edge_lies_on_fixture = [&](
		const b2Fixture& fix,
		const vec2 visible_a,
		const vec2 visible_b
	) {
		/*
			We test a point slightly behind the middle of the edge, as seen from the blast.
			If it is inside the fixture, the edge lies on the fixture's face.
		*/
		constexpr auto face_probe_depth_px = 2.f;

		const auto edge_center = (visible_a + visible_b) / 2;
		const auto probe = edge_center + (edge_center - explosion_pos).set_length(face_probe_depth_px);

		return fix.TestPoint(b2Vec2(si.get_meters(probe)));
	};

	/*
		Weight of a spot on a surface: 1 right at the blast, down to 0 at its radius.
		Squared, so that the marks gather near the closest point more strongly.
	*/
	auto calc_surface_weight_at = [&](const vec2 point) {
		const auto proximity = std::max(0.f, 1.f - (point - explosion_pos).length() / effective_radius);
		return proximity * proximity;
	};

	/*
		Cuts the stretch into short pieces and adds the ones within the blast radius.
		A piece's weight is its length times the weight at its center.
	*/
	auto add_hit_surface_pieces = [&](
		hit_surface_pieces_of_victim& of_victim,
		const b2Fixture& fix,
		const vec2 stretch_a,
		const vec2 stretch_b
	) {
		constexpr auto piece_length_px = 8.f;

		const auto stretch = stretch_b - stretch_a;
		const auto num_pieces = std::size_t(1) + static_cast<std::size_t>(stretch.length() / piece_length_px);
		const auto piece_length = stretch.length() / static_cast<real32>(num_pieces);

		for (std::size_t p = 0; p < num_pieces; ++p) {
			const auto center = stretch_a + stretch * ((static_cast<real32>(p) + 0.5f) / static_cast<real32>(num_pieces));
			const auto weight = piece_length * calc_surface_weight_at(center);

			if (weight > 0.f) {
				of_victim.pieces.push_back({ &fix, center, weight });
				of_victim.total_weight += weight;
			}
		}
	};

	for (auto i = 0u; i < response.get_num_triangles(); ++i) {
		const auto visible_triangle = response.get_world_triangle(i, request.eye_transform.pos);

		auto damaging_triangle = visible_triangle;
		damaging_triangle[1] += (damaging_triangle[1] - damaging_triangle[0]).set_length(5);
		damaging_triangle[2] += (damaging_triangle[2] - damaging_triangle[0]).set_length(5);

		if (triangle_degenerate(damaging_triangle)) {
			continue;
		}

		physics.for_each_intersection_with_triangle(
			cosm.get_si(),
			damaging_triangle,
			predefined_queries::force_explosion(),
			[&](
				const b2Fixture& fix,
				const vec2 point_a,
				const vec2 point_b
			) {
				(void)point_a;

				const auto victim_id = get_entity_that_owns(fix);
				const auto victim = cosm[victim_id];

				const bool is_self = 
					subject_alive
					&& (
						victim_id == FixtureUserdata(subject.get_id())
						|| victim.get_owning_transfer_capability() == subject.get_id()
					)
				;

				if (is_self) {
					return callback_result::CONTINUE;
				}

				if (!hit_friendlies && sender_faction.has_value()) {
					if (const auto capability = victim.get_owning_transfer_capability()) {
						if (*sender_faction == capability.get_official_faction()) {
							return callback_result::CONTINUE;
						}
					}
				}

				const bool is_explosion_body = victim.has<components::cascade_explosion>();

				if (is_explosion_body) {
					return callback_result::CONTINUE;
				}

				if (causes_chain_reaction) {
					if (const auto fuse = victim.find<components::hand_fuse>()) {
						if (fuse->armed()) {
							if (fuse->force_detonate_in_ms == -1.f) {
								fuse->force_detonate_in_ms = 200.0f;

								if (type == adverse_element_type::INTERFERENCE) {
									fuse->force_detonate_in_ms = 0.0f;
								}
							}
						}

						return callback_result::CONTINUE;
					}

					if (const auto missile = victim.find<components::missile>()) {
						missile->force_detonate_in_ms = 100;

						if (type == adverse_element_type::INTERFERENCE) {
							missile->force_detonate_in_ms = 0.0f;
						}

						return callback_result::CONTINUE;
					}
				}

				const bool in_range = [&]() {
					b2CircleShape shape;
					shape.m_radius = si.get_meters(effective_radius);

					if (const auto result = shape_overlaps_fixture(&shape, si, explosion_pos, fix)) {
						return true;
					}

					return false;
				}();

				const bool should_be_affected = in_range;

				if (should_be_affected) {
					const auto it = affected_entities_of_bodies.insert(victim_id);
					const bool is_yet_unaffected = it.second;

					if (is_yet_unaffected) {
						messages::damage_message damage_msg;
						damage_msg.type = this->type;
						damage_msg.origin.cause = cause;
						damage_msg.origin.copy_sender_from(subject);
						damage_msg.subject = victim;
						damage_msg.damage = damage;
						damage_msg.impact_velocity = (point_b - explosion_pos).normalize();
						damage_msg.normal = damage_msg.impact_velocity;
						damage_msg.point_of_impact = point_b;

						if (type == adverse_element_type::INTERFERENCE) {
							// TODO: move this calculation after refactoring sentience system to not use messages?
							auto& amount = damage_msg.damage.base;
							amount *= 1 + victim.get_effective_velocity().length() / 1000.f;
						}

						step.post_message(damage_msg);
					}

					const bool may_get_surface_decals =
						leaves_surface_decals
						&& !victim.template has<components::sentience>()
						&& ::find_material_decal_variants(victim, &material_decals_def::explosion_decals) != nullptr
					;

					if (may_get_surface_decals && edge_lies_on_fixture(fix, visible_triangle[1], visible_triangle[2])) {
						add_hit_surface_pieces(hit_surface_pieces[victim.get_id()], fix, visible_triangle[1], visible_triangle[2]);
					}
				}

				return callback_result::CONTINUE;
			}
		);
	}

	for (const auto& [victim_id, of_victim] : hit_surface_pieces) {
		const auto& [pieces, total_weight] = of_victim;

		if (pieces.empty()) {
			continue;
		}

		const auto victim = cosm[victim_id];

		const auto num_marks = std::min(
			std::size_t(1) + static_cast<std::size_t>(total_weight / EXPLOSION_SURFACE_WEIGHT_PER_DECAL),
			MAX_EXPLOSION_DECALS_PER_SURFACE
		);

		/*
			Imagine all the pieces laid out one after another on a line, each as long as its weight.
			Cut this line into num_marks equal parts and put a mark in the middle of each part.
			Heavy pieces take up more of the line, so they get more marks.
		*/
		auto current_piece = std::size_t(0);
		auto weight_before_current = 0.f;

		for (std::size_t m = 0; m < num_marks; ++m) {
			const auto mark_at_weight = (static_cast<real32>(m) + 0.5f) * total_weight / static_cast<real32>(num_marks);

			while (
				current_piece + 1 < pieces.size()
				&& weight_before_current + pieces[current_piece].weight < mark_at_weight
			) {
				weight_before_current += pieces[current_piece].weight;
				++current_piece;
			}

			const auto& piece = pieces[current_piece];
			const auto point = piece.center;

			::spawn_surface_impact_decal(
				step,
				decal_rng,
				victim,
				piece.fixture,
				point,
				(point - explosion_pos).normalize(),
				damage.base,
				0.f,
				&material_decals_def::explosion_decals,
				[]() { return 0.f; }
			);
		}
	}

	{
		physics.for_each_intersection_with_circle_meters( 
			si,
			si.get_meters(effective_radius) * wave_shake_radius_mult,
			explosion_location.to<b2Transform>(si),
			filters[predefined_filter_type::CHARACTER],
			[&](
				const b2Fixture& fix,
				const vec2,
				const vec2
			) {
				const auto victim_id = get_entity_that_owns(fix);
				const auto it = affected_entities_of_bodies.insert(victim_id);
				const bool is_yet_unaffected = it.second;

				if (is_yet_unaffected) {
					const auto victim = cosm[victim_id];

					if (const auto sentience = victim.find<components::sentience>()) {
						auto lesser_shake = damage.shake;
						lesser_shake.duration_ms *= 0.8f;
						lesser_shake.apply(now, victim.get<invariants::sentience>(), *sentience);
					}
				}

				return callback_result::CONTINUE;
			}
		);
	}

	// TODO_PERFORMANCE: This code is unnecessary for the server

	{
		auto msg = messages::exploding_ring_effect(predictability);
		auto& ring = msg.payload;

		ring.outer_radius_start_value = effective_radius / 2;
		ring.outer_radius_end_value = effective_radius;

		ring.inner_radius_start_value = 0.f;
		ring.inner_radius_end_value = effective_radius;
		
		ring.emit_particles_on_ring = true;

		ring.maximum_duration_seconds = ring_duration_seconds;

		ring.color = inner_ring_color;
		ring.center = explosion_pos;
		ring.visibility = response;

		step.post_message(msg);
	}

	{
		auto msg = messages::exploding_ring_effect(predictability);
		auto& ring = msg.payload;

		ring.outer_radius_start_value = effective_radius;
		ring.outer_radius_end_value = effective_radius / 2;

		ring.inner_radius_start_value = effective_radius / 1.5f;
		ring.inner_radius_end_value = effective_radius / 2;
		
		ring.emit_particles_on_ring = true;

		ring.maximum_duration_seconds = ring_duration_seconds;

		ring.color = outer_ring_color;
		ring.center = explosion_pos;
		ring.visibility = response;

		step.post_message(msg);
	}
}