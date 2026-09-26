precision mediump int;
precision mediump float;

smooth in vec4 theColor;

/*
	Penumbras of shadows set y to 1 and x to the position across the penumbra, from 0 at the lit side to 1.
*/

in vec2 theTexcoord;

out vec4 outputColor;

uniform vec2 light_pos;
uniform vec3 light_attenuation;
uniform vec3 multiply_color;
uniform float distance_mult;
uniform float max_distance;
uniform float cutoff_distance;

/*
	Which pass of light_system.cpp this is.
	Every light first builds the mask of its shadows in the alpha of the light texture:

	6 - resetting the mask of a light without a height to fully dark (min blending),
	8 - its visibility polygon and penumbras, into its mask (max blending),
	3 - resetting the mask of a light with a height to fully lit (max blending),
	2 - shadows of a light with a height, into its mask (min blending) - first of the obstacles reaching the ceiling,
	9 - (in between) the light removed by the lower obstacles where the ceiling-high ones let it through,
	    into the hue light texture, reading the mask from light_mask_texture,
	2 - then shadows of the lower obstacles.

	Then:

	4 - the light itself over its whole reach, premultiplied, to be scaled by its mask,
	7 - the light all shadows removed, into the removed light texture
	    (and into the hue light texture for lights keeping their hue through walls).
*/

uniform int light_pass;
uniform sampler2D light_mask_texture;

float penumbra_fade(float t) {
	return 1.0 - t * t * (3.0 - 2.0 * t);
}

/*
	How lit the ground is under a shadow of a light with a height - see light_height_shadows.h.
*/

float height_shadow_lit() {
	float lit_by_side = penumbra_fade(clamp(theTexcoord.x, 0.0, 1.0));
	float lit_past_end = 1.0 - penumbra_fade(clamp(theTexcoord.y, 0.0, 1.0));

	return 1.0 - (1.0 - lit_by_side) * (1.0 - lit_past_end);
}

void main() 
{	
	if (light_pass == 2) {
		outputColor = vec4(1.0, 1.0, 1.0, height_shadow_lit());
		return;
	}

	if (light_pass == 6) {
		outputColor = vec4(1.0, 1.0, 1.0, 0.0);
		return;
	}

	if (light_pass == 8) {
		float lit = theTexcoord.y > 0.5 ? penumbra_fade(clamp(theTexcoord.x, 0.0, 1.0)) : 1.0;
		outputColor = vec4(0.0, 0.0, 0.0, lit);
		return;
	}

	if (light_pass == 3) {
		outputColor = vec4(0.0, 0.0, 0.0, 1.0);
		return;
	}

	float light_distance = length(gl_FragCoord.xy - light_pos) * distance_mult;
	vec4 final_color = theColor;
	final_color.rgb *= multiply_color;

	final_color.a = 1.0 / (
		light_attenuation.x
		+ light_attenuation.y * light_distance
		+ light_attenuation.z * light_distance * light_distance
	); 

	if (light_distance > max_distance) {
		discard;
	}
	else if (light_distance > cutoff_distance) {
		final_color.a *= 0.5;

		float cutoff_len = max_distance - cutoff_distance;
		float cutoff_amt = (light_distance - cutoff_distance) / cutoff_len;
		cutoff_amt *= cutoff_amt;

		final_color.a *= 1.0 - cutoff_amt;
	}

	final_color.a = min(theColor.a, final_color.a);

	vec3 premultiplied = final_color.rgb * final_color.a;

	if (light_pass == 4) {
		outputColor = vec4(premultiplied, 0.0);
		return;
	}

	if (light_pass == 7 || light_pass == 9) {
		float mask = texture(light_mask_texture, gl_FragCoord.xy / vec2(textureSize(light_mask_texture, 0))).a;

		if (light_pass == 7) {
			outputColor = vec4(premultiplied * (1.0 - mask), 0.0);
		}
		else {
			outputColor = vec4(premultiplied * (1.0 - height_shadow_lit()) * mask, 0.0);
		}

		return;
	}

	outputColor = final_color;
}