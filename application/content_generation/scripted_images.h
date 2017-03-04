#pragma once
#include "augs/graphics/pixel.h"
#include "augs/misc/templated_readwrite.h"
#include "augs/padding_byte.h"
#include "augs/image/image.h"

struct scripted_image_stamp {
	std::vector<augs::image::command_variant> commands;
};

namespace augs {
	template <
		bool C,
		class F
	>
	auto introspect(
		maybe_const_ref_t<C, scripted_image_stamp> data,
		F f
	) {
		return
			f(data.commands)
		;
	}
}

void regenerate_scripted_images(
	const bool force_regenerate
);