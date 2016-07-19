#include "texture_with_image.h"
#include "augs/filesystem/file.h"
#include "game/build_settings.h"

namespace resources {
	void texture_with_image::set_from_image(augs::image img) {
		this->img = img;
		tex.set(&this->img);
	}

	void texture_with_image::set_from_image_file(std::string filename) {
		img = augs::image();
		img.from_file(filename);
		tex.set(&img);
		filename.resize(filename.size() - 4);
		filename += "_polygonized.png";

#if ENABLE_POLYGONIZATION_IN_DEBUG || !(_DEBUG || NDEBUG)
		if (augs::file_exists(filename))
			polygonize_from_file(filename);
#endif
	}
	
	void texture_with_image::polygonize_from_file(std::string filename) {
		auto polygonization_info = augs::image();
		polygonization_info.from_file(filename, false);
		polygonized = polygonization_info.get_polygonized();
	}
}