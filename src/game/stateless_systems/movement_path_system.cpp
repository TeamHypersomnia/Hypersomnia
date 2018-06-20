#include "augs/misc/randomization.h"

#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"
#include "game/transcendental/logic_step.h"

#include "game/components/transform_component.h"
#include "game/components/sprite_component.h"

#include "game/messages/interpolation_correction_request.h"
#include "game/messages/queue_destruction.h"
#include "game/messages/will_soon_be_deleted.h"

#include "game/stateless_systems/movement_path_system.h"

void movement_path_system::advance_paths(const logic_step step) const {
	auto& cosm = step.get_cosmos();
	const auto delta = step.get_delta();

	auto& npo = cosm.get_solvable_inferred({}).tree_of_npo;

	cosm.for_each_having<components::movement_path>(
		[&](const auto t) {
			auto& movement_path = t.template get<components::movement_path>();
			const auto& movement_path_def = t.template get<invariants::movement_path>();

			auto& transform = t.template get<components::transform>();

			const auto& rotation_speed = movement_path_def.continuous_rotation_speed;

			if (!augs::is_epsilon(rotation_speed)) {
				transform.rotation += rotation_speed * delta.in_seconds();
			}

			if (movement_path_def.rect_bounded.is_enabled) {
				const auto& pos = transform.pos;
				const auto tip_pos = pos + vec2(*t.find_logical_width(), 0).rotate(transform.rotation, vec2());

				const auto& data = movement_path_def.rect_bounded.value;
				const auto size = data.rect_size;

				const auto origin = movement_path.origin;
				const auto bound = xywh::center_and_size(origin.pos, size);

				const auto edges = bound.make_edges();

				auto min_dist = std::numeric_limits<real32>::max();
				
				for (auto& e : edges) {
					const auto dist = tip_pos.distance_from_segment_sq(e[0], e[1]);

					if (dist < min_dist) {
						min_dist = dist;
					}
				}

				min_dist = std::sqrt(min_dist);

				const auto center_dir = (origin.pos - pos).normalize();
				const auto current_dir = vec2::from_degrees(transform.rotation);

				const auto dir_mult = std::max(0.f, 1.f - min_dist / 20.f) / 5;
				auto target_dir = current_dir.lerp(center_dir, dir_mult);

				//LOG_NVPS(current_dir, center_dir, target_dir, dir_mult);

				if (target_dir.is_zero()) {
					target_dir.x = 1;
				}

				auto& anim_state = t.template get<components::animation>().state;

				const auto global_time = cosm.get_total_seconds_passed() + real32(t.get_guid());
				const auto global_time_sine = std::sin(global_time);

				const auto min_speed = 40;
				const auto max_speed = 100 + min_speed;
				const auto speed = static_cast<real32>(global_time_sine * global_time_sine * 100 + 40);
				const auto speed_mult = speed / max_speed;

				transform.pos += target_dir * speed * delta.in_seconds();
				transform.rotation = target_dir.degrees();

				anim_state.frame_elapsed_ms += delta.in_milliseconds() * speed_mult;
			}

			npo.infer_cache_for(t);
		}
	);
}
