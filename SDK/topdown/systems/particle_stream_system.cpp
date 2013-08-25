#include "particle_stream_system.h"
/* for utilities */
#include "particle_emitter_system.h"

#include "../components/particle_emitter_component.h"
#include "../messages/destroy_message.h"

void particle_stream_system::process_entities(world& owner) {
	double delta = timer.extract<std::chrono::milliseconds>();

	for (auto it : targets) {
		auto& stream = it->get<components::particle_stream>();
		auto& transform = it->get<components::transform>();

		delta = std::min(delta, static_cast<double>(stream.max_lifetime_ms - stream.lifetime_ms));

		stream.lifetime_ms += delta;
		stream.lifetime_ms = std::min(stream.lifetime_ms, stream.max_lifetime_ms);

		stream.particles_to_spawn += randval(stream.info->particles_per_sec_min, stream.info->particles_per_sec_max) * (delta / 1000.0);

		while (stream.particles_to_spawn >= 1.f) {
			particle_emitter_system::spawn_particle(
				stream.info->target_particle_group->get<components::particle_group>(), 
				transform.current.pos, transform.current.rotation, *stream.info);
			stream.particles_to_spawn -= 1.f;
		}

		if (stream.lifetime_ms >= stream.max_lifetime_ms)
			owner.post_message(messages::destroy_message(it));
	}
}