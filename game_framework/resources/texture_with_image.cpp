#include "texture_with_image.h"

namespace resources {
	void texture_with_image::set_from_image(augs::image img) {
		this->img = img;
		tex.set(&this->img);
	}

	void texture_with_image::set_from_image_file(std::wstring filename) {
		img = augs::image();
		img.from_file(filename);
		tex.set(&img);
	}
}