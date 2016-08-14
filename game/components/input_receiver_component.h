#pragma once

namespace components {
	struct input_receiver {
		int local = true;

		template <class Archive>
		void serialize(Archive& ar) {
			ar(CEREAL_NVP(local));
		}
	};
}



