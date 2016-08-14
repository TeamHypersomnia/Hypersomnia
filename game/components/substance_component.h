#pragma once
#include "game/transcendental/entity_handle_declaration.h"
#include "game/enums/processing_subjects.h"

namespace components {
	struct substance {
		int dummy = false;

		template <class Archive>
		void serialize(Archive& ar) {
			ar(
				CEREAL_NVP(dummy)
			);
		}
	};
}
