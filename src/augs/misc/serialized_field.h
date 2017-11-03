#pragma
#include "augs/misc/introspect_declaration.h"
#include "augs/readwrite/memory_stream.h"

namespace augs {
	struct serialized_field {
		unsigned field_index;
		memory_stream data;

		template <class T>
		serialized_field(const T& object, )
	};
}
