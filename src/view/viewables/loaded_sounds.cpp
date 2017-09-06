#include "augs/templates/introspect.h"

#include "view/viewables/loaded_sounds.h"

#include "generated/introspectors.h"

loaded_sounds::loaded_sounds(const sound_buffer_inputs_map& definitions) {
	using id_type = assets::sound_buffer_id;
	
	augs::for_each_enum_except_bounds<id_type>([&](const id_type id) {
		const auto* const def = found_or_nullptr(definitions, id);

		if (def != nullptr) {
			try_emplace(id, *def);
		}
	});
}
