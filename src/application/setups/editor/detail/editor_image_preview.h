#pragma once
#include "augs/image/image.h"
#include "view/maybe_official_path_types.h"

struct editor_image_preview {
	augs::image image;
	maybe_official_image_path path;
};
