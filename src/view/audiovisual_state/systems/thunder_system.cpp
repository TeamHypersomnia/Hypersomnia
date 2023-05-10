#include "augs/misc/randomization.h"
#include "augs/templates/container_templates.h"
#include "augs/drawing/drawing.hpp"

#include "game/enums/filters.h"

#include "game/cosmos/cosmos.h"

#include "game/inferred_caches/physics_world_cache.h"

#include "view/viewables/all_viewables_declaration.h"
#include "view/viewables/particle_effect.h"
#include "view/viewables/particle_types.h"

#include "view/audiovisual_state/systems/particles_simulation_system.h"
#include "view/audiovisual_state/systems/thunder_system.h"
#include "view/viewables/particle_types.hpp"

void thunder_system::clear() {
	thunders.clear();
}

void thunder_system::thunder::create_root_branch(randomization& rng) {
	if (container_full(branches)) {
		return;
	}

	thunder::branch b;
	b.current_lifetime_ms = 0.f;

	b.from = in.first_branch_root.pos;
	b.to = b.from + vec2::from_degrees(
		in.first_branch_root.rotation + rng.randval_h(in.branch_angle_spread)
	) * rng.randval(in.branch_length);

	b.max_lifetime_ms = rng.randval(in.max_branch_lifetime_ms);

	until_next_branching_ms = rng.randval(in.delay_between_branches_ms);
	++num_active_branches;

	branches.push_back(b);
}

void thunder_system::add(
	randomization& rng,
	const thunder_input in
) {
	ensure(in.max_all_spawned_branches > 0);

	if (container_full(thunders)) {
		return;
	}

	thunder new_thunder;
	new_thunder.in = in;
	new_thunder.create_root_branch(rng);

	thunders.push_back(new_thunder);
}

void thunder_system::advance(
	randomization& rng,
	const cosmos& cosm,
	const camera_cone queried_cone,
	const particle_effects_map& manager,
	const augs::delta dt,
	particles_simulation_system& particles_output_for_effects
) {
	const auto queried_camera_aabb = queried_cone.get_visible_world_rect_aabb();

	for (thunder& t : thunders) {
		t.until_next_branching_ms -= dt.in_milliseconds();

		const bool should_branch_now = t.until_next_branching_ms <= 0.f;

		if (should_branch_now) {
			t.until_next_branching_ms = rng.randval(t.in.delay_between_branches_ms);

			bool found_suitable_parent = false;

			const auto currently_existing_branches_num = t.branches.size();

			for (size_t i = 0; i < currently_existing_branches_num; ++i) {
				const bool is_leaf = t.branches[i].children.empty();

				if (is_leaf && t.branches[i].can_have_children) {
					const auto num_children = std::min(t.in.max_all_spawned_branches, rng.randval(0u, t.in.max_branch_children));

					for (auto ch = 0u; ch < num_children; ++ch) {
						auto& b = t.branches[i];

						if (container_full(b.children) || container_full(t.branches)) {
							break;
						}

						thunder::branch child;
						child.activated = true;
						child.current_lifetime_ms = 0.f;
						
						child.from = b.to;
						child.to =
							child.from 
							+ vec2::from_degrees(rng.randval_h(t.in.branch_angle_spread) 
							+ (b.to - b.from).degrees()) * rng.randval(t.in.branch_length)
						;

						child.max_lifetime_ms = rng.randval(t.in.max_branch_lifetime_ms);

						const auto raycast = cosm.get_solvable_inferred().physics.ray_cast_px(
							cosm.get_si(),
							child.from,
							child.to,
							filters[predefined_filter_type::CHARACTER]
						);

						if (raycast.hit) {
							child.can_have_children = false;
							child.to = raycast.intersection;
						}

						b.children.push_back(static_cast<int>(t.branches.size()));
						t.branches.push_back(child);
						++t.num_active_branches;
						--t.in.max_all_spawned_branches;
					}

					found_suitable_parent = true;
				}
			}

			if (!found_suitable_parent) {
				t.create_root_branch(rng);
			}
		}

		for (auto& b : t.branches) {
			b.current_lifetime_ms += dt.in_milliseconds();
			
			if (b.activated && b.current_lifetime_ms > b.max_lifetime_ms) {
				const bool is_leaf = b.children.empty();
				const bool visible = queried_camera_aabb.hover(ltrb::from_points(b.from, b.to));

				if (is_leaf && visible) {
					const auto* const remnants = mapped_or_nullptr(manager, cosm.get_common_assets().thunder_remnants);

					if (remnants != nullptr) {
						const auto& remnants_emission = remnants->emissions.at(0);

						{
							const auto spawner = [&](auto dummy) {
								auto new_p = particles_output_for_effects.spawn_particle<decltype(dummy)>(
									rng,
									0.f,
									remnants_emission.base_speed,
									b.to,
									0.f,
									rng.randval(remnants_emission.spread_degrees),
									remnants_emission
								);

								new_p.colorize(t.in.color.rgb());

								particles_output_for_effects.add_particle(remnants_emission.target_layer, new_p);
							};

							for (std::size_t i = 0; i < rng.randval(2u, 16u); ++i) {
								spawner(general_particle());
							}
						}
					}
				}

				b.activated = false;
				--t.num_active_branches;
			}
		}
	}

	erase_if(thunders, [&](const thunder& t) {
		return t.num_active_branches == 0 && t.in.max_all_spawned_branches == 0;
	});
}

void thunder_system::draw_thunders(
	const augs::line_drawer_with_default output,
	const camera_cone queried_cone
) const {
	const auto queried_camera_aabb = queried_cone.get_visible_world_rect_aabb();

	for (const auto& t : thunders) {
		for (const auto& b : t.branches) {
			if (b.activated) {
				if (queried_camera_aabb.hover(ltrb::from_points(b.from, b.to))) {
					output.line(b.from, b.to, t.in.color);
				}
			}
		}
	}
}