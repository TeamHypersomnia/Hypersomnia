#pragma
#include "augs/misc/introspect_declaration.h"
#include "augs/readwrite/streams.h"

namespace augs {
	struct serialized_field {
		unsigned field_index;
		stream data;

		template <class T>
		serialized_field(const T& object, )
	};
}
