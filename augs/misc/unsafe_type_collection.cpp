#include "unsafe_type_collection.h"

namespace augs {
	std::unordered_map<size_t, std::function<void(void*)>> unsafe_type_collection::destructors;
	std::unordered_map<size_t, std::function<void(void*, void*)>> unsafe_type_collection::move_constructors;
}