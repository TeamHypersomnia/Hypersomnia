#pragma once

namespace augs {
	enum class common_uniform_name {
		// GEN INTROSPECTOR enum class augs::common_uniform_name
		projection_matrix,
		startingAngleVec,
		endingAngleVec,
		eye_frag_pos,
		light_pos,
		distance_mult,
		max_distance,
		cutoff_distance,
		light_attenuation,
		multiply_color,
		texture_center,
		black_cutoff,

		basic_texture,
		smoke_texture,
		light_texture,
		afterimage_texture,
		global_color,

		shadow_texture,
		shadow_step,
		shadow_strength,
		shadow_fix,
		receiver_height,
		receiver_displacement,
		ambient_color,
		fully_lit,
		shadow_hue_preservation,
		quantize_lights,
		light_pass,
		light_mask_texture,
		removed_light_texture,
		removed_light_available,
		hue_light_texture,
		point_light_hue_preservation,

		COUNT
		// END GEN INTROSPECTOR
	};
}
