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
uniform sampler2D hue_light_texture;
uniform float point_light_hue_preservation;
uniform vec4 ambient_color;

/*
	Environment shadows.

	The shadow texture holds, per pixel:
	R - shadow height of the tallest caster whose shadow covers the pixel,
	G - strength of that shadow,
	B - shadow height of the physical body lying under the pixel (0 on the bare ground),
	A - strength of the shadows of sprites lying on the ground, which only fall onto the ground level.
	    Kept apart from red and green so that a low sprite's strength never mixes with a taller caster's height.

	NO_SHADOW areas are footprints of the maximal height, which nothing is taller than.
*/

uniform sampler2D shadow_texture;

/*
	Where the screen lies within the shadow texture, which reaches past it - see SUN_SHADOW_GUARD_BAND_PX.
*/

uniform vec2 shadow_texture_offset;

/*
	Displacement of the shadow per one pixel of shadow height, in fragment pixels.
*/

uniform vec2 shadow_step;

/*
	Zero disables receiving shadows in the current draw call.
*/

uniform float shadow_strength;

/*
	Nonzero enables the samples that guard against false shadows
	on the sunlit side of taller objects - at most max_shadow_fix_samples of them.
*/

uniform int shadow_fix;

const int max_shadow_fix_samples = 16;

/*
	Negative means the receiver's height is read from the shadow texture.
*/

uniform float receiver_height;

/*
	Zero makes the receiver's height only decide which shadows reach it, without displacing where they are sampled.
	Flat sprites lying on the ground with a height of a few pixels would otherwise show the shadows displaced
	right at their outlines.
*/

uniform int receiver_displacement;

/*
	Nonzero brightens the light in discrete bands of its intensity, zero applies it as it is.
*/

uniform int posterize_light;

/*
	0 removes the ambient color inside shadows, 1 only its brightness - keeping the hue of the surrounding light mix.
*/

uniform float shadow_hue_preservation;

vec4 fetch_shadow(highp vec2 frag_pos) {
	ivec2 size = textureSize(shadow_texture, 0);
	ivec2 texel = clamp(ivec2(floor(frag_pos + shadow_texture_offset)), ivec2(0), size - ivec2(1));
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

	float footprint = to_shadow_level(here.b);

	/*
		Under NO_SHADOW areas the sun never reaches.
	*/

	if (footprint >= 255.0) {
		return 0.0;
	}

	float r = receiver_height >= 0.0 ? receiver_height : footprint;

	/*
		Silhouette shadows: the upper half of alpha holds the foreground ones, falling on everything below the foreground,
		the lower half the ones of sprites lying on the ground, falling only on the ground level.
	*/

	float silhouette_level = to_shadow_level(here.a);
	bool foreground_silhouette = silhouette_level >= 128.0;
	float silhouette_opacity = (foreground_silhouette ? silhouette_level - 128.0 : silhouette_level) / 127.0;
	float silhouettes = foreground_silhouette || r <= 0.0 ? silhouette_opacity * shadow_strength : 0.0;

	/*
		A receiver raised above the ground samples the ground shadow further from the sun
		so that the shadow of a taller caster lands on it where it would physically.
		That sample would also catch the base of a taller object standing right behind the receiver,
		so walk towards the sample point and bail out when a taller body is found there.
	*/

	float displacement = receiver_displacement != 0 ? r : 0.0;

	if (displacement > 0.0 && shadow_fix != 0) {
		float path_len = r * length(shadow_step);
		int samples = min(max_shadow_fix_samples, int(ceil(path_len / 2.0)));

		for (int i = 1; i <= max_shadow_fix_samples; ++i) {
			if (i > samples) {
				break;
			}

			float t = r * float(i) / float(samples);

			if (to_shadow_level(fetch_shadow(frag_pos + t * shadow_step).b) > r) {
				return silhouettes;
			}
		}
	}

	vec4 s = fetch_shadow(frag_pos + displacement * shadow_step);
	float casters = to_shadow_level(s.r) > r ? s.g * shadow_strength : 0.0;
	return max(casters, silhouettes);
}

float luma(vec3 c) {
	return dot(c, vec3(0.299, 0.587, 0.114));
}

const int light_levels = 3;
const int light_step = 255/light_levels;

void main() 
{
	vec2 texcoord = gl_FragCoord.xy;
	texcoord.x /= float(textureSize(light_texture, 0).x);
	texcoord.y /= float(textureSize(light_texture, 0).y);

	vec4 light = texture(light_texture, texcoord);

	light.a = 1.0;

	/*
		The more intense the light, the more it gets brightened - up to twice, in discrete bands.

		The bands step by the light unshadowed by point lights - with the light all their shadows removed added back -
		and the shadows only scale the brightening smoothly, by the ratio of the smooth curves.
		So outside shadows the bands stay exactly as they were, and penumbras stay smooth.
		The environment shadows below work the same way - their boost comes from the unshadowed light too.
	*/

	float intensity = 1.0;

	if (posterize_light != 0) {
		vec3 unshadowed = light.rgb + texture(removed_light_texture, texcoord).rgb;

		float shadowed_intensity = min(max(max(light.r, light.g), light.b), 1.0);
		float unshadowed_intensity = min(max(max(unshadowed.r, unshadowed.g), unshadowed.b), 1.0);

		float posterized_intensity = float(
			light_step * (int(unshadowed_intensity * 255.0) / light_step + light_levels)
		) / 255.0;

		intensity = posterized_intensity * (1.0 + shadowed_intensity) / (1.0 + unshadowed_intensity);
	}

	/*
		Light removed only by the shadows of obstacles not reaching the ceiling, where walls let it through - for keeping the hue.
	*/

	if (point_light_hue_preservation > 0.0) {
		vec3 hue_source = light.rgb + texture(hue_light_texture, texcoord).rgb;
		vec3 hue_kept = hue_source * (luma(light.rgb) / max(luma(hue_source), 0.0001));
		light.rgb = mix(light.rgb, hue_kept, point_light_hue_preservation);
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