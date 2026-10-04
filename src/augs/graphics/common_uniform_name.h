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
		light_core,
		light_intensity,
		physical_lights,
		multiply_color,
		texture_center,
		black_cutoff,

		basic_texture,
		smoke_texture,
		light_texture,
		afterimage_texture,
		global_color,

		shadow_texture,
		shadow_texture_offset,
		shadow_step,
		shadow_strength,
		shadow_fix,
		receiver_height,
		receiver_displacement,
		ambient_color,
		shadow_hue_preservation,
		brighten_lights,
		light_pass,
		light_mask_texture,
		removed_light_texture,
		hue_light_texture,
		point_light_hue_preservation,
		light_resolve,
		resolved_light_texture,
		smoke_flat_intensity,
		color_encodes_footprints,
		smooth_brightening,

		COUNT
		// END GEN INTROSPECTOR
	};
}
