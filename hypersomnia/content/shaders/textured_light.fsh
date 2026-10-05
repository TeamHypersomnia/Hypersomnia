precision mediump int;
precision mediump float;

smooth in vec4 theColor;

out vec4 outputColor;

uniform vec2 light_pos;
uniform vec3 multiply_color;
uniform float distance_mult;
uniform float max_distance;

/*
	The same falloffs as in light.fsh.
*/

uniform int physical_lights;

uniform float light_core;
uniform float light_intensity;

uniform vec3 light_attenuation;
uniform float cutoff_distance;

in vec2 theTexcoord;

uniform sampler2D basic_texture;

float legacy_attenuation(float light_distance) {
	float amount = 1.0 / (
		light_attenuation.x
		+ light_attenuation.y * light_distance
		+ light_attenuation.z * light_distance * light_distance
	); 

	if (light_distance > cutoff_distance) {
		amount *= 0.5;

		float cutoff_len = max_distance - cutoff_distance;
		float cutoff_amt = (light_distance - cutoff_distance) / cutoff_len;
		cutoff_amt *= cutoff_amt;

		amount *= 1.0 - cutoff_amt;
	}

	return amount;
}

void main() 
{	
	float light_distance = length(gl_FragCoord.xy - light_pos) * distance_mult;

	if (light_distance > max_distance) {
		discard;
	}

	float texture_alpha = texture(basic_texture, theTexcoord).a;
	vec3 color = theColor.rgb * multiply_color * texture_alpha;

	/*
		Premultiplied, so that an intensity over 1 still adds more light.
	*/

	float amount;

	if (physical_lights != 0) {
		float x = light_distance / max_distance;
		float core_x = x / light_core;
		float x2 = x * x;
		float window = clamp(1.0 - x2 * x2, 0.0, 1.0);

		amount = theColor.a * light_intensity * window * window / (1.0 + core_x * core_x);
	}
	else {
		amount = min(theColor.a, legacy_attenuation(light_distance));
	}

	outputColor = vec4(color * amount, 1.0);
}