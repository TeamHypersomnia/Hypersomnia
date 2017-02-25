#include "thunder_system.h"
#include "game/detail/camera_cone.h"
#include "augs/templates/container_templates.h"
#include "augs/graphics/drawers.h"
#include "game/resources/manager.h"

void thunder_system::add(const thunder::input in) {
	static thread_local fast_randomization rng;

	thunder new_thunder;
	new_thunder.in = in;

	ensure(in.max_all_spawned_branches > 0);

	thunder::branch b;
	b.lifetime_ms = 0.f;
	b.from = in.first_branch_root.pos;
	b.to = b.from + vec2().set_from_degrees(
		in.first_branch_root.rotation + rng.randval(in.branch_angle_spread)
	) * rng.randval(in.branch_length);
	b.max_lifetime_ms = rng.randval(in.max_branch_lifetime_ms);

	new_thunder.until_next_branching_ms = rng.randval(in.delay_between_branches_ms);
	new_thunder.num_active_branches = 1u;

	new_thunder.branches.emplace_back(std::move(b));
	thunders.emplace_back(std::move(new_thunder));
}

void thunder_system::advance(
	const cosmos&,
	const augs::delta dt,
	particles_simulation_system& particles_output_for_effects
) {
	static thread_local fast_randomization rng;

	for (thunder& t : thunders) {
		t.until_next_branching_ms -= dt.in_milliseconds();

		if (t.until_next_branching_ms <= 0.f) {
			t.until_next_branching_ms = rng.randval(t.in.delay_between_branches_ms);

			for (auto& b : t.branches) {
				const bool is_leaf = b.children.empty();
				
				if (is_leaf) {
					const auto num_children = std::min(t.in.max_all_spawned_branches, rng.randval(0u, t.in.max_branch_children));

					for (auto i = 0u; i < num_children; ++i) {
						thunder::branch child;
						child.lifetime_ms = 0.f;
						child.from = b.to;
						child.to =
							child.from + vec2().set_from_degrees(rng.randval(t.in.branch_angle_spread) + (b.to - b.from).degrees()) * rng.randval(t.in.branch_length);
						child.max_lifetime_ms = rng.randval(t.in.max_branch_lifetime_ms);

						b.children.push_back(b.children.size());
						t.branches.push_back(child);
						++t.num_active_branches;
						--t.in.max_all_spawned_branches;
					}
				}
			}
		}

		for (auto& bb : t.branches) {
			bb.lifetime_ms += dt.in_milliseconds();
			
			if (bb.activated && bb.lifetime_ms > bb.max_lifetime_ms) {
				bb.activated = false;
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
				augs::draw_line(lines, camera[b.from], camera[b.to], get_resource_manager().find(assets::texture_id::BLANK)->tex, t.in.color);
			}
		}
	}
}