#pragma once
#include "augs/templates/type_mod_templates.h"

#include "game/assets/ids/asset_ids.h"
#include "augs/misc/pool/pool.h"

template <class pooled_type, class... keys>
using make_asset_pool = augs::pool<pooled_type, make_vector, asset_pool_id_size_type, keys...>;
