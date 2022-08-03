#pragma once
#include "augs/misc/pool/pooled_object_id.h"

struct editor_layer;

using editor_layer_id = augs::pooled_object_id<unsigned short, editor_layer>;
