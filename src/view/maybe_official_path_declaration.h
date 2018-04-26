#pragma once
#include "game/assets/ids/asset_ids.h"

template <class T>
struct maybe_official_path;

using maybe_official_image_path = maybe_official_path<assets::image_id>;
using maybe_official_sound_path = maybe_official_path<assets::sound_id>;
