#pragma once
#include "augs/graphics/pixel.h"
#include "augs/misc/templated_readwrite.h"
#include "augs/padding_byte.h"
#include "augs/image/image.h"

struct scripted_image_metadata {
	std::vector<augs::image::command_variant> commands;
};

namespace augs {
	template <class A>
	bool read_object(A& ar, scripted_image_metadata& data) {
		return
			read_object(ar, data.commands)
			;
	}

	template <class A>
	void write_object(A& ar, const scripted_image_metadata& data) {
		write_object(ar, data.commands);
	}
}

void regenerate_scripted_images();