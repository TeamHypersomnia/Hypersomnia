#include "thunder_system.h"
#include "game/detail/camera_cone.h"
#include "augs/templates/container_templates.h"
#include "augs/graphics/drawers.h"
#include "game/resources/manager.h"
#include "game/systems_temporary/physics_system.h"
#include "game/systems_audiovisual/particles_simulation_system.h"
#include "game/enums/filters.h"
#include "game/transcendental/cosmos.h"
#include "game/detail/particle_types.h"

void thunder_system::thunder::create_root_branch() {
	static thread_local fast_randomization rng;

	thunder::branch b;
	b.lifetime_ms = 0.f;
	b.from = in.first_branch_root.pos;
	b.to = b.from + vec2().set_from_degrees(
		in.first_branch_root.rotation + rng.randval(in.branch_angle_spread)
	) * rng.randval(in.branch_length);
	b.max_lifetime_ms = rng.randval(in.max_branch_lifetime_ms);

	until_next_branching_ms = rng.randval(in.delay_between_branches_ms);
	++num_active_branches;

	branches.push_back(b);
}

void thunder_system::add(const thunder::input in) {
	ensure(in.max_all_spawned_branches > 0);

	thunder new_thunder;
	new_thunder.in = in;
	new_thunder.create_root_branch();

	thunders.push_back(new_thunder);
}

void thunder_system::advance(
	const cosmos& cosmos,
	const augs::delta dt,
	particles_simulation_system& particles_output_for_effects
) {
	static thread_local fast_randomization rng;

	for (thunder& t : thunders) {
		t.until_next_branching_ms -= dt.in_milliseconds();

		const bool should_branch_now = t.until_next_branching_ms <= 0.f;

		if (should_branch_now) {
			t.until_next_branching_ms = rng.randval(t.in.delay_between_branches_ms);

			bool found_suitable_parent = false;

			for (size_t i = 0; i < t.branches.size(); ++i) {
				auto& b = t.branches[i];

				const bool is_leaf = b.children.empty();

				if (is_leaf && b.can_have_children) {
					const auto num_children = std::min(t.in.max_all_spawned_branches, rng.randval(0u, t.in.max_branch_children));

					for (auto i = 0u; i < num_children; ++i) {
						thunder::branch child;
						child.activated = true;
						child.lifetime_ms = 0.f;
						child.from = b.to;
						child.to =
							child.from + vec2().set_from_degrees(rng.randval(t.in.branch_angle_spread) + (b.to - b.from).degrees()) * rng.randval(t.in.branch_length);
						child.max_lifetime_ms = rng.randval(t.in.max_branch_lifetime_ms);

						const auto raycast = cosmos.systems_temporary.get<physics_system>().ray_cast_px(
							cosmos.get_si(),
							child.from,
							child.to,
							filters::bullet()
						);

						if (raycast.hit) {
							child.can_have_children = false;
							child.to = raycast.intersection;
						}

						b.children.push_back(t.branches.size());
						t.branches.push_back(child);
						++t.num_active_branches;
						--t.in.max_all_spawned_branches;
					}

					found_suitable_parent = true;
				}
			}

			if (!found_suitable_parent) {
				t.create_root_branch();
			}
		}

		for (auto& b : t.branches) {
			b.lifetime_ms += dt.in_milliseconds();
			
			if (b.activated && b.lifetime_ms > b.max_lifetime_ms) {
				const bool is_leaf = b.children.empty();

				if (is_leaf) {
					const auto remnants_emission = get_resource_manager().find(assets::particle_effect_id::THUNDER_REMNANTS)->at(0);

					{
						const auto spawner = [&](auto dummy) {
							auto& new_p = particles_output_for_effects.spawn_particle<decltype(dummy)>(
								rng,
								0.f,
								{ 20.f, 100.f },
								b.to,
								0.f,
								360.f,
								remnants_emission
							);

							new_p.colorize(t.in.color.rgb());

							particles_output_for_effects.add_particle(remnants_emission.particle_render_template.layer, new_p);
						};

						for (size_t i = 0; i < rng.randval(2u, 16u); ++i) {
							spawner(general_particle());
						}
					}
				}

				b.activated = false;
				--t.num_active_branches;
			}
		}
	}

	erase_remove(thunders, [&](const thunder& t) {
		return t.num_active_branches == 0;
	});
}

void thunder_system::draw_thunders(
	augs::vertex_line_buffer& lines,
	const camera_cone camera
) const {
	for (const auto& t : thunders) {
		for (const auto& b : t.branches) {
			if (b.activated) {
				augs::draw_line(lines, camera[b.from], camera[b.to], get_resource_manager().find(assets::texture_id::BLANK)->texture_maps[texture_map_type::DIFFUSE], t.in.color);
			}
		}
	}
}