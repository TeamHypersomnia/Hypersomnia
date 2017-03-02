#include "augs/ensure.h"

#include "texture_atlas.h"
#include "augs/texture_atlas/texture_with_image.h"
#include "augs/graphics/renderer.h"

namespace augs {
	texture_atlas::~texture_atlas() {
		destroy();
	}

	texture_atlas(
		std::vector<const augs::image*> packed_images
	);

	bool texture_atlas::pack() {
		GLint tsize;
		glGetIntegerv(GL_MAX_TEXTURE_SIZE, &tsize); glerr;;
		return pack(tsize);
	}

	bool texture_atlas::pack(int max_size) {
		int cnt = textures.size();

		std::vector<rect_xywhf*> ptr_arr;
		ptr_arr.resize(cnt, nullptr);

		rect_xywhf** p = ptr_arr.data();

		for (int i = 0; i < cnt; ++i)
		{
			p[i] = &textures[i]->tex.rect;
			p[i]->w += 2;
			p[i]->h += 2;
		}

		bool res = ::pack(p, cnt, max_size, bins);
		ensure(res && "there's a texture larger than maximum atlas size");

		for (int i = 0; i < cnt; ++i)
		{
			p[i]->w -= 2;
			p[i]->h -= 2;
		}

		return res;
	}

	void texture_atlas::create_image(const bool destroy_images) {
		ensure(bins.size() == 1);
		const auto& b = bins[0];

		double u = 1.0 / b.size.w;
		double v = 1.0 / b.size.h;

		for (unsigned i = 0; i < textures.size(); ++i) {
			textures[i]->tex.set_uv_unit(u, v);
		}

		img.create({ static_cast<unsigned>(b.size.w), static_cast<unsigned>(b.size.h) });
		img.fill(rgba(0, 0, 0, 0));

		for (unsigned i = 0; i < textures.size(); ++i) {
			const auto rc = textures[i]->tex.get_rect();

			img.blit(
				textures[i]->img,
				{ static_cast<unsigned>(rc.x), static_cast<unsigned>(rc.y) },
				rc.flipped
			);
		}

		img.swap_red_and_blue();
	}


	void texture_atlas::build(bool _mipmaps, bool _linear, image* raw_texture) {
		destroy();
		mipmaps = _mipmaps, lin = _linear;

		image& im = raw_texture ? *raw_texture : img;

		glGenTextures(1, &id); glerr;
		augs::renderer::get_current().bind_texture(*this);

		lin = !lin; if (!lin) linear(); else nearest();

		int format = 4;
		
		switch (format) {
		case 1: format = GL_LUMINANCE; break;
		case 2: format = GL_LUMINANCE_ALPHA; break;
		case 3: format = GL_BGR; break;
		case 4: format = GL_BGRA; break;
		default: ensure(false);
		}

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, im.get_size().x, im.get_size().y, 0, format, GL_UNSIGNED_BYTE, im.get_data()); glerr;

		if (mipmaps) glGenerateMipmap(GL_TEXTURE_2D); glerr;
		built = true;

		entry_for_whole_atlas.set(im);
		entry_for_whole_atlas.set_uv_unit(1.0 / im.get_size().x, 1.0 / im.get_size().y);
	}

	void texture_atlas::default_build() {
		pack();
		create_image(false);
		build(false, false);
		/* destroy the raw image as it is already uploaded to GPU */
		img.destroy();
	}
}
