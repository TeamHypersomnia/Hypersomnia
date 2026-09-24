precision mediump int;
precision mediump float;

smooth in vec4 theColor;

out vec4 outputColor;

uniform vec2 light_pos;
uniform vec3 light_attenuation;
uniform vec3 multiply_color;
uniform float distance_mult;
uniform float max_distance;
uniform float cutoff_distance;

in vec2 theTexcoord;

uniform sampler2D basic_texture;

/*
	Posterizes this light's own contribution for the pixel-art look. Zero disables it.
*/

uniform int light_levels;

float posterize_light(float alpha, vec3 color) {
	if (light_levels <= 0) {
		return alpha;
	}

	int light_step = 255 / light_levels;
	float intensity = alpha * max(max(color.r, color.g), color.b);
	float mult = float(light_step * (int(intensity * 255.0) / light_step + light_levels)) / 255.0;

	return min(alpha * mult, 1.0);
}

void main() 
{	
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

	vec4 texture_pixel = texture(basic_texture, theTexcoord);

	final_color *= texture_pixel.a;
	final_color.a = min(theColor.a, final_color.a);
	final_color.a = posterize_light(final_color.a, final_color.rgb);

	outputColor = final_color;
}