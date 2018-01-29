#pragma once
#include <string>
#include "augs/zeroed_pod.h"

class entity_flavour;
struct entity_flavours;

// TODO: rename to flavour id
using entity_flavour_id = zeroed_pod<unsigned>;
using entity_name_type = std::wstring;
