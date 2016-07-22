#pragma once
#include "game/entity_handle_declaration.h"
#include "game/enums/processing_subjects.h"

namespace components {
	struct substance {
		bool dummy = false;

		template <class Archive>
		void serialize(Archive& ar) {
			ar(
				CEREAL_NVP(dummy)
			);
		}
	};
}
