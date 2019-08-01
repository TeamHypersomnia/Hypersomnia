#pragma once

template <class D, class E>
void handle_flash_afterimage(
	augs::renderer& renderer,
	all_necessary_shaders& necessary_shaders,
	all_necessary_fbos& necessary_fbos,
	augs::graphics::texture& general_atlas,
	const E& viewed_character,
	const D get_drawer,
	const float flash_mult,
	const bool rendering_flash_afterimage,
	const vec2i screen_size
) {
	if (flash_mult > 0 || rendering_flash_afterimage) {
		/* 
			While capturing, overlay the afterimage onto the final fbo and go back to the default fbo. 
			This is to avoid a single black frame before the afterimage actually kicks in.
		*/

		if (rendering_flash_afterimage) {
			augs::graphics::fbo::set_current_to_none(renderer);
		}

		necessary_fbos.flash_afterimage->get_texture().set_as_current(renderer);
		necessary_shaders.flash_afterimage->set_as_current(renderer);
		necessary_shaders.flash_afterimage->set_projection(renderer, augs::orthographic_projection(vec2(screen_size)));

		{
			auto flash_afterimage_col = white;
			flash_afterimage_col.mult_alpha(flash_mult);

			auto whole_texture = augs::atlas_entry();
			whole_texture.atlas_space.x = 0.f;
			whole_texture.atlas_space.y = 0.f;
			whole_texture.atlas_space.w = 1.f;
			whole_texture.atlas_space.h = 1.f;

			get_drawer().color_overlay(whole_texture, screen_size, flash_afterimage_col, flip_flags::make_vertically());
		}

		renderer.call_and_clear_triangles();

		necessary_shaders.standard->set_as_current(renderer);
		general_atlas.set_as_current(renderer);
	}

	if (!rendering_flash_afterimage) {
		/* Don't draw white blinding overlay while we are capturing the flash afterimage */

		if (flash_mult > 0) {
			renderer.call_and_clear_triangles();

			auto flash_overlay_col = white;

			if (viewed_character) {
				if (const auto sentience = viewed_character.template find<components::sentience>()) {
					if (!sentience->is_conscious()) {
						flash_overlay_col = rgba(255, 40, 40, 255);
					}
				}
			}

			flash_overlay_col.mult_alpha(flash_mult);

			get_drawer().color_overlay(screen_size, flash_overlay_col);
		}
	}
}

