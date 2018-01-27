#pragma once
#include <string>
#include "augs/zeroed_pod.h"

class entity_type;
struct entity_types;

// TODO: rename to flavour id
using entity_type_id = zeroed_pod<unsigned>;
using entity_name_type = std::wstring;
