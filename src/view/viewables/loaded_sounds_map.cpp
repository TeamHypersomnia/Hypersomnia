#include "augs/templates/container_templates.h"
#include "augs/templates/enum_introspect.h"
#include "view/viewables/loaded_sounds_map.h"

loaded_sounds_map::loaded_sounds_map(const sound_buffer_inputs_map& definitions) {
	using id_type = assets::sound_buffer_id;
	
	augs::for_each_enum_except_bounds([&](const id_type id) {
		const auto* const def = mapped_or_nullptr(definitions, id);

		if (def != nullptr) {
			try_emplace(id, *def);
		}
	});
}
