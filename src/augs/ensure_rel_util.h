#pragma once
#include "augs/ensure_rel.h"

#define ensure_eq_id(a, b) \
ensure_eq(a.indirection_index, b.indirection_index) \
ensure_eq(a.version, b.version)
