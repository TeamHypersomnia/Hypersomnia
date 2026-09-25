precision mediump int;
precision mediump float;

smooth in vec4 theColor;
in vec2 theTexcoord;

out vec4 outputColor;

uniform sampler2D basic_texture;
uniform sampler2D light_texture;

/*
	Light of point lights that their shadows removed, and how much of the hue
	the shadows keep of the light they would get without it - 1 means they only get darker.
*/

uniform sampler2D removed_light_texture;
uniform int removed_light_available;
uniform float point_light_hue_preservation;
uniform vec4 ambient_color;

/*
	Environment shadows.

	The shadow texture holds, per pixel:
	R - shadow height of the tallest caster whose shadow covers the pixel,
	G - strength of that shadow,
	B - shadow height of the physical body lying under the pixel (0 on the bare ground),
	A - coverage by NO_SHADOW areas, under which the sun never reaches.
*/

uniform sampler2D shadow_texture;

/*
	Displacement of the shadow per one pixel of shadow height, in fragment pixels.
*/

uniform vec2 shadow_step;

/*
	Zero disables receiving shadows in the current draw call.
*/

uniform float shadow_strength;

/*
	Maximum number of samples that guard against false shadows
	on the sunlit side of taller objects. Zero disables the guard.
*/

uniform int shadow_fix;

/*
	Negative means the receiver's height is read from the shadow texture.
*/

uniform float receiver_height;

/*
	Nonzero draws the texture fully illuminated, darkened only by environment shadows - used for ground decals.
*/

uniform int fully_lit;

/*
	Nonzero brightens the light in discrete bands, zero brightens it smoothly over the same range.
*/

uniform int quantize_lights;

/*
	0 removes the ambient color inside shadows, 1 only its brightness - keeping the hue of the surrounding light mix.
*/

uniform float shadow_hue_preservation;

vec4 fetch_shadow(highp vec2 frag_pos) {
	ivec2 size = textureSize(shadow_texture, 0);
	ivec2 texel = clamp(ivec2(floor(frag_pos)), ivec2(0), size - ivec2(1));
	return texelFetch(shadow_texture, texel, 0);
}

float to_shadow_level(float v) {
	return floor(v * 255.0 + 0.5);
}

float calc_shadow_amount() {
	if (shadow_strength <= 0.0) {
		return 0.0;
	}

	highp vec2 frag_pos = gl_FragCoord.xy;
	vec4 here = fetch_shadow(frag_pos);

	float sun_reaches = 1.0 - here.a;

	if (sun_reaches <= 0.0) {
		return 0.0;
	}

	float r = receiver_height >= 0.0 ? receiver_height : to_shadow_level(here.b);

	/*
		A receiver raised above the ground samples the ground shadow further from the sun
		so that the shadow of a taller caster lands on it where it would physically.
		That sample would also catch the base of a taller object standing right behind the receiver,
		so walk towards the sample point and bail out when a taller body is found there.
	*/

	if (r > 0.0 && shadow_fix > 0) {
		float path_len = r * length(shadow_step);
		int samples = min(shadow_fix, int(ceil(path_len / 2.0)));

		for (int i = 1; i <= 16; ++i) {
			if (i > samples) {
				break;
			}

			float t = r * float(i) / float(samples);

			if (to_shadow_level(fetch_shadow(frag_pos + t * shadow_step).b) > r) {
				return 0.0;
			}
		}
	}

	vec4 s = fetch_shadow(frag_pos + r * shadow_step);
	return to_shadow_level(s.r) > r ? s.g * shadow_strength * sun_reaches : 0.0;
}

float luma(vec3 c) {
	return dot(c, vec3(0.299, 0.587, 0.114));
}

const int light_levels = 3;
const int light_step = 255/light_levels;

void main() 
{
	if (fully_lit != 0) {
		vec4 decal_pixel = theColor * texture(basic_texture, theTexcoord);
		decal_pixel.rgb *= 1.0 - calc_shadow_amount();
		outputColor = decal_pixel;
		return;
	}

	vec2 texcoord = gl_FragCoord.xy;
	texcoord.x /= float(textureSize(light_texture, 0).x);
	texcoord.y /= float(textureSize(light_texture, 0).y);

	vec4 light = texture(light_texture, texcoord);

	light.a = 1.0;

	/*
		What the pixel would get without the shadows of point lights.
	*/

	vec3 unshadowed = light.rgb;

	if (removed_light_available != 0) {
		unshadowed += texture(removed_light_texture, texcoord).rgb;
	}

	if (point_light_hue_preservation > 0.0) {
		vec3 hue_kept = unshadowed * (luma(light.rgb) / max(luma(unshadowed), 0.0001));
		light.rgb = mix(light.rgb, hue_kept, point_light_hue_preservation);
	}

	/*
		The more intense the light, the more it gets brightened - up to twice.

		Quantization steps the brightening by the light unshadowed by point lights,
		and their shadows only scale it smoothly, by the ratio of the smooth curves.
		So outside shadows the bands stay exactly as they were, and penumbras stay smooth.
		The environment shadows below work the same way - their boost comes from the unshadowed light too.
	*/

	float shadowed_intensity = min(max(max(light.r, light.g), light.b), 1.0);
	float unshadowed_intensity = min(max(max(unshadowed.r, unshadowed.g), unshadowed.b), 1.0);

	float intensity = 1.0 + shadowed_intensity;

	if (quantize_lights != 0) {
		float quantized_intensity = float(
			
			light_step * (int(unshadowed_intensity * 255.0) / light_step + light_levels)

			) / 255.0;

		intensity = quantized_intensity * (1.0 + shadowed_intensity) / (1.0 + unshadowed_intensity);
	}

	/*
		Shadows remove the ambient part of the light only - lights still illuminate shadowed areas.
	*/

	float shadow = calc_shadow_amount();

	if (shadow > 0.0) {
		vec3 without_ambient = max(light.rgb - ambient_color.rgb * shadow, vec3(0.0));
		vec3 hue_kept = light.rgb * (luma(without_ambient) / max(luma(light.rgb), 0.0001));
		light.rgb = mix(without_ambient, hue_kept, shadow_hue_preservation);
	}
	light.rgb *= intensity;

	vec4 texComponent = texture(basic_texture, theTexcoord);
	vec4 pixel = theColor * texComponent * light;
	pixel.r = min(texComponent.r, pixel.r);
	pixel.g = min(texComponent.g, pixel.g);
	pixel.b = min(texComponent.b, pixel.b);

	outputColor = pixel;
}