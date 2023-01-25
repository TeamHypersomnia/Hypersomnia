#pragma once
#include "augs/misc/pool/pooled_object_id.h"

struct editor_layer;

using editor_layer_pool_size_type = unsigned;
using editor_layer_id = augs::pooled_object_id<editor_layer_pool_size_type, editor_layer>;
