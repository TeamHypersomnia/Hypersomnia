#include "game/components/explosive_component.h"
#include "game/detail/explosive/detonate.h"
#include "game/cosmos/logic_step.h"
#include "game/messages/queue_deletion.h"
#include "game/cosmos/data_living_one_step.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"
#include "game/cosmos/create_entity.hpp"

template <class T, std::size_t I, class F>
auto vectorize_array(const std::array<T, I>& arr, F&& predicate) {
	std::vector<T> output;
	output.reserve(I);

	for (const auto& f : arr) {
		if (predicate(f)) { 
			output.emplace_back(f);
		}
	}

	return output;
}

void detonate(const detonate_input in, const bool destroy_subject) {
	const auto& e = in.explosive;

	if (!e.is_set()) {
		return;
	}

	const auto& step = in.step;
	auto& cosm = step.get_cosmos();

	const auto subject = cosm[in.subject];

	e.explosion.instantiate(step, in.location, damage_cause(subject));

	if (destroy_subject) {
		step.queue_deletion_of(subject, "Detonation");
	}

	const auto cascade_inputs = vectorize_array(e.cascade, [](const auto& f) { return f.flavour_id.is_set(); });

	for (const auto& c_in : cascade_inputs) {
		const auto n = c_in.num_spawned;
		auto rng = cosm.get_nontemporal_rng_for(subject);

		const auto angle_dt = c_in.spawn_spread / n;

		const auto t = in.location;
		const auto rotation = subject.get_effective_velocity().degrees();
		const auto left = rotation - c_in.spawn_spread / 2;

		for (unsigned i = 0; i < n; ++i) {
			const auto target_speed = rng.randval(c_in.initial_speed);
			const auto target_angle_offset = [&]() {
				const auto variation = angle_dt * c_in.spawn_angle_variation;
				const auto h = variation / 2;
				return rng.randval_h(h);
			}();

			const auto vel_angle = target_angle_offset + (n == 1 ? rotation : left + angle_dt * i);

			components::sender sender_info;

			if (const auto maybe_subject_sender = subject.find<components::sender>()) {
				sender_info = *maybe_subject_sender;
			}
			else {
				sender_info.faction_of_sender = subject.get_official_faction();
			}

			sender_info.set_direct(subject);

			const auto cascade_transform = transformr(t.pos, vel_angle);
			const auto cascade_vel = vec2::from_degrees(vel_angle) * target_speed;

			const int explosions_left = rng.randval(c_in.num_explosions);
			const auto next_explosion_in_ms_mult = rng.randval(0.f, 1.0f); ;

			cosmic::queue_create_entity(
				step,
				c_in.flavour_id,
				[sender_info, cascade_transform, cascade_vel, explosions_left, next_explosion_in_ms_mult](const auto typed_handle, auto&&...) {
					typed_handle.template get<components::sender>() = sender_info;
					typed_handle.set_logic_transform(cascade_transform);

					{
						const auto& body = typed_handle.template get<components::rigid_body>();
						body.set_velocity(cascade_vel);
					}

					{
						const auto& cascade_def = typed_handle.template get<invariants::cascade_explosion>();

						auto& cascade = typed_handle.template get<components::cascade_explosion>();

						cascade.explosions_left = explosions_left;
						
						const auto& clk = typed_handle.get_cosmos().get_clock();	

						const auto next_explosion_in_ms = next_explosion_in_ms_mult * cascade_def.explosion_interval_ms.value;

						cascade.when_next_explosion = clk.now;
						cascade.when_next_explosion.step += next_explosion_in_ms / clk.dt.in_milliseconds();
					}
				}
			);
		}
	}
}

void detonate_if(const entity_id& id, const transformr& where, const logic_step& step) {
	step.get_cosmos()[id].dispatch_on_having_all<invariants::explosive>([&](const auto typed_handle) {
		detonate({
			step, id, typed_handle.template get<invariants::explosive>(), where
		});
	});
}
