#pragma once

namespace components {
	struct input_receiver {
		bool local = true;

		template <class Archive>
		void serialize(Archive& ar) {
			ar(CEREAL_NVP(local));
		}
	};
}



