#pragma once
#include "stdafx.h"

#include "../options.h"
#include "texture_baker.h"
#include "font.h"

namespace augmentations {
	namespace texture_baker {
		void texture::set_uv_unit(double u, double v) {
			x = float(double(rect.x) * u);
			y = float(double(rect.y) * v);
			w = float(double(rect.w) * u);
			h = float(double(rect.h) * v);
		}

		texture::texture() : img(nullptr), ltoa(false) {}
		texture::texture(image* img) : img(img), rect(img->get_size()), ltoa(false) {}

		void texture::set(image* _img) {
			img = _img;
			rect = rects::xywhf(img->get_size());
		}

		void texture::luminosity_to_alpha(bool flag) {
			ltoa = flag;
		}

		rects::xywhf texture::get_rect() const {
			return rect;
		}

		vec2<int> texture::get_size() const {
			return !rect.flipped ? vec2<int>(rect.w, rect.h) : vec2<int>(rect.h, rect.w);
		}

		void texture::get_uv(float u, float v, float& u_out, float& v_out) const {
			if(!rect.flipped) {
				u_out = x + w * u;
				v_out = y + h * v;
			}
			else {
				u_out = x + w * v;
				v_out = y + h * u;
			}
		}
		
		void texture::get_uv(vec2<float>& texture_space) const {
			auto temp = texture_space;
			get_uv(temp.x, temp.y, texture_space.x, texture_space.y);
		}

		float texture::get_u(int vertex_num_from_cw_rect) const {
			vertex_num_from_cw_rect %= 4;
			float u = 0.f;

			switch(vertex_num_from_cw_rect) {
			case 0: u = 0.f; break; 
			case 1: u = rect.flipped ? 0.f : 1.f; break; 
			case 2: u = 1.f; break; 
			case 3: u = rect.flipped ? 1.f : 0.f; break; 
			default: break;
			}

			return x + w * u;
		}

		float texture::get_v(int vertex_num_from_cw_rect) const {
			vertex_num_from_cw_rect %= 4;
			float v = 0.f;

			switch(vertex_num_from_cw_rect) {
			case 0: v = 0.f; break; 
			case 1: v = rect.flipped ? 1.f : 0.f; break; 
			case 2: v = 1.f; break; 
			case 3: v = rect.flipped ? 0.f : 1.f; break; 
			default: break;
			}

			return y + h * v;
		}

		void texture::translate_uv(vec2<float> uv) {
			uv *= vec2<float>(w/rect.w, h/rect.h);
			x += uv.x;
			y += uv.y;
		}

		void texture::scale_uv(float u_scalar, float v_scalar) {
			w *= u_scalar;
			h *= v_scalar;
		}

		void texture::get_uv(const rects::texture& uv, rects::texture& out) const {
			get_uv(uv.u1, uv.v1, out.u1, out.v1);
			get_uv(uv.u2, uv.v2, out.u2, out.v2);
		}

		float texture::get_u_unit() const {
			return rect.flipped ? h : w;
		}

		float texture::get_v_unit() const {
			return rect.flipped ? w : h;
		}

		atlas::atlas(int wp) : built(false), rep(true), lin(false), mipmaps(false) {
		}

		atlas::~atlas() { 
			destroy(); 
		}

		unsigned atlas::current = 0;

		bool atlas::gen_packed(std::vector<texture*>& in_textures, 
			std::vector<atlas*>& inout_atlases) {
				return true;
		}

		void atlas::quick_add(image* in_images, texture* out_textures, int n_tex, font_file* in_files, font* out_fonts, int n_fonts) {
			if(in_images && out_textures) {
				for(int i = 0; i < n_tex; ++i) {
					out_textures[i].set(in_images + i);
					textures.push_back(out_textures + i);
				}
			}
			if(in_files && out_fonts) {
				for(int i = 0; i < n_fonts; ++i) {
					out_fonts[i].build(in_files + i);
					out_fonts[i].add_to_atlas(*this);
				}
			}
		}

		bool atlas::pack() {
			GLint tsize;
			glGetIntegerv(GL_MAX_TEXTURE_SIZE, &tsize);
			return pack(tsize);
		} 

		bool atlas::pack(int max_size) {
			int cnt = textures.size();
			ptr_arr.reserve(cnt);

			rects::xywhf** p = ptr_arr.data();

			for(int i = 0; i < cnt; ++i)
				p[i] = &textures[i]->rect;

			int res = rect2D(p, cnt, max_size, b);

			bool notenough = res != 1;
			bool texlarger = res != 2;
			errs(notenough, "not enough space");
			errs(texlarger, "texture larger than max_size");

			return !res;
		}

		void atlas::create_image(int atlas_channels, bool destroy_images) {
			double u = 1.0 / b.size.w; 
			double v = 1.0 / b.size.h;

			for(unsigned i = 0; i < textures.size(); ++i)
				textures[i]->set_uv_unit(u, v);

			unsigned char pixel[] = { 0, atlas_channels == 2 ? 255 : 0, 0, 255 };
			img.create(b.size.w, b.size.h, atlas_channels);
			img.fill(pixel);

			rects::xywhf rc;
			for(unsigned i = 0; i < textures.size(); ++i) {
				rc = textures[i]->get_rect();
				img.blit(*textures[i]->img, rc.x, rc.y, rects::xywhf(0, 0, rc.w, rc.h, rc.flipped), textures[i]->ltoa);  

				if(destroy_images)
					textures[i]->img->destroy();
			}
		}

		bool atlas::is_mipmapped() const {
			return mipmaps;
		}

		void atlas::destroy() {
			if(built)
				glDeleteTextures(1, &id);

			rep = true;
			lin = mipmaps = built = false;
		}

		void atlas::build(bool _mipmaps, bool _linear, image* raw_texture) {
			destroy();
			mipmaps = _mipmaps, lin = _linear;

			image& im = raw_texture ? *raw_texture : img;

			glGenTextures(1, &id);
			_bind();

			lin = !lin; if(!lin) linear(); else nearest();

			int format = im.get_channels();
			switch(format) {
			case 1: format = GL_LUMINANCE; break;
			case 2: format = GL_LUMINANCE_ALPHA; break;
			case 3: format = GL_BGR; break;
			case 4: format = GL_BGRA; break;
			}

			glTexImage2D(GL_TEXTURE_2D, 0, im.get_channels(), im.get_size().w, im.get_size().h, 0, format, GL_UNSIGNED_BYTE, im.ptr());

			if(mipmaps) glGenerateMipmap(GL_TEXTURE_2D);

			built = true;

			atlas_texture.set(&im);
			atlas_texture.set_uv_unit(1.0/im.get_size().w, 1.0/im.get_size().h);
		}
		
		void atlas::default_build() {
			pack();
			create_image(4, true);
			build(false, false);
			/* destroy the raw image as it is already uploaded to GPU */
			img.destroy();
		}

		void atlas::bind() {
			if(current != id) _bind();
		}

		void atlas::_bind() {
			glBindTexture(GL_TEXTURE_2D, current = id);
		}

		void atlas::repeat() {
			if(!rep) {
				rep = true;
				bind();
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			}
		}

		void atlas::clamp() {
			if(rep) {
				rep = false;
				bind();
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
			}
		}

		void atlas::nearest() {
			bind();
			lin = false;
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mipmaps ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST); 
		}

		void atlas::linear() {
			bind();
			lin = true;
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mipmaps ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR); 
		}
	}
}